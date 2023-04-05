
#ifndef SEAL_ORAM_ORAM_H
#define SEAL_ORAM_ORAM_H

#include <string>

class ORAM {
public:
    ORAM() {}
    virtual ~ORAM() {}

    
    virtual std::string get(const std::string & key) = 0;
    virtual void put(const std::string & key, const std::string & value) = 0;

    virtual void display() = 0;
    virtual double getcnt() = 0;
    virtual std::string get(const uint32_t & key) = 0;
    virtual void put(const uint32_t & key, const std::string & value) = 0;
};

#endif //SEAL_ORAM_ORAM_H
