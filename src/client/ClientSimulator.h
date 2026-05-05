#ifndef CLIENT_SIMULATOR_H
#define CLIENT_SIMULATOR_H

#include "Client.h"
#include <thread>
#include <atomic>
#include <functional>

#define MAX_CLIENTS 50

class ClientSimulator {
private:
    Client* clients[MAX_CLIENTS];
    std::thread threads[MAX_CLIENTS];

    int clientCount;
    std::atomic<bool> running;
    std::atomic<int> minDelayMs;
    std::atomic<int> maxDelayMs;

    std::function<void(const BackupRequest&)> dispatcher;

    void runClient(Client* client);

public:
    ClientSimulator();

    void setDispatcher(std::function<void(const BackupRequest&)> func);

    void addClient(Client* client);

    void start();
    void stop();
    void wait();
    void setPacing(int minMs, int maxMs);
};

#endif