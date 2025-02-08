// Copyright 2024 Accenture.

#ifndef GUARD_51B063D4_A3A5_435A_93E1_8E93825857C5
#define GUARD_51B063D4_A3A5_435A_93E1_8E93825857C5

#include "uds/base/Service.h"

namespace uds
{
class IDiagAuthenticator;

/**
 * Helper base class for UDS services.
 *
 *
 * \see Service
 */
class ServiceWithAuthentication : public Service
{
public:
    explicit ServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        DiagSession::DiagSessionMask sessionMask);

    explicit ServiceWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagAuthenticator();
     */
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fDiagAuthenticator;
};

} // namespace uds

#endif // GUARD_51B063D4_A3A5_435A_93E1_8E93825857C5
