cmake_minimum_required(VERSION 3.12)
project(tinychain_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(tinychain_cpp src/main.cpp src/Block.cpp src/Block.h src/OutPoint.cpp src/OutPoint.h src/Params.h src/sha256.cpp src/sha256.h src/Transaction.cpp src/Transaction.h src/TxIn.cpp src/TxIn.h src/TxOut.cpp src/TxOut.h src/UnspentTxOut.cpp src/UnspentTxOut.h)

add_subdirectory (lib/evpp)
target_link_libraries (tinychain_cpp evpp)
link_directories (lib/evpp)

include("${PROJECT_SOURCE_DIR}/lib/evpp/build/evpp/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/apps/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/labs/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/test/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/examples/cmake_install.cmake")
include("${PROJECT_SOURCE_DIR}/lib/evpp/build/benchmark/cmake_install.cmake")

include_directories("${PROJECT_SOURCE_DIR}/lib/cereal/include")