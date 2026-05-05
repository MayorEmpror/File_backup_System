#ifndef ICLIENT_H
#define ICLIENT_H

#include "BackupRequest.h"

class IClient {
public:
    virtual ~IClient() {}

    virtual int getId() const = 0;
    virtual void generateRequest() = 0;
};

#endif