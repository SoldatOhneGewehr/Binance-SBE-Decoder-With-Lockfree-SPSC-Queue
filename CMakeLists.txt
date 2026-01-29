cmake_minimum_required(VERSION 3.15)
project(MyQueue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add executable
add_executable(myqueue
    src/main.cpp
    src/BinanceSBEWebSocket.cpp
)

# Link libraries
target_link_libraries(myqueue
    PRIVATE
        Boost::system
        OpenSSL::SSL
        OpenSSL::Crypto
        pthread
)

# Compiler flags for performance
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(myqueue PRIVATE -O3 -march=native)
endif()