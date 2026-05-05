#include "Client.h"
#include <cstdlib>

Client::Client(int id, const std::string& name)
    : clientId(id), name(name) {}

void Client::setRequestHandler(std::function<void(const BackupRequest&)> handler) {
    requestHandler = handler;
}

int Client::getId() const {
    return clientId;
}

void Client::generateRequest() {
    std::string file = "file_" + std::to_string(rand() % 1000) + ".dat";
    size_t size = 100 + rand() % 5000;

    BackupRequest req(clientId, file, size);

    if (requestHandler) {
        requestHandler(req);
    }
}