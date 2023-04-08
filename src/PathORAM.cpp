#include "PathORAM.h"
#include <cmath>
#include <cryptopp/osrng.h>
#include "MongoConnector.h"
#include "Config.h"
#include <iostream>
using namespace CryptoPP;

PathORAM::PathORAM(const uint32_t& n) {
    //树高，n=7则height=3
    height = (uint32_t)floor(log2((double)n)) + 1;
    //叶子层bucket的数量
    n_blocks = (uint32_t)1 << (height - 1);
    stash.clear();
    //对于每一个block，存其叶子bucket的位置(路径+level)
    pos_map = new std::pair<uint32_t, uint32_t>[n*PathORAM_Z];
    //建立连接
    conn = new MongoConnector(server_host, "oram.path");
    //生成一个随机数的密钥
    key = new byte[Util::key_length];
    Util::prng.GenerateBlock(key, Util::key_length);
    //sbuffer用于存储一条路径的信息
    sbuffer = new std::string[(height) * PathORAM_Z];
    allblock = new std::string[n*PathORAM_Z];
    //遍历每一个bucket,将其用加密的dummy head填满
    for (uint32_t i = 0; i < (1 << height) - 1; ++i) {
        for (uint32_t j = 0; j < PathORAM_Z; ++j) {
            std::string tmp  = Util::generate_random_block(B - Util::aes_block_size - sizeof(uint32_t));
            int32_t dummyID = -1;
            std::string dID = std::string((const char *)(& dummyID), sizeof(uint32_t));
            std::string cipher;
            Util::aes_encrypt(dID + tmp, key, cipher);
            //将block以数组的形式存储在server中
            conn->insert(i * PathORAM_Z + j, cipher);
        }
    }
    //默认初始时将高度初始化为height+1也即不存在
    for (size_t i = 0; i < n*PathORAM_Z; ++i) pos_map[i] = std::make_pair(Util::rand_int(n_blocks),height+1);
}

PathORAM::~PathORAM() {
    delete[] key;
    delete[] sbuffer;
    delete[] pos_map;
    delete conn;
}
//read操作
std::string PathORAM::get(const std::string & key) {
    std::string res;
    uint32_t int_key;
    sscanf(key.c_str(), "%d", &int_key);
    //如果当前存在在stash中，那么直接返回即可
    if(stash.find(int_key) != stash.end()) return stash[int_key];
    access('r', int_key, res);
    return res;
}
//write操作
void PathORAM::put(const std::string & key, const std::string & value) {
    uint32_t int_key;
    sscanf(key.c_str(), "%d", &int_key);
    std::string value2 = value;
    //已经存在于stash中就应该直接写
    if(stash.find(int_key) != stash.end()) {
        stash[int_key]=value;
        return;
    }
    access('w', int_key, value2);
}
//read操作
std::string PathORAM::get(const uint32_t & key) {
    std::string res;
    if(stash.find(key) != stash.end()) return stash[key];
    access('r', key, res);
    return res;
}
//write操作，使用数字访问block号
void PathORAM::put(const uint32_t & key, const std::string & value) {
    std::string value2 = value;
    if(stash.find(key) != stash.end()) {
        stash[key]=value;
        return;
    }
    access('w', key, value2);
}

void PathORAM::access(const char& op, const uint32_t& block_id, std::string& data) {
    //x记录原有Path,remap block
    uint32_t x = pos_map[block_id].first;
    pos_map[block_id].first = Util::rand_int(n_blocks);
    pos_map[block_id].second = height+1;
    //read path,length是一个参数，其实就是一条路径block的数量
    size_t length;
    fetchAlongPath(x, sbuffer, length);
    for (size_t i = 0; i < length; ++i) {
        std::string plain;
        Util::aes_decrypt(sbuffer[i], key, plain);
        int32_t b_id;
        memcpy(&b_id, plain.c_str(), sizeof(uint32_t));
        //如果不是dummy block，则将数据先写到stash,反正后续阶段会写回的
        if (b_id != -1) {
            stash[b_id] = plain.substr(sizeof(uint32_t));
            pos_map[b_id].second=height+1;
        }
    }

    //read操作复制数据，write操作覆盖stash
    if (op == 'r') data = stash[block_id];
    else stash[block_id] = data;

    //寻找与原叶子节点属于同一path的block
    for (uint32_t i = 0; i < height; ++i) {
        uint32_t tot = 0;
        uint32_t base = i * PathORAM_Z;
        std::unordered_map<uint32_t, std::string>::iterator j, tmp;
        j = stash.begin();
        while (j != stash.end() && tot < PathORAM_Z) {
            //检测与原叶子节点是否属于同一path,是的话则将他加入sbuffer写回队列
            if (check(pos_map[j->first].first, x, i)) {
                std::string b_id = std::string((const char *)(&(j->first)), sizeof(uint32_t));
                sbuffer[base + tot] = b_id + j->second;
                //写一下level信息
                pos_map[j->first].second=height-1-i;
                tmp = j; ++j; stash.erase(tmp);
                ++tot;
            } else ++j;
        }
        //不足的用dummy block替代
        for (int k = tot; k < PathORAM_Z; ++k) {
            std::string tmp_block  = Util::generate_random_block(B - Util::aes_block_size - sizeof(uint32_t));
            int32_t dummyID = -1;
            std::string dID = std::string((const char *)(& dummyID), sizeof(uint32_t));
            sbuffer[base + k] = dID + tmp_block;
        }
    }
    //write Path
    for (size_t i = 0; i < height * PathORAM_Z; ++i) {
        std::string cipher;
        Util::aes_encrypt(sbuffer[i], key, cipher);
        sbuffer[i] = cipher;
    }
    loadAlongPath(x, sbuffer, height * PathORAM_Z);
}

void PathORAM::fetchaccess(const char& op, const uint32_t& block_id, std::string& data){
    //x记录原有Path,remap block
    uint32_t x = pos_map[block_id].first;
    pos_map[block_id].first = Util::rand_int(n_blocks);
    pos_map[block_id].second = height+1;
    //read path,length是一个参数，其实就是一条路径block的数量
    size_t length;
    fetchAlongPath(x, sbuffer, length);
    for (size_t i = 0; i < length; ++i) {
        std::string plain;
        Util::aes_decrypt(sbuffer[i], key, plain);
        int32_t b_id;
        memcpy(&b_id, plain.c_str(), sizeof(uint32_t));
        //如果不是dummy block，则将数据先写到stash,反正后续阶段会写回的
        if (b_id != -1) {
            stash[b_id] = plain.substr(sizeof(uint32_t));
            pos_map[block_id].second = height+1;
        }
    }

    //read操作复制数据，write操作覆盖stash
    if (op == 'r') data = stash[block_id];
    else stash[block_id] = data;
}

void PathORAM::loadaccess(const char& op, const uint32_t& block_id, std::string& data,const uint32_t& path_id){
    
    /*使用先前那个read的块的ID*/
    //pos_map[block_id].first = Util::rand_int(n_blocks);
    pos_map[block_id].second = height+1;

    //read操作复制数据，write操作覆盖stash
    if (op == 'r') data = stash[block_id];
    else stash[block_id] = data;

    //寻找与原叶子节点属于同一path的block
    for (uint32_t i = 0; i < height; ++i) {
        uint32_t tot = 0;
        uint32_t base = i * PathORAM_Z;
        std::unordered_map<uint32_t, std::string>::iterator j, tmp;
        j = stash.begin();
        while (j != stash.end() && tot < PathORAM_Z) {
            //检测与原叶子节点是否属于同一path,是的话则将他加入sbuffer写回队列
            if (check(pos_map[j->first].first, path_id, i)) {
                std::string b_id = std::string((const char *)(&(j->first)), sizeof(uint32_t));
                sbuffer[base + tot] = b_id + j->second;
                //写一下level信息
                pos_map[j->first].second=height-1-i;
                tmp = j; ++j; stash.erase(tmp);
                ++tot;
            } else ++j;
        }
        //不足的用dummy block替代
        for (int k = tot; k < PathORAM_Z; ++k) {
            std::string tmp_block  = Util::generate_random_block(B - Util::aes_block_size - sizeof(uint32_t));
            int32_t dummyID = -1;
            std::string dID = std::string((const char *)(& dummyID), sizeof(uint32_t));
            sbuffer[base + k] = dID + tmp_block;
        }
    }
    //write Path
    for (size_t i = 0; i < height * PathORAM_Z; ++i) {
        std::string cipher;
        Util::aes_encrypt(sbuffer[i], key, cipher);
        sbuffer[i] = cipher;
    }
    loadAlongPath(path_id, sbuffer, height * PathORAM_Z);
}

bool PathORAM::check(int x, int y, int l) {
    return (x >> l) == (y >> l);
}
//读入Path
void PathORAM::fetchAlongPath(const uint32_t& x, std::string* sbuffer, size_t& length) {
    //找到block所对应叶子bucket的位置
    uint32_t cur_pos = x + (1 << (height - 1));
    std::vector<uint32_t> ids;
    //将当前bucket的所有push到ids中，接着向上找寻父亲结点直至根节点
    while (cur_pos > 0) {
        for (uint32_t i = 0; i < PathORAM_Z; ++i)
            ids.push_back((cur_pos - 1) * PathORAM_Z + i);
        cur_pos >>= 1;
    }
    //将所有要的block从服务器读出
    conn->find(ids, sbuffer, length);
}
//写回Path
void PathORAM::loadAlongPath(const uint32_t& x, const std::string* sbuffer, const size_t& length) {
    uint32_t cur_pos = x + (1 << (height - 1));
    uint32_t offset = 0;
    insert_buffer.clear();
    while (cur_pos > 0) {
        for (uint32_t i = 0; i < PathORAM_Z; ++i)
            //位置与数据配对
            insert_buffer.emplace_back(std::make_pair((cur_pos - 1) * PathORAM_Z + i, sbuffer[offset + i]));
        offset += PathORAM_Z;
        cur_pos >>= 1;
    }
    conn->update(insert_buffer);
}

bool PathORAM::IsAvailable(){
    return waitlist.size()<waitlist_size;
}

bool PathORAM::IsEmpty(){
    return waitlist.empty();
}

void PathORAM::display(){
    cnt=0;
    printf("All element in Stash:\n");
    for(std::pair<uint32_t, std::string> kv:stash){
        printf("block_id:%d belonging path:%d\n",kv.first,pos_map[kv.first].first);
    }
    size_t length;
    printf("Now showing the tree block:\n");
    fetchAllBlock(allblock, length);
    uint32_t totoal_node= ((uint32_t)1 << height) -1;
    for(int i=0;i<totoal_node;i++){
        for(int j=0;j<PathORAM_Z;j++){
            std::string plain;
            Util::aes_decrypt(allblock[i*PathORAM_Z+j], key, plain);
            int32_t b_id;
            memcpy(&b_id, plain.c_str(), sizeof(uint32_t));
            if(b_id!=-1&&pos_map[b_id].second>=height-2) cnt+=1;
            if(b_id!=-1)
                printf("block_ID:%d path:%d level:%d\n",b_id,pos_map[b_id].first,pos_map[b_id].second);
            else
                printf("block_ID:%d\n",b_id);
        }
    }
    printf("---------------------------\n");
}

void PathORAM::fetchAllBlock(std::string* allblock, size_t& length) {
    std::vector<uint32_t> ids;
    //将当前bucket的所有push到ids中，接着向上找寻父亲结点直至根节点
    uint32_t totoal_node= ((uint32_t)1 << height) -1;
    uint32_t cur_pos = 0;
    while (cur_pos < totoal_node) {
        for (uint32_t i = 0; i < PathORAM_Z; ++i)
            ids.push_back(cur_pos * PathORAM_Z + i);
        cur_pos += 1;
    }
    //将所有要的block从服务器读出
    conn->findAll(ids, allblock, length);
}

void PathORAM::schedule(){
    int i,j;
    bool fusion = false;
    for(i=0;i<waitlist.size();i++){
        Request r1=waitlist[i];
        uint32_t p1=pos_map[r1.id].first;
        uint32_t level1=pos_map[r1.id].second;
        if(level1==height+1) continue;
        for(j=i+1;j<waitlist.size();j++){
            Request r2=waitlist[j];
            uint32_t p2=pos_map[r2.id].first;
            uint32_t level2=pos_map[r2.id].second;
            if(level2==height+1) continue;
            if(level1<level2){
                uint32_t start = (p1/((uint32_t)1 << (height - 1 - level1)))*((uint32_t)1 << (height - 1 - level1));
                if(p2<=start+((uint32_t)1 << (height - 1 - level1))-1&&p2>=start){
                    // std::cout<<p1<<" "<<level1<<" "<<p2<<" "<<level2<<std::endl;
                    // std::cout<<"level1 is lower "<<p1<<std::endl;
                    //std::cout<<start<<" "<<start+((uint32_t)1 << (height - 1 - level1))-1<<std::endl;
                    fusion=true;
                    break;
                }
            }
            else{
                uint32_t start = (p2/((uint32_t)1 << (height - 1 - level2)))*((uint32_t)1 << (height - 1 - level2));
                if(p1<=start+((uint32_t)1 << (height - 1 - level2))-1&&p1>=start){
                    // std::cout<<p1<<" "<<level1<<" "<<p2<<" "<<level2<<std::endl;
                    // std::cout<<"level2 is lower  "<<p2<<std::endl;
                    // std::cout<<start<<" "<<start+((uint32_t)1 << (height - 1 - level2))-1<<std::endl;
                    fusion=true;
                    break;
                }
            }
        }
        if(fusion) break;
    }
    if(fusion){
        Request r1=waitlist[i];
        Request r2=waitlist[j];
        uint32_t level1=pos_map[r1.id].second;
        uint32_t level2=pos_map[r2.id].second;
        printf("fusion to access level %d at Path %d with level %d at Path %d\n",pos_map[r1.id].second,pos_map[r1.id].first,pos_map[r2.id].second,pos_map[r2.id].first);
        if(level1<level2){
                //原来的路径号,也是要写回的路径号
                uint32_t x = pos_map[r1.id].first;
                if(r1.write) fetchaccess('w',r1.id,r1.value);
                else fetchaccess('r',r1.id,r1.value);
                //新的r2把他和r1映射到同一条道路
                pos_map[r2.id].first = pos_map[r1.id].first;
                if(r2.write) loadaccess('w',r2.id,r2.value,x);
                else fetchaccess('r',r2.id,r2.value);
        }
        else{
            //原来的路径号,也是要写回的路径号
            uint32_t x = pos_map[r2.id].first;
            if(r2.write) fetchaccess('w',r2.id,r2.value);
            else fetchaccess('r',r2.id,r2.value);
            //新的r2把他和r1映射到同一条道路
            pos_map[r1.id].first = pos_map[r2.id].first;
            if(r1.write) loadaccess('w',r1.id,r1.value,x);
            else fetchaccess('r',r1.id,r1.value); 
        }
        waitlist.erase(waitlist.begin() + i);
        waitlist.erase(waitlist.begin() + j-1);
        printf("------------------------\n");

    }
    //无法进行fusion
    else{
        if(waitlist[0].write){
            put(waitlist[0].id,waitlist[0].value);
        }
        else{
            std::string output=get(waitlist[0].id);
        }
        waitlist.erase(waitlist.begin());
    }

}

double PathORAM::getcnt(){
    return cnt;
}

void PathORAM::addRequest(Request R){
    waitlist.push_back(R);
}