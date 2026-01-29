#include "Decoder.hpp"

using Buffer = std::array<std::byte, 128>;

MessageHeaderDecoded DataDecoder::DecodeMessageHeader(Buffer buffer)
{
    spot_stream::MessageHeader header(reinterpret_cast<char*>(buffer.data()), buffer.size());
    
    MessageHeaderDecoded decoded;
    decoded.blockLength = header.blockLength();
    decoded.templateId = header.templateId();
    decoded.schemaId = header.schemaId();
    decoded.version = header.version();

    return decoded;
}

BestBidAskStreamEventDecoded DataDecoder::DecodeBestBidAskStreamEvent(Buffer buffer)
{
    spot_stream::MessageHeader header(reinterpret_cast<char*>(buffer.data()), buffer.size());
    
    spot_stream::BestBidAskStreamEvent event;
    event.wrapForDecode(
        reinterpret_cast<char*>(buffer.data()),
        spot_stream::MessageHeader::encodedLength(),
        header.blockLength(),
        header.version(),
        buffer.size()
    );

    BestBidAskStreamEventDecoded decoded;
    decoded.eventTime = event.eventTime();
    decoded.bookUpdateId = event.bookUpdateId();
    decoded.priceExponent = event.priceExponent();
    decoded.qtyExponent = event.qtyExponent();
    decoded.bidPrice = event.bidPrice();
    decoded.bidQty = event.bidQty();
    decoded.askPrice = event.askPrice();
    decoded.askQty = event.askQty();
    decoded.symbol = event.getSymbolAsString();
    
    return decoded;
}