// Copyright 2024 Accenture.

#ifndef GUARD_B0036F63_D172_47D2_9CA9_5E92AFD39E74
#define GUARD_B0036F63_D172_47D2_9CA9_5E92AFD39E74

#include "uds/base/SubfunctionWithAuthentication.h"

namespace uds
{
class IDiagSessionManager;

/**
 * Base class for UDS subfunctions
 *
 */
class SubfunctionWithAuthenticationAndSessionControl : public SubfunctionWithAuthentication
{
public:
    SubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        DiagSession::DiagSessionMask sessionMask);

    SubfunctionWithAuthenticationAndSessionControl(
        IDiagAuthenticator const& authenticator,
        IDiagSessionManager& sessionManager,
        uint8_t const implementedRequest[],
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager& getDiagSessionManager() override;

    /**
     * \see AbstractDiagJob::getDiagSessionManager()
     */
    IDiagSessionManager const& getDiagSessionManager() const override;

private:
    IDiagSessionManager& fSessionManager;
};

} // namespace uds

#endif // GUARD_B0036F63_D172_47D2_9CA9_5E92AFD39E74
