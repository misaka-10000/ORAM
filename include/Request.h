#ifndef SEAL_ORAM_Request_H
#define SEAL_ORAM_Request_H

#include <iostream>
#include <string>


class Request {
public:
    Request(const std::string & key);
    Request(const std::string & key, const std::string & value);
    Request(const uint32_t & key);
    Request(const uint32_t & key, const std::string & value);

private:

    std::string value;
    uint32_t id;
    bool write;
friend class PathORAM;
};

#endif
