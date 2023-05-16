#ifndef SEAL_ORAM_PATHORAM_H
#define SEAL_ORAM_PATHORAM_H

#include <unordered_map>
#include <cryptopp/config.h>
#include "ServerConnector.h"
#include "Util.h"
#include "Request.h"
#include <vector>

class PathORAM{
public:
    PathORAM(const uint32_t& n);
    virtual ~PathORAM();
    std::string get(const std::string & key);
    void put(const std::string & key, const std::string & value);
    std::string get(const uint32_t & key);
    void put(const uint32_t & key, const std::string & value);
    bool IsAvailable();
    bool IsEmpty();
    void display();
    double getcnt();
    void disp();
    void s_disp();
    uint64_t cnt;
    std::vector<Request> waitlist;
    void addRequest(Request R);
    //schedule according to the waitlist
    void schedule();
    //第一个元素是对应Path,第二个元素是所在level
    std::pair<uint32_t, uint32_t> *pos_map;
    uint32_t fusion_cnt;
    uint32_t ori_cnt;
    uint32_t hit;
private:
    void access(const char& op, const uint32_t& block_id, std::string& data);
    void fetchaccess(const char& op, const uint32_t& block_id, std::string& data);
    void loadaccess(const char& op, const uint32_t& block_id, std::string& data,const uint32_t& path_id);
    bool check(int x, int y, int l);
    void fetchAllBlock(std::string* allblock, size_t& length);
    void fetchAlongPath(const uint32_t& x, std::string* sbuffer, size_t& length);
    void loadAlongPath(const uint32_t& x, const std::string* sbuffer, const size_t& length);
    std::unordered_map<uint32_t, std::string> stash;
    std::vector< std::pair<uint32_t, std::string> > insert_buffer;

    std::string* allblock;
    byte* key;
    std::string* sbuffer;
    uint32_t n_blocks;
    uint32_t height;
    ServerConnector* conn;
};

#endif //SEAL_ORAM_PATHORAM_H
