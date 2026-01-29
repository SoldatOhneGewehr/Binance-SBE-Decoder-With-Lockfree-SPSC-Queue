#pragma once

#include <cstring>
#include <vector>
#include "../generated/spot_stream/BestBidAskStreamEvent.h"
#include "../generated/spot_stream/DepthDiffStreamEvent.h"
#include "../generated/spot_stream/DepthSnapshotStreamEvent.h"
#include "../generated/spot_stream/TradesStreamEvent.h"
#include "../generated/spot_stream/MessageHeader.h"

class DataEncoder {
public:
    DataEncoder() : buffer_(1024 * 64), position_(0) {}

    // Encode BestBidAskStreamEvent
    void encode(const BestBidAskStreamEvent& event) {
        encodeMessageHeader();
        // Encode event-specific fields
        appendBytes(&event, sizeof(BestBidAskStreamEvent));
    }

    // Encode DepthDiffStreamEvent
    void encode(const DepthDiffStreamEvent& event) {
        encodeMessageHeader();
        appendBytes(&event, sizeof(DepthDiffStreamEvent));
    }

    // Encode DepthSnapshotStreamEvent
    void encode(const DepthSnapshotStreamEvent& event) {
        encodeMessageHeader();
        appendBytes(&event, sizeof(DepthSnapshotStreamEvent));
    }

    // Encode TradesStreamEvent
    void encode(const TradesStreamEvent& event) {
        encodeMessageHeader();
        appendBytes(&event, sizeof(TradesStreamEvent));
    }

    const uint8_t* getBuffer() const { return buffer_.data(); }
    size_t getSize() const { return position_; }

    void reset() { position_ = 0; }

private:
    std::vector<uint8_t> buffer_;
    size_t position_;

    void encodeMessageHeader() {
        // Encode header if needed
    }

    void appendBytes(const void* data, size_t length) {
        if (position_ + length > buffer_.size()) {
            buffer_.resize(buffer_.size() * 2);
        }
        std::memcpy(buffer_.data() + position_, data, length);
        position_ += length;
    }
};