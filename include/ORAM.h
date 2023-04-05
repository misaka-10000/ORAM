/*
 *  Copyright 2016 by SEAL-ORAM Project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

//
// Created by Dong Xie on 7/6/2016.
//

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
