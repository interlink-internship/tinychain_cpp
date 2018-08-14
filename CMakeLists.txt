cmake_minimum_required(VERSION 3.12)
project(tinychain_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(tinychain_cpp src/main.cpp src/Block.h src/OutPoint.h src/Params.h src/sha256.cpp src/sha256.h src/Transaction.h src/TxIn.h src/TxOut.h src/UnspentTxOut.h src/TinyChain.cpp src/TinyChain.h src/LocatedBlock.h src/TxoutForTxin.h src/Utility.h src/Utility.cpp)

add_subdirectory (lib/evpp)
target_link_libraries (tinychain_cpp evpp)
link_directories (lib/evpp)

include("${PROJECT_SOURCE_DIR}/lib/evpp/build/evpp/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/apps/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/labs/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/test/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/examples/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/benchmark/cmake_install.cmake")
<<<<<<< HEAD
=======

>>>>>>> c844b6a2954fbf1b590fbdeea0cbd88053324983
