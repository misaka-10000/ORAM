#include "Request.h"
Request::Request(const std::string & key){
    uint32_t int_key;
    sscanf(key.c_str(), "%d", &int_key);
    this->id=int_key;
    write=false;

}
Request::Request(const std::string & key, const std::string & value){
    this->value=value;
    uint32_t int_key;
    sscanf(key.c_str(), "%d", &int_key);
    this->id=int_key;
    write=true;
}
Request::Request(const uint32_t & key){
    this->id=key;
    write=false;
}
Request::Request(const uint32_t & key, const std::string & value){
    this->id=key;
    this->value=value;
    write=true;
}