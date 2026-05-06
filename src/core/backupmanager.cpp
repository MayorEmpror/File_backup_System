#include "backupmanager.h"
// Used for sleep durations in simulated work and pacing.
#include <chrono>
// Provides `rand()` used for failure simulation and timing jitter.
#include <cstdlib>
// Used to build log messages efficiently.
#include <sstream>
// Used for `std::this_thread::sleep_for`.
#include <thread>

BackupManager::BackupManager(int workers, int maxDiskWriters)
    // Start in a stopped state; `start()` flips this true.
    : running_(false),
      // No workers are active until threads pull work from the queue.
      activeWorkers_(0),
      // Semaphore capacity defines maximum concurrent "disk writers".
      diskSlots_(maxDiskWriters),
      // Store configured worker count for thread creation.
      workerCount_(workers),
      // Initialize all statistics counters to zero.
      totalProcessed_(0),
      totalCompressedBytes_(0),
      totalOriginalBytes_(0),
      totalFailed_(0),
      totalRecovered_(0) {}

BackupManager::~BackupManager() {
    // Request shutdown so worker loops can exit.
    stop();
    // Join worker threads to ensure clean destruction with no background activity.
    wait();
}

void BackupManager::setLogCallback(std::function<void(const std::string&)> cb) {
    // Store the callback that will receive log messages.
    logCallback_ = cb;
}

void BackupManager::setProgressCallback(std::function<void(int)> cb) {
    // Store the callback that will receive progress percentage.
    progressCallback_ = cb;
}

void BackupManager::setStatsCallback(std::function<void(const Stats&)> cb) {
    // Store the callback that will receive periodic stats snapshots.
    statsCallback_ = cb;
}

void BackupManager::setRequestCallback(std::function<void(const RequestEvent&)> cb) {
    // Store the callback that will receive per-request table events.
    requestCallback_ = cb;
}

void BackupManager::emitLog(const std::string& msg) {
    // Only call into the UI if a callback is installed.
    if (logCallback_) {
        // Forward the message to the consumer (typically UI).
        logCallback_(msg);
    }
}

void BackupManager::updateProgress() {
    // If nobody cares about progress, skip the computation.
    if (!progressCallback_) {
        return;
    }

    // Serialize reads of the stats counters for a consistent progress calculation.
    std::lock_guard<std::mutex> lock(statsMutex_);
    // If nothing has been counted yet, progress is defined as 0%.
    if (totalOriginalBytes_ == 0) {
        progressCallback_(0);
        return;
    }

    // Compute percent = compressedBytes/originalBytes * 100, cast to int for UI.
    int progress = static_cast<int>(
        (100.0 * static_cast<double>(totalCompressedBytes_)) /
        static_cast<double>(totalOriginalBytes_)
    );
    // Clamp to 100 in case rounding or accounting leads to >100.
    if (progress > 100) {
        progress = 100;
    }
    // Emit the computed percentage.
    progressCallback_(progress);
}

void BackupManager::emitStats() {
    // If nobody is listening for stats, skip snapshot creation.
    if (!statsCallback_) {
        return;
    }

    // Local snapshot object populated under locks, then emitted without holding locks.
    Stats snapshot;
    {
        // Copy stats counters under the stats lock to keep the snapshot consistent.
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        snapshot.processed = totalProcessed_;
        snapshot.failed = totalFailed_;
        snapshot.recovered = totalRecovered_;
        snapshot.originalBytes = totalOriginalBytes_;
        snapshot.compressedBytes = totalCompressedBytes_;
    }
    {
        // Read queue size under the queue lock to avoid data races.
        std::lock_guard<std::mutex> queueLock(queueMutex_);
        snapshot.queueDepth = queue_.size();
    }
    // Read active worker count from the atomic without taking the stats lock.
    snapshot.activeWorkers = activeWorkers_.load();
    // Deliver snapshot to consumer (typically UI).
    statsCallback_(snapshot);
}

void BackupManager::start() {
    // Avoid starting twice; if already running, do nothing.
    if (running_) {
        return;
    }
    // Flip the running flag so worker loops stay alive.
    running_ = true;
    // Create and launch worker threads.
    for (int i = 0; i < workerCount_; ++i) {
        // Each thread runs `workerLoop` with a 1-based worker id.
        workers_.push_back(std::thread(&BackupManager::workerLoop, this, i + 1));
    }
    // Emit a startup message for observability.
    emitLog("Backup manager started with " + std::to_string(workerCount_) + " workers.");
}

void BackupManager::stop() {
    // Avoid duplicate stop signals.
    if (!running_) {
        return;
    }
    // Flip running false so workers can exit once work is drained.
    running_ = false;
    // Wake all workers so they can re-check the stop condition.
    queueCv_.notify_all();
    // Log the stop request.
    emitLog("Stopping backup manager...");
}

void BackupManager::wait() {
    // Join all worker threads that were started.
    for (std::size_t i = 0; i < workers_.size(); ++i) {
        // Only join threads that are actually joinable.
        if (workers_[i].joinable()) {
            workers_[i].join();
        }
    }
    // Clear the vector to release thread objects and signal that workers are no longer running.
    workers_.clear();

    // Build a final summary message under the stats lock so counts are consistent.
    std::lock_guard<std::mutex> lock(statsMutex_);
    // Compose a single line summary of work done.
    std::ostringstream os;
    os << "Processed=" << totalProcessed_
       << ", failed=" << totalFailed_
       << ", recovered=" << totalRecovered_
       << ", compressed=" << totalCompressedBytes_
       << "/" << totalOriginalBytes_ << " bytes";
    // Emit the summary message to the log output.
    emitLog(os.str());
}

void BackupManager::submit(const BackupRequest& request) {
    // Enqueue the request under the queue mutex to keep the queue thread-safe.
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push(request);
    }
    // Wake one worker to handle the new request.
    queueCv_.notify_one();
    // Refresh stats so UI can reflect updated queue depth.
    emitStats();
}

void BackupManager::workerLoop(int workerId) {
    // Keep looping while running is true, or while there is still work in the queue to drain.
    while (running_ || !queue_.empty()) {
        // Default-construct a placeholder request (will be overwritten when work is available).
        BackupRequest request(0, "", 0);
        // Flag indicating whether we successfully popped a request.
        bool gotWork = false;

        {
            // Unique lock is required by condition_variable::wait.
            std::unique_lock<std::mutex> lock(queueMutex_);
            // Wait until either stopping is requested, or work is available.
            queueCv_.wait(lock, [this]() {
                return !running_ || !queue_.empty();
            });

            // If work exists, pop one request off the queue.
            if (!queue_.empty()) {
                request = queue_.front();
                queue_.pop();
                gotWork = true;
                // Track that this worker is now actively processing a request.
                ++activeWorkers_;
            }
        }

        // If we woke up but there was no work to take (e.g. stop signal), continue loop and re-check conditions.
        if (!gotWork) {
            continue;
        }

        // Process the request (incremental check, simulated write/compression, callbacks).
        processRequest(request, workerId);
        // Mark this worker as no longer active.
        --activeWorkers_;
        // Recompute and emit overall progress.
        updateProgress();
        // Emit a stats snapshot after completing this request.
        emitStats();
    }
}

bool BackupManager::processRequest(const BackupRequest& request, int workerId) {
    {
        // Compose a log message describing which worker is handling which request.
        std::ostringstream os;
        os << "Worker " << workerId << " handling client " << request.clientId
           << " file=" << request.fileName << " size=" << request.fileSize;
        // Emit the log message to the UI/consumer.
        emitLog(os.str());
    }

    // Delta represents how many bytes we "need" to store for this backup (used for incremental simulation).
    std::size_t delta = request.fileSize;
    // Whether we determined this request can be skipped due to no change.
    bool skipped = false;
    // Initial status label for UI; may change based on incremental/failure states.
    std::string status = "Confirmed";
    {
        // Lock metadata while checking/updating the index to keep incremental logic thread-safe.
        std::lock_guard<std::mutex> lock(metadataMutex_);
        // Look up the last known size for this file name.
        std::map<std::string, std::size_t>::iterator it = backupIndex_.find(request.fileName);
        if (it != backupIndex_.end()) {
            // If the size hasn't changed, treat this as an incremental no-op.
            if (it->second == request.fileSize) {
                emitLog("Incremental: unchanged file, skipping " + request.fileName);
                skipped = true;
                status = "Checked In";
            }
            // If not skipped, compute delta as size growth; if size shrank or equal, keep a minimal delta.
            if (!skipped) {
                delta = request.fileSize > it->second ? request.fileSize - it->second : 1;
            }
        }
    }
    // If we skipped the write, emit a RequestEvent (if enabled) and return success.
    if (skipped) {
        // Only build and emit an event if a request callback exists.
        if (requestCallback_) {
            // Populate an event structure for the UI table.
            RequestEvent event;
            event.clientId = request.clientId;
            event.fileName = request.fileName;
            event.fileSize = request.fileSize;
            event.source = (request.clientId % 3 == 0) ? "airbnb" : ((request.clientId % 2 == 0) ? "Expedia" : "Booking.com");
            event.status = status;
            event.compressionPct = 100;
            event.storedBytes = 0;
            // Deliver the event to the UI/consumer.
            requestCallback_(event);
        }
        // Skipping is treated as a successful handling of the request.
        return true;
    }

    // Acquire a disk slot to enforce max concurrent writers.
    diskSlots_.acquire();
    // Simulate disk write latency with a base delay plus random jitter.
    std::this_thread::sleep_for(std::chrono::milliseconds(40 + (rand() % 120)));

    // Randomly simulate a write failure with 10% probability.
    bool failed = (rand() % 100) < 10;
    if (failed) {
        // Log the failure and indicate a recovery attempt.
        emitLog("Write failed for " + request.fileName + ", recovery retry...");
        {
            // Under the stats lock, record that a failure occurred.
            std::lock_guard<std::mutex> statsLock(statsMutex_);
            ++totalFailed_;
        }
        // Simulate recovery time.
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        // Recovery succeeds in this simulation by forcing `failed` false.
        failed = false;
        {
            // Under the stats lock, record that a recovery occurred.
            std::lock_guard<std::mutex> statsLock(statsMutex_);
            ++totalRecovered_;
        }
        // Log recovery completion.
        emitLog("Recovery succeeded for " + request.fileName);
        // Update status label to reflect the recovery path for UI.
        status = "Due In";
    }

    // Simulate compression: store 60% of the delta.
    std::size_t compressedBytes = static_cast<std::size_t>(delta * 0.6);
    // Ensure we don't produce a 0-byte store for non-zero delta due to truncation.
    if (compressedBytes == 0 && delta > 0) {
        compressedBytes = 1;
    }

    {
        // Update the backup index under the metadata lock to reflect the latest known size for this file.
        std::lock_guard<std::mutex> metadataLock(metadataMutex_);
        backupIndex_[request.fileName] = request.fileSize;
    }

    {
        // Update aggregate statistics under the stats lock.
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        // Count this request as processed.
        ++totalProcessed_;
        // Accumulate the original delta bytes.
        totalOriginalBytes_ += delta;
        // Accumulate the compressed stored bytes.
        totalCompressedBytes_ += compressedBytes;
    }

    // Compose a log message describing the storage result.
    std::ostringstream os;
    os << "Stored " << request.fileName << " (delta=" << delta
       << ", compressed=" << compressedBytes << ")";
    // Emit the storage log line.
    emitLog(os.str());

    // Release the disk slot so another writer can proceed.
    diskSlots_.release();
    // If configured, emit a per-request event for UI table display.
    if (requestCallback_) {
        // Populate the event fields that the UI needs.
        RequestEvent event;
        event.clientId = request.clientId;
        event.fileName = request.fileName;
        event.fileSize = request.fileSize;
        event.source = (request.clientId % 3 == 0) ? "airbnb" : ((request.clientId % 2 == 0) ? "Expedia" : "Booking.com");
        event.status = status;
        // Compute compression percentage as (stored/delta)*100 for display.
        event.compressionPct = static_cast<int>((100.0 * static_cast<double>(compressedBytes)) / static_cast<double>(delta));
        event.storedBytes = compressedBytes;
        // Deliver the event to the consumer (typically UI).
        requestCallback_(event);
    }
    // Indicate success (in this simulation, we always end up succeeding after recovery).
    return true;
}
