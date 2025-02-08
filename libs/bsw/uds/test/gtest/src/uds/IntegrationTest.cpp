// Copyright 2024 Accenture.

/**
 * Contains a UDS integration test.
 */
#include "common/busid/BusId.h"
#include "transport/AbstractTransportLayerMock.h"
#include "transport/TransportConfiguration.h"
#include "transport/TransportMessage.h"
#include "transport/TransportMessageListenerMock.h"
#include "transport/TransportMessageProcessedListenerMock.h"
#include "transport/TransportMessageProviderMock.h"
#include "transport/TransportMessageWithBuffer.h"
#include "uds/DiagDispatcher.h"
#include "uds/DiagnosisConfiguration.h"
#include "uds/UdsLogger.h"
#include "uds/application/AbstractDiagApplication.h"
#include "uds/base/AbstractDiagJobMock.h"
#include "uds/connection/DiagConnectionManager.h"
#include "uds/connection/IncomingDiagConnectionMock.h"
#include "uds/connection/OutgoingDiagConnectionMock.h"
#include "uds/lifecycle/UdsLifecycleConnectorMock.h"
#include "uds/services/ecureset/EnableRapidPowerShutdown.h"
#include "uds/services/ecureset/HardReset.h"
#include "uds/services/ecureset/PowerDown.h"
#include "uds/services/ecureset/SoftReset.h"
#include "uds/services/readdata/ReadDataByIdentifier.h"
#include "uds/session/ApplicationDefaultSession.h"
#include "uds/session/ApplicationExtendedSession.h"
#include "uds/session/DiagSessionManagerMock.h"
#include "util/estd/function_mock.h"
#include "util/logger/ComponentMappingMock.h"
#include "util/logger/Logger.h"
#include "util/logger/LoggerOutputMock.h"

#include <async/AsyncMock.h>
#include <async/TestContext.h>
#include <etl/array.h>

#include <gmock/gmock.h>
#include <gtest/esr_extensions.h>

#define CONTEXT_EXECUTE fContext.execute()

namespace
{
using namespace uds;
using namespace ::util::logger;
using namespace ::testing;
using namespace ::transport::test;

static constexpr uint16_t TESTER_ID = 0xF0;

MATCHER_P(SameAddress, n, "")
{
    *result_listener << "addresses don't match: actual: " << &arg << ", expected: " << n;
    return &arg == n;
}

/*
 *
 * Implementation of MyReadDataByIdentifier
 *
 */
class MyReadDataByIdentifier : public uds::AbstractDiagJob
{
public:
    MyReadDataByIdentifier()
    : AbstractDiagJob(
        IMPLEMENTED_REQUEST,
        sizeof(IMPLEMENTED_REQUEST),
        1,
        DiagSession::DiagSessionMask::getInstance() << DiagSession::APPLICATION_DEFAULT_SESSION())
    {}

    uds::DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override
    {
        Logger::debug(UDS, "MyReadDataByIdentifier::verify()");
        if (!compare(request, getImplementedRequest() + 1, 2))
        {
            return uds::DiagReturnCode::NOT_RESPONSIBLE;
        }
        return uds::DiagReturnCode::OK;
    }

    uds::DiagReturnCode::Type process(
        uds::IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override
    {
        Logger::debug(UDS, "MyReadDataByIdentifier::process()");
        uds::PositiveResponse& response = connection.releaseRequestGetResponse();
        response.appendUint8(1);
        response.appendUint8(2);
        response.appendUint8(3);
        connection.sendPositiveResponseInternal(response.getLength(), *this);
        return uds::DiagReturnCode::OK;
    }

private:
    static uint8_t const IMPLEMENTED_REQUEST[3];
};

uint8_t const MyReadDataByIdentifier::IMPLEMENTED_REQUEST[3] = {0x22, 0x01, 0x01};

/**
 * MyDiagnosisApplication is a class that provides a specific implementation
 * of the AbstractDiagApplication class. This class gives the opportunity to
 * create outgoing request messages and request outgoing diagnose connections.
 */
class MyDiagnosisApplication : public uds::AbstractDiagApplication
{
public:
    explicit MyDiagnosisApplication(IOutgoingDiagConnectionProvider& connectionProvider)
    : AbstractDiagApplication(connectionProvider)
    {}

    ::uds::ErrorCode sendEcuReset(uint8_t ecuId)
    {
        return sendEcuResetService(
            0x01,   // Subservice HardReset
            ecuId); // Target EcuId
    }

    void createRequestMessageForEcuReset(
        uint8_t ecuId, uds::OutgoingDiagConnection*& pCurrentOutgoingConnection)
    {
        createRequestMessageForEcuResetService(
            0x01,  // Subservice HardReset
            ecuId, // Target EcuId
            pCurrentOutgoingConnection);
    }

    void createRequestMessageForPowerdown(uds::OutgoingDiagConnection*& pCurrentOutgoingConnection)
    {
        createRequestMessageForEcuResetService(
            0x41,                                       // Subservice PowerDown
            DiagCodes::FUNCTIONAL_ID_ALL_ISO14229_ECUS, // Target Functional
            pCurrentOutgoingConnection);
    }

    void responseReceived(
        OutgoingDiagConnection& connection,
        uint8_t sourceDiagAddress,
        uint8_t const response[],
        uint16_t responseLength) override
    {
        // nothing to do, we don't expect responses
    }

    void responseTimeout(OutgoingDiagConnection& connection) override
    {
        // nothing to do, we don't expect responses
    }

    virtual void requestSent(OutgoingDiagConnection& connection, RequestSendResult result) {}

private:
    ::uds::ErrorCode sendEcuResetService(uint8_t subService, uint8_t targetId)
    {
        uint8_t const REQUEST_LENGTH                            = 9;
        uds::OutgoingDiagConnection* pCurrentOutgoingConnection = nullptr;

        createRequestMessageForEcuResetService(subService, targetId, pCurrentOutgoingConnection);

        return pCurrentOutgoingConnection->sendDiagRequest(REQUEST_LENGTH, *this);
    }

    void createRequestMessageForEcuResetService(
        uint8_t subService,
        uint8_t targetId,
        uds::OutgoingDiagConnection*& pCurrentOutgoingConnection)
    {
        getOutgoingDiagConnection(TESTER_ID, pCurrentOutgoingConnection);

        uint8_t* requestBuffer;
        uint16_t requestBufferLength;
        pCurrentOutgoingConnection->getRequestBuffer(requestBuffer, requestBufferLength);

        // first the parameters for the TAS
        requestBuffer[0] = 0x31; // Request Routine Control
        requestBuffer[1] = 0x01; // Id Routine Control: Start Routine
        requestBuffer[2] = 0x0F; // Routine Identifier (MSB)
        requestBuffer[3] = 0x0B; // Routine Identifier (LSB)
        // now the diag request the TAS should execute
        requestBuffer[4] = targetId;
        requestBuffer[5] = 0x00; // length (MSB)
        requestBuffer[6] = 0x02; // length (LSB)
        requestBuffer[7] = 0x11; // Service ID: request ecu reset
        requestBuffer[8] = subService;
    }
};

/*
 *
 * Implementation of UdsIntegration
 *
 */
class UdsIntegration : public Test
{
protected:
    static uint8_t const ECU_UDS_ADDRESS           = 0x10;
    static uint8_t const NUM_INCOMING_CONNECTIONS  = 1;
    static uint8_t const NUM_OUTGOING_CONNECTIONS  = 2;
    static uint8_t const MAX_NUM_INCOMING_MESSAGES = 10;

    static uint8_t const ECU_RESET        = 0x11U;
    static uint8_t const HARD_RESET       = 0x01U;
    static uint8_t const SOFT_RESET       = 0x03U;
    static uint8_t const RAPID_POWER_DOWN = 0x04U;
    static uint8_t const POWER_DOWN       = 0x41U;

    static uint8_t const SOURCE_ID = 0xF1U;
    static uint8_t const TARGET_ID = 0x10U;

    static uint16_t const EMPTY_RESPONSE    = 0x0U;
    static uint8_t const SUBFUNCTION_LENGTH = 1U;

    UdsIntegration()
    : fContext(2U)
    , _udsConfiguration(
          ECU_UDS_ADDRESS,
          0xDF,
          0u,
          transport::TransportConfiguration::DIAG_PAYLOAD_SIZE,
          true,
          false,
          true,
          fContext)
    , _udsConfiguration2(
          ECU_UDS_ADDRESS,
          transport::TransportMessage::INVALID_ADDRESS,
          0u,
          transport::TransportConfiguration::DIAG_PAYLOAD_SIZE,
          true,
          true,
          true,
          fContext)
    , _sessionManager()
    , _jobRoot()
    , _messageListener()
    , _messageProvider()
    , _messageProcessedListener()
    , _incomingDiagConnection(fContext)
    , _lifecycle()
    , _udsDispatcher(_udsConfiguration, _sessionManager, _jobRoot, fContext)
    , _udsDispatcher2(_udsConfiguration2, _sessionManager, _jobRoot, fContext)
    , _rdbi()
    , _myRdbi()
    , _hardReset(_lifecycle, _udsDispatcher)
    , _softReset(_lifecycle, _udsDispatcher)
    , _powerDown(_lifecycle)
    , _enableRapidPowerShutdown(_lifecycle)
    , _outgoingDiagConnectionProvider(_udsDispatcher)
    , _myDiagnosisApplication(_outgoingDiagConnectionProvider)
    , _outgoingSender(0u)
    , _diagConnectionManager(
          _udsConfiguration, _outgoingSender, _messageProvider, fContext, _udsDispatcher)
    , pTransportLayer(nullptr)
    {
        fContext.handleAll();
        uds::AbstractDiagJob::setDefaultDiagSessionManager(_sessionManager);
        _udsDispatcher.setTransportMessageListener(&_messageListener);
        _udsDispatcher.setTransportMessageProvider(&_messageProvider);
        _udsDispatcher.addAbstractDiagJob(_rdbi);
        _udsDispatcher.addAbstractDiagJob(_myRdbi);
        _udsDispatcher.addAbstractDiagJob(_hardReset);
        _udsDispatcher.addAbstractDiagJob(_softReset);
        _udsDispatcher.addAbstractDiagJob(_powerDown);
        _udsDispatcher.addAbstractDiagJob(_enableRapidPowerShutdown);
    }

    ~UdsIntegration() override
    {
        _udsDispatcher.removeAbstractDiagJob(_enableRapidPowerShutdown);
        _udsDispatcher.removeAbstractDiagJob(_powerDown);
        _udsDispatcher.removeAbstractDiagJob(_softReset);
        _udsDispatcher.removeAbstractDiagJob(_hardReset);
        _udsDispatcher.removeAbstractDiagJob(_myRdbi);
        _udsDispatcher.removeAbstractDiagJob(_rdbi);
    }

    async::TestContext fContext;
    uds::DiagnosisConfiguration<NUM_INCOMING_CONNECTIONS, NUM_OUTGOING_CONNECTIONS, 1>
        _udsConfiguration;
    uds::DiagnosisConfiguration<NUM_INCOMING_CONNECTIONS, NUM_OUTGOING_CONNECTIONS, 1>
        _udsConfiguration2;
    StrictMock<uds::DiagSessionManagerMock> _sessionManager;
    DiagJobRoot _jobRoot;
    StrictMock<transport::TransportMessageListenerMock> _messageListener;
    StrictMock<transport::TransportMessageProviderMock> _messageProvider;
    StrictMock<transport::TransportMessageProcessedListenerMock> _messageProcessedListener;
    StrictMock<uds::IncomingDiagConnectionMock> _incomingDiagConnection;
    StrictMock<UdsLifecycleConnectorMock> _lifecycle;
    uds::DiagDispatcher2 _udsDispatcher;
    uds::DiagDispatcher2 _udsDispatcher2;
    uds::ReadDataByIdentifier _rdbi;
    MyReadDataByIdentifier _myRdbi;
    uds::HardReset _hardReset;
    uds::SoftReset _softReset;
    uds::PowerDown _powerDown;
    uds::EnableRapidPowerShutdown _enableRapidPowerShutdown;
    IOutgoingDiagConnectionProvider& _outgoingDiagConnectionProvider;
    MyDiagnosisApplication _myDiagnosisApplication;
    StrictMock<transport::AbstractTransportLayerMock> _outgoingSender;
    uds::DiagConnectionManager _diagConnectionManager;
    transport::AbstractTransportLayer* pTransportLayer;
    async::AsyncMock fAsyncMock;

public:
    void shutdownComplete(transport::AbstractTransportLayer& transportLayer)
    {
        pTransportLayer = &transportLayer;
    }
};

TEST_F(UdsIntegration, positive_response)
{
    uint8_t buffer[]           = {0x22, 0x01, 0x01};
    uint8_t expectedResponse[] = {0x62, 0x01, 0x01, 0x01, 0x02, 0x03};

    TransportMessageWithBuffer pRequest(0xF1, 0x10, buffer, sizeof(expectedResponse));
    TransportMessageWithBuffer pResponse(0x10, 0xF1, expectedResponse);

    transport::ITransportMessageProcessedListener* pProcessedListener = nullptr;
    transport::TransportMessage* pMessage                             = nullptr;

    _udsDispatcher.send(*pRequest, &_messageProcessedListener);
    // see DiagDispatcher2::trigger()
    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(2);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_rdbi), NotNull(), 2))
        .With(Args<2, 3>(ElementsAre(0x01, 0x01)))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_myRdbi), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, responseSent(_, uds::DiagReturnCode::OK, NotNull(), 3))
        .With(Args<2, 3>(ElementsAre(0x01, 0x02, 0x03)));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), Eq(ByRef(*pResponse)), NotNull()))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&pMessage)),
            SaveArg<2>(&pProcessedListener),
            Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));

    EXPECT_EQ(_udsDispatcher.getSourceId(), 0x10);

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;

    EXPECT_CALL(
        _messageProcessedListener,
        transportMessageProcessed(
            Eq(ByRef(*pRequest)),
            transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(SameAddress(pMessage)));

    pProcessedListener->transportMessageProcessed(
        *pMessage,
        transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    CONTEXT_EXECUTE;
}

/**
 * \desc
 * A RDBI not found must result in negative response ROOR
 */
TEST_F(UdsIntegration, negative_response_request_out_of_range)
{
    uint8_t buffer[]           = {0x22, 0x01, 0x02};
    uint8_t expectedResponse[] = {0x7F, 0x22, DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE};

    TransportMessageWithBuffer pRequest(0xF1, 0x10, buffer, sizeof(expectedResponse));
    TransportMessageWithBuffer pResponse(0x10, 0xF1, expectedResponse);

    transport::ITransportMessageProcessedListener* pProcessedListener = nullptr;
    transport::TransportMessage* pMessage                             = nullptr;

    _udsDispatcher.send(*pRequest, &_messageProcessedListener);

    // see DiagDispatcher2::trigger()
    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(2);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_rdbi), NotNull(), 2))
        .With(Args<2, 3>(ElementsAre(0x01, 0x02)))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(
        _sessionManager, responseSent(_, DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE, NotNull(), 0));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), Eq(ByRef(*pResponse)), NotNull()))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&pMessage)),
            SaveArg<2>(&pProcessedListener),
            Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;

    EXPECT_CALL(
        _messageProcessedListener,
        transportMessageProcessed(
            SameAddress(pRequest.get()),
            transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(SameAddress(pMessage)));

    pProcessedListener->transportMessageProcessed(
        *pMessage,
        transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    CONTEXT_EXECUTE;
}

/**
 * \desc
 *
 * No response to an incoming NRC
 */
TEST_F(UdsIntegration, no_response_for_7f)
{
    uint8_t buffer[] = {0x7fU, 0x01U, 0x02U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setServiceId(0x7fU);
    transportMessage.setSourceId(0x01U);
    transportMessage.setTargetId(0x02U);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setTransportMessage(transportMessage);
    response1.setProcessedListener(&_udsDispatcher);

    _udsConfiguration.AcceptAllRequests = true;
    _udsConfiguration.acquireIncomingDiagConnection();

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

/**
 * \desc
 * A RDBI in wrong session must result in negative response ROOR
 */
TEST_F(UdsIntegration, negative_response_request_out_of_range_in_wrong_session)
{
    uint8_t buffer[]           = {0x22, 0x01, 0x01};
    uint8_t expectedResponse[] = {0x7F, 0x22, DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE};

    TransportMessageWithBuffer pRequest(0xF1, 0x10, buffer, sizeof(expectedResponse));

    TransportMessageWithBuffer pResponse(0x10, 0xF1, expectedResponse);

    transport::ITransportMessageProcessedListener* pProcessedListener = nullptr;
    transport::TransportMessage* pMessage                             = nullptr;

    _udsDispatcher.send(*pRequest, &_messageProcessedListener);

    // see DiagDispatcher2::trigger()
    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(2);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_rdbi), NotNull(), 2))
        .With(Args<2, 3>(ElementsAre(0x01, 0x01)))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_myRdbi), NotNull(), 2))
        .With(Args<2, 3>(ElementsAre(0x01, 0x01)))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(
        _sessionManager, responseSent(_, DiagReturnCode::ISO_REQUEST_OUT_OF_RANGE, NotNull(), 0));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), Eq(ByRef(*pResponse)), NotNull()))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&pMessage)),
            SaveArg<2>(&pProcessedListener),
            Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR)));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;

    EXPECT_CALL(
        _messageProcessedListener,
        transportMessageProcessed(
            SameAddress(pRequest.get()),
            transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(SameAddress(pMessage)));

    pProcessedListener->transportMessageProcessed(
        *pMessage,
        transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR);
    CONTEXT_EXECUTE;
}

TEST_F(UdsIntegration, OutgoingDiagConnection_sendEcuReset_expect_OK_status)
{
    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    uint8_t ecuId = 0xA0;

    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(1);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, NotNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    EXPECT_EQ(::uds::ErrorCode::OK, _myDiagnosisApplication.sendEcuReset(ecuId));
}

TEST_F(
    UdsIntegration,
    two_OutgoingDiagConnections_create_ecuReset_and_powerdown_requests_send_them_in_reverse_order_\
expect_OK_status)
{
    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    transport::TransportMessage message2;
    ::etl::array<uint8_t, 9> requestBuffer2;
    message2.init(requestBuffer2.data(), requestBuffer2.size());
    transport::TransportMessage* pMessage2 = &message2;

    uint8_t ecuId = 0xA0;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, NotNull()))
        .WillRepeatedly(
            Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    uds::OutgoingDiagConnection* pCurrentOutgoingConnectionForEcuReset  = nullptr;
    uds::OutgoingDiagConnection* pCurrentOutgoingConnectionForPowerdown = nullptr;

    _myDiagnosisApplication.createRequestMessageForEcuReset(
        ecuId, pCurrentOutgoingConnectionForEcuReset);

    memcpy(requestBuffer.data(), pMessage->getBuffer(), requestBuffer.size() * sizeof(uint8_t));

    _myDiagnosisApplication.createRequestMessageForPowerdown(
        pCurrentOutgoingConnectionForPowerdown);

    memcpy(requestBuffer2.data(), pMessage2->getBuffer(), requestBuffer2.size() * sizeof(uint8_t));

    // send first the "powerdown" request and then the "ecu reset" request to
    // not meet the true case "if (pListener == pNotificationListener)"
    // immediately in AbstractTransportLayer::ErrorCode DiagDispatcher2::send(...)
    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_OK,
        _udsDispatcher.send(*pMessage, pCurrentOutgoingConnectionForPowerdown));

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_OK,
        _udsDispatcher.send(*pMessage2, pCurrentOutgoingConnectionForEcuReset));
}

TEST_F(
    UdsIntegration,
    create_OutgoingDiagConnection_sendEcuReset_check_if_message_is_received_expect_TP_SEND_FAIL)
{
    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    uint8_t ecuId = 0xA0;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, NotNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    uds::OutgoingDiagConnection* pCurrentOutgoingConnectionForEcuReset = nullptr;

    _myDiagnosisApplication.createRequestMessageForEcuReset(
        ecuId, pCurrentOutgoingConnectionForEcuReset);

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        _udsDispatcher.send(*pMessage, pCurrentOutgoingConnectionForEcuReset));
}

TEST_F(
    UdsIntegration,
    a_TransportMessage_sent_by_an_IncomingDiagConnection_is_not_received_completely_so_\
TP_SEND_FAIL_is_expected)
{
    uint8_t buffer[]           = {0x22, 0x01, 0x01};
    uint8_t expectedResponse[] = {0x62, 0x01, 0x01, 0x01, 0x02, 0x03};

    TransportMessageWithBuffer pRequest(0xF1, 0x10, buffer, sizeof(expectedResponse));

    TransportMessageWithBuffer pResponse(0x10, 0xF1, expectedResponse);

    transport::ITransportMessageProcessedListener* pProcessedListener = nullptr;
    transport::TransportMessage* pMessage                             = nullptr;

    _udsDispatcher.send(*pRequest, &_messageProcessedListener);

    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(2);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_rdbi), NotNull(), 2))
        .With(Args<2, 3>(ElementsAre(0x01, 0x01)))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, acceptedJob(_, Ref(_myRdbi), NotNull(), 0))
        .WillOnce(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, responseSent(_, uds::DiagReturnCode::OK, NotNull(), 3))
        .With(Args<2, 3>(ElementsAre(0x01, 0x02, 0x03)));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), Eq(ByRef(*pResponse)), NotNull()))
        .WillOnce(DoAll(
            WithArg<1>(SaveRef<0>(&pMessage)),
            SaveArg<2>(&pProcessedListener),
            Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR)));

    EXPECT_CALL(
        _messageProcessedListener,
        transportMessageProcessed(
            SameAddress(pRequest.get()),
            transport::ITransportMessageProcessedListener::ProcessingResult::PROCESSED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(*pResponse));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    send_returns_TP_SEND_FAIL_if_target_address_is_not_diagAddress_and_broadcastAddress_is_not_DF)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x15U, buffer, sizeof(expectedResponse));

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        _udsDispatcher.send(*pRequest, &_messageProcessedListener));
}

TEST_F(UdsIntegration, send_calls_enqueueMessage_and_return_TP_SEND_FAIL_if_isEnabled_return_false)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0xDFU, buffer, sizeof(expectedResponse));

    _udsDispatcher.disable();

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        _udsDispatcher.send(*pRequest, &_messageProcessedListener));
}

TEST_F(
    UdsIntegration,
    send_calls_enqueueMessage_and_return_TP_MESSAGE_INCOMPLETE_if_isComplete_return_false)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0xDFU, buffer, sizeof(expectedResponse));
    pRequest->resetValidBytes();

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_MESSAGE_INCOMPLETE,
        _udsDispatcher.send(*pRequest, &_messageProcessedListener));
}

TEST_F(
    UdsIntegration, send_calls_enqueueMessage_and_return_TP_QUEUE_FULL_if_jobQueue_is_already_full)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0xDFU, buffer, sizeof(expectedResponse));

    _udsConfiguration.SendJobQueue.emplace();

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_QUEUE_FULL,
        _udsDispatcher.send(*pRequest, &_messageProcessedListener));
}

TEST_F(UdsIntegration, send_calls_enqueueMessage_and_return_TP_OK_if_jobQueue_is_not_full)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x15U, buffer, sizeof(expectedResponse));

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_OK, _udsDispatcher2.send(*pRequest, 0UL));
}

TEST_F(UdsIntegration, resume_returns_TP_SEND_FAIL_if_the_TargetId_is_not_the_DiagAddress)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x15U, buffer, sizeof(expectedResponse));

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_SEND_FAIL,
        _udsDispatcher.resume(*pRequest, &_messageProcessedListener));
}

TEST_F(UdsIntegration, resume_returns_TP_OK_if_the_TargetId_and_DiagAddress_are_identically)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    EXPECT_EQ(
        transport::AbstractTransportLayer::ErrorCode::TP_OK,
        _udsDispatcher.resume(*pRequest, &_messageProcessedListener));
}

TEST_F(UdsIntegration, dispatchTriggerEventRequest_returns_1_if_jobQueue_is_full)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer transportMessage(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    _udsConfiguration.SendJobQueue.emplace();

    EXPECT_EQ(1, _udsDispatcher.dispatchTriggerEventRequest(*transportMessage));
}

TEST_F(
    UdsIntegration,
    dispatchTriggerEventRequest_calls_sendBusyResponse_if_sendBusyNegativeResponse_is_true)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer transportMessage(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    EXPECT_EQ(1, _udsDispatcher.dispatchTriggerEventRequest(*transportMessage));
}

TEST_F(
    UdsIntegration,
    dispatchTriggerEventRequest_calls_execute_and_terminate_the_connection_if_DiagReturnCode_is_neq_OK)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer transportMessage(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    EXPECT_CALL(fAsyncMock, schedule(_, _, _, _, _)).Times(2);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_sessionManager, responseSent(_, _, _, _));

    EXPECT_CALL(_messageListener, messageReceived(_, _, _))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(SameAddress(pMessage)));

    EXPECT_EQ(0, _udsDispatcher.dispatchTriggerEventRequest(*transportMessage));
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration, dispatchTriggerEventRequest_returns_1_if_no_IncomingConnection_can_be_requested)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer transportMessage(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    StrictMock<estd::function_mock<void()>> initComplete;
    EXPECT_CALL(initComplete, callee());

    _udsDispatcher.getConnectionManager().shutdown(initComplete);

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_EXTENDED_SESSION()));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    EXPECT_EQ(1, _udsDispatcher.dispatchTriggerEventRequest(*transportMessage));
}

TEST_F(UdsIntegration, sendBusyResponse_calls_an_error_log_message_if_messageReceived_fails)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer transportMessage(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    StrictMock<ComponentMappingMock> componentMapping;
    StrictMock<LoggerOutputMock> loggerOutput;

    Logger::init(componentMapping, loggerOutput);

    ComponentInfo::PlainInfo component;
    component._nameInfo._string = "UDS";
    ComponentInfo componentInfo(UDS, &component);

    LevelInfo::PlainInfo info;
    info._level            = LEVEL_ERROR;
    info._nameInfo._string = "ERROR";
    LevelInfo levelInfo(&info);

    EXPECT_CALL(componentMapping, isEnabled(UDS, LEVEL_ERROR)).WillRepeatedly(Return(true));

    EXPECT_CALL(componentMapping, getComponentInfo(UDS)).WillRepeatedly(Return(componentInfo));

    EXPECT_CALL(componentMapping, getLevelInfo(LEVEL_ERROR)).WillRepeatedly(Return(levelInfo));

    EXPECT_CALL(
        loggerOutput, logOutput(componentInfo, levelInfo, HasSubstr("request discarded"), _))
        .Times(1);

    EXPECT_CALL(
        loggerOutput,
        logOutput(componentInfo, levelInfo, HasSubstr("Could not send BUSY_REPEAT_REQUEST"), _))
        .Times(1);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillOnce(Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_NO_MSG_AVAILABLE));

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    EXPECT_EQ(1, _udsDispatcher.dispatchTriggerEventRequest(*transportMessage));

    // clean up
    Logger::shutdown();
}

TEST_F(UdsIntegration, dispatchIncomingRequest_returns_earlier_if_request_comes_from_invalid_source)
{
    uint8_t buffer[] = {0x22U, 0x01U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(transport::TransportMessage::INVALID_ADDRESS);

    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();
    response1.setTransportMessage(transportMessage);
    response1.setProcessedListener(&_udsDispatcher);

    EXPECT_CALL(_messageProvider, releaseTransportMessage(Ref(transportMessage)));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    dispatchIncomingRequest_sends_BusyNegativeResponse_successful_and_call_transportMessageProcessed)
{
    uint8_t buffer[] = {0x22U, 0x01U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0xDFU);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setTransportMessage(transportMessage);
    response1.setProcessedListener(&_udsDispatcher);

    _udsConfiguration.AcceptAllRequests = true;
    _udsConfiguration.acquireIncomingDiagConnection();

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    dispatchIncomingRequest_sends_BusyNegativeResponse_if_no_functional_buffer_is_available)
{
    uint8_t buffer[] = {0x22U, 0x01U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0xDFU);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setTransportMessage(transportMessage);
    response1.setProcessedListener(&_udsDispatcher);

    _udsConfiguration.AcceptAllRequests = true;

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_NO_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(UdsIntegration, dispatchIncomingRequest_setProcessedListener_if_no_one_was_set_before)
{
    uint8_t buffer[] = {0x22U, 0x01U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0xDFU);

    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setTransportMessage(transportMessage);

    _udsConfiguration.AcceptAllRequests = true;
    _udsConfiguration.acquireIncomingDiagConnection();

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    dispatchIncomingRequest_calls_releaseTransportMessage_if_a_ProcessedListener_was_set_before)
{
    uint8_t buffer[] = {0x22U, 0x01U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setServiceId(0x31U);
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0xDFU);

    transport::TransportMessage message;
    ::etl::array<uint8_t, 9> requestBuffer;
    message.init(requestBuffer.data(), requestBuffer.size());
    transport::TransportMessage* pMessage = &message;

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(DoAll(
            SetArgReferee<5>(pMessage),
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_OK)));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setProcessedListener(&_udsDispatcher);
    response1.setTransportMessage(transportMessage);

    _udsConfiguration.AcceptAllRequests = true;
    _udsConfiguration.acquireIncomingDiagConnection();

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    dispatchIncomingRequest_isFunctionalAddressed_and_isTesterPresentWithSuppressPosResponse_are_true)
{
    uint8_t buffer[] = {0x3EU, 0x80U, 0x01U};

    transport::TransportMessage transportMessage(buffer, sizeof(buffer));
    transportMessage.setServiceId(0x3EU);
    transportMessage.setSourceId(0x10U);
    transportMessage.setTargetId(0xDFU);

    EXPECT_CALL(_messageProvider, getTransportMessage(_, _, _, _, _, _))
        .WillRepeatedly(
            Return(transport::ITransportMessageProvider::ErrorCode::TPMSG_NOT_RESPONSIBLE));

    transport::TransportJob& response1 = _udsConfiguration.SendJobQueue.emplace();

    response1.setTransportMessage(transportMessage);
    response1.setProcessedListener(&_udsDispatcher);

    _udsConfiguration.AcceptAllRequests = true;
    _udsConfiguration.acquireIncomingDiagConnection();

    EXPECT_CALL(_messageListener, messageReceived(Eq(0u), _, IsNull()))
        .WillOnce(Return(transport::ITransportMessageListener::ReceiveResult::RECEIVED_ERROR));

    EXPECT_CALL(_messageProvider, releaseTransportMessage(_)).Times(AtLeast(1));

    _udsDispatcher.processQueue();
    CONTEXT_EXECUTE;
}

TEST_F(
    UdsIntegration,
    getOutgoingDiagConnection_returns_NO_CONNECTION_AVAILABLE_if_dispatcher_is_disable)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    _udsDispatcher.disable();

    uds::OutgoingDiagConnection* pCurrentOutgoingConnection = nullptr;

    EXPECT_EQ(
        IOutgoingDiagConnectionProvider::NO_CONNECTION_AVAILABLE,
        _udsDispatcher.getOutgoingDiagConnection(
            TESTER_ID, pCurrentOutgoingConnection, pRequest.get()));
}

TEST_F(
    UdsIntegration,
    getOutgoingDiagConnection_returns_status_value_of_requestOutgoingConnection_if_no_connection_exists)
{
    uint8_t buffer[]           = {0x22U, 0x01U, 0x01U};
    uint8_t expectedResponse[] = {0x62U, 0x01U, 0x01U, 0x01U, 0x02U, 0x03U};

    TransportMessageWithBuffer pRequest(0xF1U, 0x10U, buffer, sizeof(expectedResponse));

    uds::OutgoingDiagConnection* pCurrentOutgoingConnection = nullptr;

    EXPECT_EQ(
        IOutgoingDiagConnectionProvider::GENERAL_ERROR,
        _udsDispatcher.getOutgoingDiagConnection(
            TESTER_ID, pCurrentOutgoingConnection, pRequest.get()));
}

TEST_F(UdsIntegration, init)
{
    EXPECT_EQ(transport::AbstractTransportLayer::ErrorCode::TP_OK, _udsDispatcher.init());
}

TEST_F(UdsIntegration, shutdown)
{
    EXPECT_TRUE(pTransportLayer == 0UL);
    EXPECT_FALSE(_udsDispatcher.shutdown(
        transport::AbstractTransportLayer::ShutdownDelegate::
            create<UdsIntegration, &UdsIntegration::shutdownComplete>(*this)));
    EXPECT_TRUE(pTransportLayer != 0UL);
}

TEST_F(
    UdsIntegration,
    HardReset_process_which_is_calling_by_execute_returns_OK_if_ModeChange_is_possible)
{
    uint8_t const hardResetRequest[] = {HARD_RESET};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, hardResetRequest, EMPTY_RESPONSE);

    _incomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_lifecycle, isModeChangePossible()).WillOnce(Return(true));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _hardReset.execute(_incomingDiagConnection, hardResetRequest, sizeof(hardResetRequest)));

    EXPECT_FALSE(_incomingDiagConnection.isBusy());
}

TEST_F(
    UdsIntegration,
    HardReset_process_returns_ISO_CONDITIONS_NOT_CORRECT_if_ModeChange_is_not_possible)
{
    uint8_t const hardResetRequest[] = {HARD_RESET};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, hardResetRequest, EMPTY_RESPONSE);

    _incomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_lifecycle, isModeChangePossible()).WillOnce(Return(false));

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        _hardReset.execute(_incomingDiagConnection, hardResetRequest, sizeof(hardResetRequest)));
}

TEST_F(
    UdsIntegration,
    calling_HardReset_responseSent_will_not_enable_DiagDispatcher_if_LIFECYCLE_is_OK)
{
    _udsDispatcher.disable();
    EXPECT_CALL(_incomingDiagConnection, terminate()).Times(1U);

    EXPECT_CALL(_lifecycle, requestShutdown(IUdsLifecycleConnector::HARD_RESET, _))
        .WillOnce(Return(true));

    _hardReset.responseSent(_incomingDiagConnection, AbstractDiagJob::RESPONSE_SENT);

    EXPECT_FALSE(_udsDispatcher.isEnabled());
}

TEST_F(
    UdsIntegration,
    calling_HardReset_responseSent_will_enable_DiagDispatcher_if_LIFECYCLE_is_not_OK)
{
    _udsDispatcher.disable();
    EXPECT_CALL(_incomingDiagConnection, terminate()).Times(1U);

    EXPECT_CALL(_lifecycle, requestShutdown(IUdsLifecycleConnector::HARD_RESET, _))
        .WillOnce(Return(false));

    _hardReset.responseSent(_incomingDiagConnection, AbstractDiagJob::RESPONSE_SENT);

    EXPECT_TRUE(_udsDispatcher.isEnabled());
}

TEST_F(
    UdsIntegration,
    SoftReset_process_which_is_calling_by_execute_returns_OK_if_ModeChange_is_possible)
{
    uint8_t const softResetRequest[] = {ECU_RESET, SOFT_RESET};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, softResetRequest, EMPTY_RESPONSE);

    _incomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_lifecycle, isModeChangePossible()).WillOnce(Return(true));

    EXPECT_EQ(
        DiagReturnCode::OK,
        _softReset.execute(
            _incomingDiagConnection,
            &softResetRequest[1U], // prefix has been handled by the DiagDispatcher
            SUBFUNCTION_LENGTH));
}

TEST_F(
    UdsIntegration,
    SoftReset_process_returns_ISO_CONDITIONS_NOT_CORRECT_if_ModeChange_is_not_possible)
{
    uint8_t const softResetRequest[] = {ECU_RESET, SOFT_RESET};

    TransportMessageWithBuffer pRequest(SOURCE_ID, TARGET_ID, softResetRequest, EMPTY_RESPONSE);

    _incomingDiagConnection.fpRequestMessage = pRequest.get();

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));

    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(_lifecycle, isModeChangePossible()).WillOnce(Return(false));

    EXPECT_EQ(
        DiagReturnCode::ISO_CONDITIONS_NOT_CORRECT,
        _softReset.execute(
            _incomingDiagConnection,
            &softResetRequest[1U], // prefix has been handled by the DiagDispatcher
            SUBFUNCTION_LENGTH));
}

TEST_F(
    UdsIntegration,
    calling_SoftReset_responseSent_will_not_enable_DiagDispatcher_if_LIFECYCLE_is_OK)
{
    _udsDispatcher.disable();
    EXPECT_CALL(_incomingDiagConnection, terminate());

    EXPECT_CALL(_lifecycle, requestShutdown(IUdsLifecycleConnector::SOFT_RESET, _))
        .WillOnce(Return(true));

    _softReset.responseSent(_incomingDiagConnection, AbstractDiagJob::RESPONSE_SENT);

    EXPECT_FALSE(_udsDispatcher.isEnabled());
}

TEST_F(
    UdsIntegration,
    calling_SoftReset_responseSent_will_enable_DiagDispatcher_if_LIFECYCLE_is_not_OK)
{
    _udsDispatcher.disable();
    EXPECT_CALL(_incomingDiagConnection, terminate());

    EXPECT_CALL(_lifecycle, requestShutdown(IUdsLifecycleConnector::SOFT_RESET, _))
        .WillOnce(Return(false));

    _softReset.responseSent(_incomingDiagConnection, AbstractDiagJob::RESPONSE_SENT);

    EXPECT_TRUE(_udsDispatcher.isEnabled());
}

TEST_F(UdsIntegration, calling_PowerDown)
{
    uint8_t const POWER_DOWN_REQUEST[] = {ECU_RESET, POWER_DOWN};

    TransportMessageWithBuffer pRequest(
        SOURCE_ID, TARGET_ID, POWER_DOWN_REQUEST, sizeof(POWER_DOWN_REQUEST));

    _incomingDiagConnection.fpRequestMessage = pRequest.get();
    _incomingDiagConnection.fpMessageSender  = &_udsDispatcher;
    _incomingDiagConnection.setDiagSessionManager(_sessionManager);
    _incomingDiagConnection.fServiceId = ECU_RESET;

    _incomingDiagConnection.fOpen = true;

    EXPECT_CALL(_lifecycle, requestPowerdown(false, _)).WillOnce(Return(true));

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(
        _sessionManager,
        responseSent(Ref(_incomingDiagConnection), DiagReturnCode::OK, NotNull(), _));

    EXPECT_CALL(_incomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        _powerDown.execute(_incomingDiagConnection, POWER_DOWN_REQUEST + 1, 1U));
    CONTEXT_EXECUTE;
}

TEST_F(UdsIntegration, calling_RapidPowerDown)
{
    uint8_t const POWER_DOWN_REQUEST[] = {ECU_RESET, RAPID_POWER_DOWN};

    TransportMessageWithBuffer pRequest(
        SOURCE_ID, TARGET_ID, POWER_DOWN_REQUEST, sizeof(POWER_DOWN_REQUEST));

    _incomingDiagConnection.fpRequestMessage = pRequest.get();
    _incomingDiagConnection.fpMessageSender  = &_udsDispatcher;
    _incomingDiagConnection.setDiagSessionManager(_sessionManager);
    _incomingDiagConnection.fServiceId = ECU_RESET;

    _incomingDiagConnection.fOpen = true;

    EXPECT_CALL(_lifecycle, requestPowerdown(true, _)).WillOnce(Return(true));

    EXPECT_CALL(_sessionManager, getActiveSession())
        .WillRepeatedly(ReturnRef(DiagSession::APPLICATION_DEFAULT_SESSION()));
    EXPECT_CALL(_sessionManager, acceptedJob(_, _, _, _))
        .WillRepeatedly(Return(uds::DiagReturnCode::OK));

    EXPECT_CALL(
        _sessionManager,
        responseSent(Ref(_incomingDiagConnection), DiagReturnCode::OK, NotNull(), _));

    EXPECT_CALL(_incomingDiagConnection, terminate());

    EXPECT_EQ(
        DiagReturnCode::OK,
        _enableRapidPowerShutdown.execute(_incomingDiagConnection, POWER_DOWN_REQUEST + 1, 1U));
    CONTEXT_EXECUTE;
}

} // anonymous namespace
