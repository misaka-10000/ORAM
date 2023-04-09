#include <mongo/client/dbclient.h>
#include <cryptopp/osrng.h>
#include "PathORAM.h"
#include "Config.h"
#include <iostream>
using namespace mongo;
using namespace CryptoPP;

bool randomBool(){
    return 0 + (rand() % (1 - 0 + 1)) == 1;
}


int main() {
    mongo::client::initialize();
    srand((uint32_t)time(NULL));
    uint32_t N = 32767;
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
    //printf("occupation rate of block close to leaf:%lf\n",oram->getcnt()/(N*PathORAM_Z));

    int32_t request_num=0;
    while(!oram->IsEmpty()||request_num<1024){
        while(oram->IsAvailable()&&request_num<1024){
            char str[12];
            uint32_t blockID = Util::rand_int(1024);
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
        //std::cout<<"-----------------------"<<std::endl;
        oram->schedule();
        //std::cout<<"-----------------------"<<std::endl;
    }
    //oram->display();
    std::cout<<"fusion cnt"<<oram->fusion_cnt<<"   "<<"ordinary cnt:"<<oram->ori_cnt<<std::endl;

    // size_t roundNum = 5;
    // for(size_t r = 0; r < roundNum; r ++) {
    //     for(size_t i = 0; i < N*PathORAM_Z; i ++) {
    //         char str[12];
    //         sprintf(str, "%zu\n", i);
    //         std::string key(str);
    //         std::string block = oram->get(key);
    //         uint32_t value;
    //         memcpy((& value), block.c_str(), sizeof(uint32_t));
    //         //printf("%d ", value);
    //     }
    //     //printf("\n=========================================================================\n");
    // }

    //printf("\n=========================================================================\n");
    //printf("the probility of two continuous access with path overlapping is %d\n",(oram->getcnt())*100/(roundNum*N*PathORAM_Z+N*PathORAM_Z+N*PathORAM_Z));
    delete oram;
    mongo::client::shutdown();
    return 0;
}