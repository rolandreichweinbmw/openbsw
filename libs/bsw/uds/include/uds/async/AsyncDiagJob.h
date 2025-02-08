// Copyright 2024 Accenture.

#ifndef GUARD_8AFBD8AF_CBF6_461E_98EE_A83D33504AF5
#define GUARD_8AFBD8AF_CBF6_461E_98EE_A83D33504AF5

#include "async/util/Call.h"
#include "uds/async/AsyncDiagJobHelper.h"
#include "uds/base/AbstractDiagJob.h"
#include "uds/connection/IncomingDiagConnection.h"

#include <etl/utility.h>

namespace uds
{
/**
 * Template function that encapsulates an asynchronous diagnostic job, makes
 * it capable of storing parallel requests and execute the process() function
 * from within a specified context
 *
 * \note The AsyncDiagJob class needs to derive from the diagnostic job class
 *       to reliably receive the responseSent to trigger processing of stored
 *       requests.
 *
 * To support any kind of diag job to encapsulate the asynchronous class derives
 * from the specified base class.
 *
 * \tparam T    Diagnostic job class to encapsulate
 */
template<class T>
class AsyncDiagJob : public T
{
public:
    /**
     * Constructor with variable number of parameters to forward to base class constructor.
     * \param asyncHelper Reference to async helper interface
     * \param args Parameters for base class constructor
     */
    template<typename... Args>
    AsyncDiagJob(IAsyncDiagHelper& asyncHelper, ::async::ContextType context, Args&&... args);

    void responseSent(
        IncomingDiagConnection& connection, typename T::ResponseSendResult result) override;

protected:
    DiagReturnCode::Type process(
        IncomingDiagConnection& connection,
        uint8_t const request[],
        uint16_t requestLength) override;

private:
    void asyncProcess(
        IncomingDiagConnection* connection, uint8_t const request[], uint16_t requestLength);

    AsyncDiagJobHelper fAsyncJobHelper;
    ::async::Function fProcess;
    ::async::ContextType fContext;
};

/**
 * Inline implementations
 */
template<class T>
template<typename... Args>
AsyncDiagJob<T>::AsyncDiagJob(
    IAsyncDiagHelper& asyncHelper, ::async::ContextType context, Args&&... args)
: T(::etl::forward<Args>(args)...)
, fAsyncJobHelper(asyncHelper, *this, context)
, fProcess([&]() { asyncProcess(nullptr, nullptr, 0U); })
, fContext(context)
{}

template<class T>
void AsyncDiagJob<T>::responseSent(
    IncomingDiagConnection& connection, typename T::ResponseSendResult const result)
{
    T::responseSent(connection, result);
    fAsyncJobHelper.endAsyncRequest();
}

template<class T>
DiagReturnCode::Type AsyncDiagJob<T>::process(
    IncomingDiagConnection& connection, uint8_t const request[], uint16_t const requestLength)
{
    if (fAsyncJobHelper.hasPendingAsyncRequest())
    {
        return fAsyncJobHelper.enqueueRequest(connection, request, requestLength);
    }
    fAsyncJobHelper.startAsyncRequest(connection);
    auto lambda
        = [&, request, requestLength]() { asyncProcess(&connection, request, requestLength); };
    fProcess = ::async::Function(lambda);
    ::async::execute(fContext, fProcess);

    return DiagReturnCode::OK;
}

template<class T>
void AsyncDiagJob<T>::asyncProcess(
    IncomingDiagConnection* connection, uint8_t const request[], uint16_t const requestLength)
{
    DiagReturnCode::Type responseCode = T::process(*connection, request, requestLength);
    if (responseCode != DiagReturnCode::OK)
    {
        connection->sendNegativeResponse(responseCode, *this);
        connection->terminate();
    }
}

} // namespace uds

#endif // GUARD_8AFBD8AF_CBF6_461E_98EE_A83D33504AF5
