// Copyright 2024 Accenture.

#ifndef GUARD_DBD10C2B_75A6_4975_A404_DEFFD8716534
#define GUARD_DBD10C2B_75A6_4975_A404_DEFFD8716534

#include "uds/DiagReturnCode.h"
#include "uds/async/IAsyncDiagHelper.h"

#include <async/util/Call.h>
#include <etl/intrusive_forward_list.h>

namespace uds
{
class AbstractDiagJob;
class IncomingDiagConnection;

/**
 * Helper class for handling nested diagnostic requests
 */
class AsyncDiagJobHelper
{
public:
    /**
     * Constructor
     * \param helper Reference to async helper class
     * \param job Reference to diag job
     * \param diagcontext Context to execute on
     */
    AsyncDiagJobHelper(
        IAsyncDiagHelper& asyncHelper, AbstractDiagJob& job, ::async::ContextType diagcontext);

    AsyncDiagJobHelper(AsyncDiagJobHelper const&)            = delete;
    AsyncDiagJobHelper& operator=(AsyncDiagJobHelper const&) = delete;

    /**
     * Check whether an asynchronous request is currently active
     */
    bool hasPendingAsyncRequest() const;

    /**
     * Get connection bound to the pending async request.
     * Note: Call this function only if hasPendingAsyncRequest() returns true
     * \return Reference to connection bound to this request
     */
    IncomingDiagConnection& getPendingConnection() const;

    /**
     * Try to enqueue a request.
     * \param connection Connection to enqueue
     * \return Result of enqueuing
     */
    DiagReturnCode::Type enqueueRequest(
        IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength);

    /**
     * Start processing an async request.
     * \param connection Reference to connection bound to this request
     */
    void startAsyncRequest(IncomingDiagConnection& connection);

    /**
     * End processing an async request. This call will retrigger processing of
     * queued requests.
     */
    void endAsyncRequest();

private:
    void triggerNextRequests();

    IAsyncDiagHelper& fAsyncHelper;
    AbstractDiagJob& fJob;
    IncomingDiagConnection* fPendingAsyncConnection;
    ::etl::intrusive_forward_list<IAsyncDiagHelper::StoredRequest, ::etl::forward_link<0>>
        fPendingRequests;
    ::async::Function fTriggerNextRequests;
    ::async::ContextType fContext;
};

} // namespace uds

#endif // GUARD_DBD10C2B_75A6_4975_A404_DEFFD8716534
