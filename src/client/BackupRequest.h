#ifndef BACKUP_REQUEST_H
#define BACKUP_REQUEST_H

// Standard library string type for representing file names.
#include <string>
// C time facilities used for storing a timestamp.
#include <ctime>

// A lightweight value-type representing a single backup request coming from a client.
struct BackupRequest {
    // Identifier of the client that produced this request.
    int clientId;
    // Logical name/path of the file to be "backed up" (simulation).
    std::string fileName;
    // Size of the file in bytes (or simulated bytes) used for delta/compression math.
    size_t fileSize;
    // Time the request was created (used for logging/ordering/metadata).
    std::time_t timestamp;

    // Construct a request with the client id, file name, and file size.
    BackupRequest(int id, const std::string& file, size_t size)
        // Initialize the identifying fields using the provided arguments.
        : clientId(id), fileName(file), fileSize(size) {
        // Capture creation time at construction so the request carries its own timestamp.
        timestamp = std::time(NULL);
    }
};

#endif