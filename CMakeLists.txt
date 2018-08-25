cmake_minimum_required(VERSION 3.9)
project(tinychain_cpp)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_LIST_DIR}/cmake
        ${CMAKE_MODULE_PATH})

add_executable(tinychain_cpp src/main.cpp src/Params.h src/OutPoint.h src/TxOut.h src/TxIn.h src/UnspentTxOut.h src/Transaction.h src/TxoutForTxin.h src/Block.h src/LocatedBlock.h src/Utility.h src/Utility.cpp src/UtxoSet.h src/Mempool.h src/TinyChain.h src/TinyChain.cpp src/Connection.h src/Server.h src/Client.h src/Message.h /usr/local/include/secp256k1.h)

#OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(tinychain_cpp OpenSSL::SSL)

#GMP
find_library(GMP_LIBRARY gmp /usr/lib)
target_link_libraries(tinychain_cpp ${GMP_LIBRARY})

#SECP256K1
find_library(SECP256K1_LIBRARY secp256k1 /usr/lib)
target_link_libraries(tinychain_cpp ${SECP256K1_LIBRARY})

#Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.45.0 COMPONENTS filesystem regex thread)

if(Boost_FOUND)
    target_link_libraries(tinychain_cpp  Boost::filesystem Boost::regex Boost::thread boost_coroutine)
endif()

#json parser
add_subdirectory (lib/json)
target_link_libraries (tinychain_cpp nlohmann_json)
link_directories (lib/json)

#secp256k1
include_directories(lib/secp256k1/src)
include_directories(lib/secp256k1)

#GMP
#set(CMAKE_C_FLAGS "-lgmp -lgmpn -lsecp256k1")

#GMP, secp256k1, boost