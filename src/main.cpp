#include <iostream>
#include <thread>
#include <array>

#include "MyQueue.hpp"
#include "BinanceSBEWebSocket.hpp"
#include "Decoder.hpp"

constexpr auto cpu1 = 3;
constexpr auto cpu2 = 4;

static void pinThread(int cpu) {
    if (cpu < 0) {
        return;
    }
    ::cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (::pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == -1) {
        std::perror("pthread_setaffinity_rp");
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    constexpr auto queue_capacity = 1'000'000;
    constexpr auto iterations = 1'000;

    DataDecoder decoder;
    
    std::string api_key = "gRPiT4IzLmzQ3VK7TJbXV7jE8TB8rSsqAxJqvAR0wSPDkjPnhC10fQvhJFHjSo31";
    std::string host = "stream-sbe.binance.com";
    std::string port = "9443";
    // std::string path = "/ws/btcusdt@bestBidAsk";
    std::string path = "/stream?streams=btcusdt@bestBidAsk/ethusdt@bestBidAsk/bnbusdt@bestBidAsk/adausdt@bestBidAsk/xrpusdt@bestBidAsk";

    WebSocket ws(api_key);
    ws.connect(host, port, path);
    
    MyQueue<RawBlock> queue(queue_capacity);

    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread producer([&queue, &ws]() {
        pinThread(cpu1);
        RawBlock block{};
        for (uint64_t i = 0; i < iterations; ++i) {
            ws.read_binary(block);
            while (!queue.push(block)) {
                // Spin until push succeeds
            }
        }
    });

    std::thread consumer([&queue, &ws, &decoder]() {
        pinThread(cpu2);
        uint64_t count = 0;
        RawBlock value;
        while (count < iterations) {
            if (queue.pop(value)) {
                ++count;
                auto decoded_header = decoder.DecodeMessageHeader(value.data);
                if(decoded_header.templateId == 10001) {
                    auto decoded_event = decoder.DecodeBestBidAskStreamEvent(value.data);
                    std::cout << "Event Time: " << decoded_event.eventTime
                            << ", Book Update ID: " << decoded_event.bookUpdateId
                            << ", Bid Price: " << decoded_event.bidPrice
                            << ", Ask Price: " << decoded_event.askPrice
                            << ", Symbol: " << decoded_event.symbol << std::endl;
                }
            }
        }
    });

    producer.join();
    consumer.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    auto ops_per_second = (iterations * 1000.0) / duration.count();
    std::cout << "Time: " << duration.count() << " ms\n";
    std::cout << "Operations: " << iterations << "\n";
    std::cout << "Throughput: " << ops_per_second / 1e6 << " M ops/sec\n";
    
    return 0;
}