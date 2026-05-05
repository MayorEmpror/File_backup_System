#ifndef CLIENT_H
#define CLIENT_H

#include "IClient.h"
#include <string>
#include <functional>

class Client : public IClient {
private:
    int clientId;
    std::string name;

    std::function<void(const BackupRequest&)> requestHandler;

public:
    Client(int id, const std::string& name);

    void setRequestHandler(std::function<void(const BackupRequest&)> handler);

    int getId() const;
    void generateRequest();
};

#endif