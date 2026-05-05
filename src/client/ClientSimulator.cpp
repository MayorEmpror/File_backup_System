#include "ClientSimulator.h"
#include <chrono>
#include <cstdlib>

ClientSimulator::ClientSimulator() {
    clientCount = 0;
    running = false;
    minDelayMs = 200;
    maxDelayMs = 800;
}

void ClientSimulator::setDispatcher(std::function<void(const BackupRequest&)> func) {
    dispatcher = func;
}

void ClientSimulator::addClient(Client* client) {
    if (clientCount >= MAX_CLIENTS)
        return;

    client->setRequestHandler(dispatcher);
    clients[clientCount++] = client;
}

void ClientSimulator::runClient(Client* client) {
    while (running) {
        client->generateRequest();
        int minMs = minDelayMs.load();
        int maxMs = maxDelayMs.load();
        if (maxMs < minMs) {
            maxMs = minMs;
        }
        int span = maxMs - minMs;
        int delay = minMs + (span > 0 ? rand() % (span + 1) : 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }
}

void ClientSimulator::start() {
    running = true;

    int i;
    for (i = 0; i < clientCount; i++) {
        threads[i] = std::thread(&ClientSimulator::runClient, this, clients[i]);
    }
}

void ClientSimulator::stop() {
    running = false;
}

void ClientSimulator::wait() {
    int i;
    for (i = 0; i < clientCount; i++) {
        if (threads[i].joinable()) {
            threads[i].join();
        }
    }
}

void ClientSimulator::setPacing(int minMs, int maxMs) {
    if (minMs < 10) {
        minMs = 10;
    }
    if (maxMs < minMs) {
        maxMs = minMs;
    }
    minDelayMs = minMs;
    maxDelayMs = maxMs;
}