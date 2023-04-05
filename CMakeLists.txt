cmake_minimum_required(VERSION 3.3)
project(SEAL_ORAM)

include_directories(include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3")

set(GENERAL_SOURCE_FILES
        include/ServerConnector.h
        include/MongoConnector.h
        include/Config.h
        include/Util.h
        include/ORAM.h
        include/Request.h
        src/Request.cpp
        src/MongoConnector.cpp
        src/Util.cpp)

set(PATHORAM_SOURCE_FILES
        include/PathORAM.h
        src/PathORAM.cpp)


# add_executable(IBSOS_main IBSOS_main.cpp ${GENERAL_SOURCE_FILES} ${IBSOS_SOURCE_FILES})
# target_link_libraries(IBSOS_main cryptopp pthread mongoclient boost_thread boost_system boost_regex)

add_executable(PathORAM_main PathORAM_main.cpp ${GENERAL_SOURCE_FILES} ${PATHORAM_SOURCE_FILES})
target_link_libraries(PathORAM_main cryptopp pthread mongoclient boost_thread boost_system boost_regex)
