# MyQueue

A C++ library for decoding Binance WebSocket streams using Simple Binary Encoding (SBE).

## Overview

MyQueue provides real-time market data decoding from Binance WebSocket streams, supporting multiple event types with efficient binary encoding. Uses a lock free SPSC queue implementation from [Presentation of Charles Frasch at cppcon2023](https://github.com/CharlesFrasch/cppcon2023) modified to support 128 byte-lenght blocks.

## Features

- **WebSocket Integration**: `BinanceSBEWebSocket` for stream connection handling
- **SBE Decoding**: Efficient binary message decoding
- **Multiple Stream Events**:
    - Best Bid/Ask data (`BestBidAskStreamEvent`)
    - Depth snapshots (`DepthSnapshotStreamEvent`)
    - Depth differences (`DepthDiffStreamEvent`)
    - Trade information (`TradesStreamEvent`)
- **Queue Management**: Core queue operations in `MyQueue.hpp`

## Project Structure

```
MyQueue/
├── include/          # Header files
│   ├── BinanceSBEWebSocket.hpp
│   ├── Decoder.hpp
│   └── MyQueue.hpp
├── src/              # Implementation
│   ├── BinanceSBEWebSocket.cpp
│   ├── Decoder.cpp
│   └── main.cpp
└── generated/        # SBE-generated code
        └── spot_stream/  # Market data schemas
```

## Building

Create a `build` directory and run CMake:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the executable in the build directory
```bash
./myqueue
```
