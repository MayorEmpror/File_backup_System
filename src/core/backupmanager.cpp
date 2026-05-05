#include "backupmanager.h"
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <thread>

BackupManager::BackupManager(int workers, int maxDiskWriters)
    : running_(false),
      activeWorkers_(0),
      diskSlots_(maxDiskWriters),
      workerCount_(workers),
      totalProcessed_(0),
      totalCompressedBytes_(0),
      totalOriginalBytes_(0),
      totalFailed_(0),
      totalRecovered_(0) {}

BackupManager::~BackupManager() {
    stop();
    wait();
}

void BackupManager::setLogCallback(std::function<void(const std::string&)> cb) {
    logCallback_ = cb;
}

void BackupManager::setProgressCallback(std::function<void(int)> cb) {
    progressCallback_ = cb;
}

void BackupManager::setStatsCallback(std::function<void(const Stats&)> cb) {
    statsCallback_ = cb;
}

void BackupManager::setRequestCallback(std::function<void(const RequestEvent&)> cb) {
    requestCallback_ = cb;
}

void BackupManager::emitLog(const std::string& msg) {
    if (logCallback_) {
        logCallback_(msg);
    }
}

void BackupManager::updateProgress() {
    if (!progressCallback_) {
        return;
    }

    std::lock_guard<std::mutex> lock(statsMutex_);
    if (totalOriginalBytes_ == 0) {
        progressCallback_(0);
        return;
    }

    int progress = static_cast<int>(
        (100.0 * static_cast<double>(totalCompressedBytes_)) /
        static_cast<double>(totalOriginalBytes_)
    );
    if (progress > 100) {
        progress = 100;
    }
    progressCallback_(progress);
}

void BackupManager::emitStats() {
    if (!statsCallback_) {
        return;
    }

    Stats snapshot;
    {
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        snapshot.processed = totalProcessed_;
        snapshot.failed = totalFailed_;
        snapshot.recovered = totalRecovered_;
        snapshot.originalBytes = totalOriginalBytes_;
        snapshot.compressedBytes = totalCompressedBytes_;
    }
    {
        std::lock_guard<std::mutex> queueLock(queueMutex_);
        snapshot.queueDepth = queue_.size();
    }
    snapshot.activeWorkers = activeWorkers_.load();
    statsCallback_(snapshot);
}

void BackupManager::start() {
    if (running_) {
        return;
    }
    running_ = true;
    for (int i = 0; i < workerCount_; ++i) {
        workers_.push_back(std::thread(&BackupManager::workerLoop, this, i + 1));
    }
    emitLog("Backup manager started with " + std::to_string(workerCount_) + " workers.");
}

void BackupManager::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    queueCv_.notify_all();
    emitLog("Stopping backup manager...");
}

void BackupManager::wait() {
    for (std::size_t i = 0; i < workers_.size(); ++i) {
        if (workers_[i].joinable()) {
            workers_[i].join();
        }
    }
    workers_.clear();

    std::lock_guard<std::mutex> lock(statsMutex_);
    std::ostringstream os;
    os << "Processed=" << totalProcessed_
       << ", failed=" << totalFailed_
       << ", recovered=" << totalRecovered_
       << ", compressed=" << totalCompressedBytes_
       << "/" << totalOriginalBytes_ << " bytes";
    emitLog(os.str());
}

void BackupManager::submit(const BackupRequest& request) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push(request);
    }
    queueCv_.notify_one();
    emitStats();
}

void BackupManager::workerLoop(int workerId) {
    while (running_ || !queue_.empty()) {
        BackupRequest request(0, "", 0);
        bool gotWork = false;

        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this]() {
                return !running_ || !queue_.empty();
            });

            if (!queue_.empty()) {
                request = queue_.front();
                queue_.pop();
                gotWork = true;
                ++activeWorkers_;
            }
        }

        if (!gotWork) {
            continue;
        }

        processRequest(request, workerId);
        --activeWorkers_;
        updateProgress();
        emitStats();
    }
}

bool BackupManager::processRequest(const BackupRequest& request, int workerId) {
    {
        std::ostringstream os;
        os << "Worker " << workerId << " handling client " << request.clientId
           << " file=" << request.fileName << " size=" << request.fileSize;
        emitLog(os.str());
    }

    std::size_t delta = request.fileSize;
    bool skipped = false;
    std::string status = "Confirmed";
    {
        std::lock_guard<std::mutex> lock(metadataMutex_);
        std::map<std::string, std::size_t>::iterator it = backupIndex_.find(request.fileName);
        if (it != backupIndex_.end()) {
            if (it->second == request.fileSize) {
                emitLog("Incremental: unchanged file, skipping " + request.fileName);
                skipped = true;
                status = "Checked In";
            }
            if (!skipped) {
                delta = request.fileSize > it->second ? request.fileSize - it->second : 1;
            }
        }
    }
    if (skipped) {
        if (requestCallback_) {
            RequestEvent event;
            event.clientId = request.clientId;
            event.fileName = request.fileName;
            event.fileSize = request.fileSize;
            event.source = (request.clientId % 3 == 0) ? "airbnb" : ((request.clientId % 2 == 0) ? "Expedia" : "Booking.com");
            event.status = status;
            event.compressionPct = 100;
            event.storedBytes = 0;
            requestCallback_(event);
        }
        return true;
    }

    diskSlots_.acquire();
    std::this_thread::sleep_for(std::chrono::milliseconds(40 + (rand() % 120)));

    bool failed = (rand() % 100) < 10;
    if (failed) {
        emitLog("Write failed for " + request.fileName + ", recovery retry...");
        {
            std::lock_guard<std::mutex> statsLock(statsMutex_);
            ++totalFailed_;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        failed = false;
        {
            std::lock_guard<std::mutex> statsLock(statsMutex_);
            ++totalRecovered_;
        }
        emitLog("Recovery succeeded for " + request.fileName);
        status = "Due In";
    }

    std::size_t compressedBytes = static_cast<std::size_t>(delta * 0.6);
    if (compressedBytes == 0 && delta > 0) {
        compressedBytes = 1;
    }

    {
        std::lock_guard<std::mutex> metadataLock(metadataMutex_);
        backupIndex_[request.fileName] = request.fileSize;
    }

    {
        std::lock_guard<std::mutex> statsLock(statsMutex_);
        ++totalProcessed_;
        totalOriginalBytes_ += delta;
        totalCompressedBytes_ += compressedBytes;
    }

    std::ostringstream os;
    os << "Stored " << request.fileName << " (delta=" << delta
       << ", compressed=" << compressedBytes << ")";
    emitLog(os.str());

    diskSlots_.release();
    if (requestCallback_) {
        RequestEvent event;
        event.clientId = request.clientId;
        event.fileName = request.fileName;
        event.fileSize = request.fileSize;
        event.source = (request.clientId % 3 == 0) ? "airbnb" : ((request.clientId % 2 == 0) ? "Expedia" : "Booking.com");
        event.status = status;
        event.compressionPct = static_cast<int>((100.0 * static_cast<double>(compressedBytes)) / static_cast<double>(delta));
        event.storedBytes = compressedBytes;
        requestCallback_(event);
    }
    return true;
}
