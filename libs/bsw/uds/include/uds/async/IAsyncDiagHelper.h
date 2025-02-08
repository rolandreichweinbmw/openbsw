// Copyright 2024 Accenture.

#ifndef GUARD_CD4D716E_A170_4650_84C7_E9C4239BD9C3
#define GUARD_CD4D716E_A170_4650_84C7_E9C4239BD9C3

#include <etl/intrusive_forward_list.h>

namespace uds
{
class AbstractDiagJob;
class IncomingDiagConnection;

/**
 * Interface for storing/processing requests
 */
class IAsyncDiagHelper
{
public:
    class StoredRequest : public ::etl::forward_link<0>
    {
    public:
        StoredRequest(
            IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength);

        IncomingDiagConnection& getConnection() const;
        uint8_t const* getRequest() const;
        uint16_t getRequestLength() const;

    private:
        IncomingDiagConnection* fConnection;
        uint8_t const* fRequest;
        uint16_t fRequestLength;
    };

    /**
     * Allocate and return a pointer to a stored request object that
     * can be processed and released later with processAndReleaseRequest.
     * \param connection Reference to connection as received in AbstractDiagJob::process
     * \param request Request as received in AbstractDiagJob::process
     * \param requestLength requestLength as received in AbstractDiagJob::process
     */
    virtual StoredRequest* allocateRequest(
        IncomingDiagConnection& connection, uint8_t const* request, uint16_t requestLength)
        = 0;

    /**
     * Process a previously allocated request and release the object.
     * \param job Reference to job for which process will be called again
     * \param request Reference to stored request as returned by a previous call to
     *        allocateRequest()
     */
    virtual void processAndReleaseRequest(AbstractDiagJob& job, StoredRequest& request) = 0;
};

/**
 * Inline implementation
 */
inline IAsyncDiagHelper::StoredRequest::StoredRequest(
    IncomingDiagConnection& connection, uint8_t const* const request, uint16_t const requestLength)
: fConnection(&connection), fRequest(request), fRequestLength(requestLength)
{}

inline IncomingDiagConnection& IAsyncDiagHelper::StoredRequest::getConnection() const
{
    return *fConnection;
}

inline uint8_t const* IAsyncDiagHelper::StoredRequest::getRequest() const { return fRequest; }

inline uint16_t IAsyncDiagHelper::StoredRequest::getRequestLength() const { return fRequestLength; }

} // namespace uds

#endif // GUARD_CD4D716E_A170_4650_84C7_E9C4239BD9C3
