

#ifndef SEAL_ORAM_SERVER_CONNECTOR_H
#define SEAL_ORAM_SERVER_CONNECTOR_H

#include <string>
#include <vector>

class ServerConnector {
public:
    ServerConnector() {}
    virtual ~ServerConnector() {}

    struct iterator {
        virtual ~iterator() {}
        virtual bool hasNext() = 0;
        virtual std::string next() = 0;
    };

    virtual void clear(const std::string& ns = "") = 0;
    virtual void insert(const uint32_t& id, const std::string& encrypted_block, const std::string& ns = "") = 0;
    virtual void insert(const std::vector< std::pair<uint32_t, std::string> >& blocks, const std::string& ns = "") = 0;
    virtual void insert(const std::string* sbuffer, const uint32_t& low, const size_t& len, const std::string& ns = "") = 0;
    virtual void insert(const std::vector< std::pair<std::string, std::string> >& blocks, const std::string& ns = "") = 0;
    virtual void insertWithTag(const std::vector< std::pair<std::string, std::string> >& blocks, const std::string& ns = "") = 0;
    virtual iterator* scan() = 0;
    virtual std::string find(const uint32_t& id, const std::string& ns = "") = 0;
    virtual void find(const std::vector<uint32_t>& ids, std::string* sbuffer, size_t& length, const std::string& ns = "") = 0;
    virtual std::string fetch(const std::string& id, const std::string& ns = "") = 0;
    virtual void find(const uint32_t& low, const uint32_t& high, std::vector<std::string>& blocks, const std::string& ns = "") = 0;
    virtual void findByTag(const uint32_t& tag, std::string* sbuffer, size_t& length, const std::string& ns = "") = 0;
    virtual void update(const uint32_t& id, const std::string& data, const std::string& ns = "") = 0;
    virtual void update(const std::string* sbuffer, const uint32_t& low, const size_t& len, const std::string& ns = "") = 0;
    virtual void update(const std::vector< std::pair<uint32_t, std::string> > blocks, const std::string& ns = "") = 0;
};

#endif //SEAL_ORAM_SERVER_CONNECTOR_H