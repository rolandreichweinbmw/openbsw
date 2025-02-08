// Copyright 2024 Accenture.

#ifndef GUARD_B6F27E3D_F3D0_4FD9_A1F7_54049110A24C
#define GUARD_B6F27E3D_F3D0_4FD9_A1F7_54049110A24C

#include "transport/ITransportMessageProvidingListener.h"

#include <etl/span.h>

#include <gmock/gmock.h>

#include <set>

namespace transport
{
class TransportMessageProvidingListenerMock : public ITransportMessageProvidingListener
{
public:
    TransportMessageProvidingListenerMock(bool defaultImpl = true);

    virtual ~TransportMessageProvidingListenerMock(){};

    MOCK_METHOD3(
        messageReceived,
        ReceiveResult(
            uint8_t sourceBusId,
            TransportMessage& transportMessage,
            ITransportMessageProcessedListener* pNotificationListener));

    MOCK_METHOD6(
        getTransportMessage,
        ErrorCode(
            uint8_t srcBusId,
            uint16_t sourceId,
            uint16_t targetId,
            uint16_t size,
            ::etl::span<uint8_t const> const& peek,
            TransportMessage*& pTransportMessage));

    MOCK_METHOD1(releaseTransportMessage, void(TransportMessage& transportMessage));

    MOCK_METHOD1(refuseSourceId, void(uint8_t sourceId));

    MOCK_METHOD1(refuseTargetId, void(uint8_t targetId));

    MOCK_METHOD0(dump, void());

    ErrorCode getTransportMessageImplementation(
        uint8_t srcBusId,
        uint16_t sourceId,
        uint16_t targetId,
        uint16_t size,
        ::etl::span<uint8_t const> const& peek,
        TransportMessage*& pTransportMessage);
    ReceiveResult messageReceivedImplementation(
        uint8_t sourceBusId,
        TransportMessage& transportMessage,
        ITransportMessageProcessedListener* pNotificationListener);
    void releaseTransportMessageImplementation(TransportMessage& transportMessage);

private:
    void refuseSourceIdImplementation(uint8_t sourceId);
    void refuseTargetIdImplementation(uint8_t targetId);

    std::set<uint8_t> fRefusedSourceIds;
    std::set<uint8_t> fRefusedTargetIds;
};

} // namespace transport

#endif /* GUARD_B6F27E3D_F3D0_4FD9_A1F7_54049110A24C */
