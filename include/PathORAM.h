#ifndef SEAL_ORAM_PATHORAM_H
#define SEAL_ORAM_PATHORAM_H

#include <unordered_map>
#include <cryptopp/config.h>
#include "ServerConnector.h"
#include "ORAM.h"
#include "Util.h"
#include "Request.h"

class PathORAM: public ORAM {
public:
    PathORAM(const uint32_t& n);
    virtual ~PathORAM();
    virtual std::string get(const std::string & key);
    virtual void put(const std::string & key, const std::string & value);
    virtual std::string get(const uint32_t & key);
    virtual void put(const uint32_t & key, const std::string & value);
    bool IsAvailable();
    virtual void display();
    virtual double getcnt();
    int32_t cnt;
private:
    void access(const char& op, const uint32_t& block_id, std::string& data);
    void fetchaccess(const char& op, const uint32_t& block_id, std::string& data);
    void loadaccess(const char& op, const uint32_t& block_id, std::string& data,const uint32_t& path_id);
    bool check(int x, int y, int l);
    //schedule according to the waitlist
    void schedule();

    void fetchAllBlock(std::string* allblock, size_t& length);
    void fetchAlongPath(const uint32_t& x, std::string* sbuffer, size_t& length);
    void loadAlongPath(const uint32_t& x, const std::string* sbuffer, const size_t& length);
    std::unordered_map<uint32_t, std::string> stash;
    //第一个元素是对应Path,第二个元素是所在level
    std::pair<uint32_t, uint32_t> *pos_map;
    std::vector< std::pair<uint32_t, std::string> > insert_buffer;
    std::vector<Request> waitlist;

    std::string* allblock;
    byte* key;
    std::string* sbuffer;
    uint32_t n_blocks;
    uint32_t height;

    ServerConnector* conn;
};

#endif //SEAL_ORAM_PATHORAM_H
