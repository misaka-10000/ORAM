

#ifndef SEAL_ORAM_CONFIG_H
#define SEAL_ORAM_CONFIG_H

#include <cstdint>
#include <string>

const uint32_t B = 64;
const std::string server_host = "localhost";

/* The basic binary tree ORAM */
const uint32_t Gamma = 2; //The eviction rate
const uint32_t Alpha = 2; //The ratio of the size of each bucket to log N

/* Path ORAM */
const uint32_t PathORAM_Z = 4;

/* TP-ORAM */
const double R = 0.9;
const double capacity_parameter = 4.6;
const double small_capacity_parameter = 6.0;
/* # of partitions can be modified in PartitionORAM.h */

const uint32_t waitlist_size=60;

#endif //SEAL_ORAM_CONFIG_H
