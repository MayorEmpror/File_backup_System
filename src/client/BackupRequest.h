#ifndef BACKUP_REQUEST_H
#define BACKUP_REQUEST_H

#include <string>
#include <ctime>

struct BackupRequest {
    int clientId;
    std::string fileName;
    size_t fileSize;
    std::time_t timestamp;

    BackupRequest(int id, const std::string& file, size_t size)
        : clientId(id), fileName(file), fileSize(size) {
        timestamp = std::time(NULL);
    }
};

#endif