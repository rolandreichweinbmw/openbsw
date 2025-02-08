// Copyright 2024 Accenture.

#ifndef GUARD_226CD74E_9691_480F_80CB_967EF19AAC26
#define GUARD_226CD74E_9691_480F_80CB_967EF19AAC26

#include "docan/datalink/DoCanCanDataLinkLayer.h"

#include <etl/span.h>

namespace docan
{
/**
 * helper class for encoding/decoding frame payloads with normal addressing.
 * \tparam MessageSize message size to be used for data link layer
 * \tparam FrameSize frame size to be used for data link layer
 */
template<class MessageSize = uint16_t, class FrameSize = uint8_t, class FrameIndex = uint16_t>
class DoCanNormalAddressing
{
public:
    using DataLinkLayerType   = DoCanCanDataLinkLayer<MessageSize, FrameSize, FrameIndex>;
    using DataLinkAddressType = typename DataLinkLayerType::AddressType;

    /**
     * Decode reception address from received CAN frame.
     * \param canId CAN identifier
     * \param payload reference to payload
     */
    static DataLinkAddressType
    decodeReceptionAddress(uint32_t canId, ::etl::span<uint8_t const> const& payload);

    /**
     * Encode transmission address into CAN frame identifier and payload.
     * \param transmissionAddress transmission address to encode
     * \param canId reference to can identifier to be set
     * \param payload reference to payload to encode
     */
    static void encodeTransmissionAddress(
        uint32_t transmissionAddress, uint32_t& canId, ::etl::span<uint8_t> const& payload);
};

/**
 * Inline implementations.
 */
template<class MessageSize, class FrameSize, class FrameIndex>
inline typename DoCanNormalAddressing<MessageSize, FrameSize, FrameIndex>::DataLinkAddressType
DoCanNormalAddressing<MessageSize, FrameSize, FrameIndex>::decodeReceptionAddress(
    uint32_t const canId, ::etl::span<uint8_t const> const& /*payload*/)
{
    return canId;
}

template<class MessageSize, class FrameSize, class FrameIndex>
inline void DoCanNormalAddressing<MessageSize, FrameSize, FrameIndex>::encodeTransmissionAddress(
    uint32_t const transmissionAddress, uint32_t& canId, ::etl::span<uint8_t> const& /*payload*/)
{
    canId = transmissionAddress;
}
} // namespace docan

#endif // GUARD_226CD74E_9691_480F_80CB_967EF19AAC26
