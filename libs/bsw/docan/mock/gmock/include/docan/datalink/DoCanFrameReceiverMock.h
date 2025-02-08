// Copyright 2024 Accenture.

#ifndef GUARD_1D7D7314_A980_4A40_A383_A66A0DDEE0DF
#define GUARD_1D7D7314_A980_4A40_A383_A66A0DDEE0DF

#include "docan/common/DoCanConstants.h"
#include "docan/datalink/IDoCanDataFrameTransmitter.h"

#include <gmock/gmock.h>

namespace docan
{
/**
 * Interface for DoCan frame receiver.
 * \tparam DataLinkLayer class providing data link functionality
 */
template<class DataLinkLayer>
class DoCanFrameReceiverMock : public IDoCanFrameReceiver<DataLinkLayer>
{
public:
    using DataLinkAddressType = typename DataLinkLayer::AddressType;
    using MessageSizeType     = typename DataLinkLayer::MessageSizeType;
    using FrameSizeType       = typename DataLinkLayer::FrameSizeType;
    using FrameIndexType      = typename DataLinkLayer::FrameIndexType;
    using ConnectionType      = DoCanConnection<DataLinkLayer>;

    MOCK_METHOD5_T(
        firstDataFrameReceived,
        void(
            ConnectionType const& connection,
            MessageSizeType messageSize,
            FrameIndexType frameCount,
            FrameSizeType consecutiveFrameDataSize,
            ::etl::span<uint8_t const> const& data));
    MOCK_METHOD3_T(
        consecutiveDataFrameReceived,
        void(
            DataLinkAddressType receptionAddress,
            uint8_t sequenceNumber,
            ::etl::span<uint8_t const> const& data));
    MOCK_METHOD4_T(
        flowControlFrameReceived,
        void(
            DataLinkAddressType receptionAddress,
            FlowStatus flowStatus,
            uint8_t blockSize,
            uint8_t encodedMinSeparationTime));
};

} // namespace docan

#endif // GUARD_1D7D7314_A980_4A40_A383_A66A0DDEE0DF
