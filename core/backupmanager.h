#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include "../client/BackupRequest.h"
#include "Semaphore.h"
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class BackupManager {
public:
    struct Stats {
        std::size_t processed;
        std::size_t failed;
        std::size_t recovered;
        std::size_t originalBytes;
        std::size_t compressedBytes;
        std::size_t queueDepth;
        int activeWorkers;
    };
    struct RequestEvent {
        int clientId;
        std::string fileName;
        std::size_t fileSize;
        std::string source;
        std::string status;
        int compressionPct;
        std::size_t storedBytes;
    };

private:
    std::queue<BackupRequest> queue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;

    std::map<std::string, std::size_t> backupIndex_;
    std::mutex metadataMutex_;

    std::vector<std::thread> workers_;
    std::atomic<bool> running_;
    std::atomic<int> activeWorkers_;

    Semaphore diskSlots_;
    int workerCount_;

    std::size_t totalProcessed_;
    std::size_t totalCompressedBytes_;
    std::size_t totalOriginalBytes_;
    std::size_t totalFailed_;
    std::size_t totalRecovered_;
    std::mutex statsMutex_;

    std::function<void(const std::string&)> logCallback_;
    std::function<void(int)> progressCallback_;
    std::function<void(const Stats&)> statsCallback_;
    std::function<void(const RequestEvent&)> requestCallback_;

    void workerLoop(int workerId);
    void emitLog(const std::string& msg);
    void updateProgress();
    void emitStats();
    bool processRequest(const BackupRequest& request, int workerId);

public:
    BackupManager(int workers, int maxDiskWriters);
    ~BackupManager();

    void setLogCallback(std::function<void(const std::string&)> cb);
    void setProgressCallback(std::function<void(int)> cb);
    void setStatsCallback(std::function<void(const Stats&)> cb);
    void setRequestCallback(std::function<void(const RequestEvent&)> cb);

    void start();
    void stop();
    void wait();

    void submit(const BackupRequest& request);
};

#endif
