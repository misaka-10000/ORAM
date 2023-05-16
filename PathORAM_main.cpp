#include <mongo/client/dbclient.h>
#include <cryptopp/osrng.h>
#include "PathORAM.h"
#include "Config.h"
#include <iostream>
#include <fstream>
using namespace mongo;
using namespace CryptoPP;

bool randomBool(){
    return 0 + (rand() % (1 - 0 + 1)) == 1;
}


int main() {
    mongo::client::initialize();
    srand((uint32_t)time(NULL));
    uint32_t N = 1023;
    PathORAM* oram = new PathORAM(N);
    //基本上填满的initialization
    for(uint32_t i = 0; i < N*PathORAM_Z; i ++) {
        char str[12];
        sprintf(str, "%zu\n", (size_t)i);
        std::string key(str);
        std::string value;
        const uint32_t tmp_len = B - AES::BLOCKSIZE - 2 * sizeof(uint32_t);
        byte tmp_buffer[tmp_len];
        AutoSeededRandomPool prng;
        prng.GenerateBlock(tmp_buffer, tmp_len);
        value = std::string((const char *)tmp_buffer, tmp_len);
        int32_t blockID = i;
        std::string bID = std::string((const char *)(& blockID), sizeof(uint32_t));
        value = bID + value;
        oram->put(key, value);
    }
    //oram->display();
    int32_t request_num=0;
    while(!oram->IsEmpty()||request_num<1000){
        while(oram->IsAvailable()&&request_num<1000){
            char str[12];
            uint32_t blockID = Util::rand_int(N*PathORAM_Z);
            sprintf(str, "%zu\n",(size_t)blockID);
            std::string key(str);
            std::string value;
            const uint32_t tmp_len = B - AES::BLOCKSIZE - 2 * sizeof(uint32_t);
            byte tmp_buffer[tmp_len];
            AutoSeededRandomPool prng;
            prng.GenerateBlock(tmp_buffer, tmp_len);
            value = std::string((const char *)tmp_buffer, tmp_len);
            std::string bID = std::string((const char *)(& blockID), sizeof(uint32_t));
            value = bID + value;
            Request r(blockID,value);
            oram->addRequest(r);
            request_num++;
        }
        oram->schedule();
    }
    std::cout<<oram->hit<<std::endl;
    delete oram;
    mongo::client::shutdown();
    return 0;
}