#ifndef DECODER_HPP
#define DECODER_HPP

#include <array>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include "BestBidAskStreamEvent.h"
#include "DepthDiffStreamEvent.h"
#include "DepthSnapshotStreamEvent.h"
#include "TradesStreamEvent.h"
#include "MessageHeader.h"
#include "GroupSize16Encoding.h"
#include "GroupSizeEncoding.h"
#include "VarString8.h"

using Buffer = std::array<std::byte, 128>;


struct MessageHeaderDecoded {
    std::uint16_t blockLength;
    std::uint16_t templateId;
    std::uint16_t schemaId;
    std::uint16_t version;
};

struct BestBidAskStreamEventDecoded {
    std::int64_t eventTime;
    std::int64_t bookUpdateId;
    std::int8_t priceExponent;
    std::int8_t qtyExponent;
    std::int64_t bidPrice;
    std::int64_t bidQty;
    std::int64_t askPrice;
    std::int64_t askQty;
    std::string symbol;
};

struct DepthDiffStreamEventDecoded {

};

class DataDecoder
{
public:
    DataDecoder() = default;
    MessageHeaderDecoded DecodeMessageHeader(Buffer buffer);
    BestBidAskStreamEventDecoded DecodeBestBidAskStreamEvent(Buffer buffer);
    DepthDiffStreamEventDecoded DecodeDepthDiffStreamEvent(Buffer buffer);

private:
    spot_stream::BestBidAskStreamEvent bbase;
    spot_stream::BoolEnum be;
    spot_stream::DepthDiffStreamEvent ddse;
    spot_stream::DepthSnapshotStreamEvent dsse;
    spot_stream::GroupSize16Encoding gs16e;
    spot_stream::GroupSizeEncoding gse;
    spot_stream::MessageHeader mh;
    spot_stream::TradesStreamEvent tse;
    spot_stream::VarString8 vs8;
};

#endif // DECODER_HPP