#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

// BackupRequest is the payload type in the manager's work queue.
#include "../client/BackupRequest.h"
// Semaphore is used to limit concurrent "disk" writers.
#include "Semaphore.h"
// Atomics are used for lock-free flags/counters shared across threads.
#include <atomic>
// Condition variable is used to block worker threads when the queue is empty.
#include <condition_variable>
// std::size_t type used for byte counts and sizes.
#include <cstddef>
// std::function used for callbacks into the UI/logging layer.
#include <functional>
// std::map used as the backup index (fileName -> last backed-up size).
#include <map>
// std::mutex used to protect shared state (queue, metadata, stats).
#include <mutex>
// std::queue used as the FIFO request queue.
#include <queue>
// std::string for file names and human-readable fields.
#include <string>
// std::thread used for worker threads.
#include <thread>
// std::vector used to store worker thread objects.
#include <vector>

// Central coordinator that accepts BackupRequest objects and processes them concurrently via worker threads.
class BackupManager {
public:
    // Aggregated snapshot of system statistics for UI display.
    struct Stats {
        // Total number of successfully processed (stored or skipped) requests.
        std::size_t processed;
        // Total number of simulated write failures encountered.
        std::size_t failed;
        // Total number of simulated recoveries performed after a failure.
        std::size_t recovered;
        // Total original bytes (delta bytes) that would have been stored without compression.
        std::size_t originalBytes;
        // Total compressed bytes actually "stored" after compression simulation.
        std::size_t compressedBytes;
        // Current number of queued requests waiting for workers.
        std::size_t queueDepth;
        // Current number of workers actively processing a request.
        int activeWorkers;
    };
    // Per-request event emitted to the UI for populating the request table.
    struct RequestEvent {
        // Client id associated with this request.
        int clientId;
        // File name associated with this request.
        std::string fileName;
        // Full file size reported by the client.
        std::size_t fileSize;
        // Source label used by UI filters (simulated vendor).
        std::string source;
        // Human-readable status for the request lifecycle.
        std::string status;
        // Compression percentage (how much of delta remains) or ratio indicator.
        int compressionPct;
        // Number of bytes stored for this request after compression (or 0 for skip).
        std::size_t storedBytes;
    };

private:
    // FIFO queue holding pending backup requests.
    std::queue<BackupRequest> queue_;
    // Protects access to `queue_`.
    std::mutex queueMutex_;
    // Used to wake workers when new work arrives or when stopping.
    std::condition_variable queueCv_;

    // Maps file name to last backed-up size for incremental backup simulation.
    std::map<std::string, std::size_t> backupIndex_;
    // Protects access to metadata like `backupIndex_`.
    std::mutex metadataMutex_;

    // Worker thread objects created by `start()` and joined by `wait()`.
    std::vector<std::thread> workers_;
    // Global running flag controlling worker loop lifetime.
    std::atomic<bool> running_;
    // Count of workers currently inside `processRequest`.
    std::atomic<int> activeWorkers_;

    // Counting semaphore limiting how many workers may write concurrently.
    Semaphore diskSlots_;
    // Configured number of worker threads to start.
    int workerCount_;

    // Stats counters updated during processing; protected by `statsMutex_` where needed.
    std::size_t totalProcessed_;
    std::size_t totalCompressedBytes_;
    std::size_t totalOriginalBytes_;
    std::size_t totalFailed_;
    std::size_t totalRecovered_;
    // Protects reads/writes to the stats counters.
    std::mutex statsMutex_;

    // Optional callback for emitting log strings to the UI.
    std::function<void(const std::string&)> logCallback_;
    // Optional callback for emitting overall progress percentage to the UI.
    std::function<void(int)> progressCallback_;
    // Optional callback for emitting stats snapshots to the UI.
    std::function<void(const Stats&)> statsCallback_;
    // Optional callback for emitting per-request events to the UI table.
    std::function<void(const RequestEvent&)> requestCallback_;

    // Worker thread main loop: waits for work, processes requests, updates UI callbacks.
    void workerLoop(int workerId);
    // Helper to route a log message if a callback is installed.
    void emitLog(const std::string& msg);
    // Computes and emits progress percentage based on compressed vs original bytes.
    void updateProgress();
    // Builds and emits a Stats snapshot to the UI.
    void emitStats();
    // Performs the actual request processing (incremental check, simulated write, simulated compression).
    bool processRequest(const BackupRequest& request, int workerId);

public:
    // Construct a manager with N workers and a max number of concurrent disk writers.
    BackupManager(int workers, int maxDiskWriters);
    // Ensure the system stops and joins threads on destruction.
    ~BackupManager();

    // Install a log callback (e.g. UI appending to a QTextEdit).
    void setLogCallback(std::function<void(const std::string&)> cb);
    // Install a progress callback (e.g. UI progress bar).
    void setProgressCallback(std::function<void(int)> cb);
    // Install a stats callback (e.g. UI KPI cards/charts).
    void setStatsCallback(std::function<void(const Stats&)> cb);
    // Install a per-request callback (e.g. UI table rows).
    void setRequestCallback(std::function<void(const RequestEvent&)> cb);

    // Start worker threads and begin processing.
    void start();
    // Signal the system to stop accepting/processing new work (workers exit once queue drains).
    void stop();
    // Join worker threads and emit a final summary.
    void wait();

    // Enqueue a new request for processing by workers.
    void submit(const BackupRequest& request);
};

#endif
