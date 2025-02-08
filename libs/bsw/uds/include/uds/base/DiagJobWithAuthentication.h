// Copyright 2024 Accenture.

#ifndef GUARD_57F36AE3_ABD3_47CB_B568_89F35E1F8C23
#define GUARD_57F36AE3_ABD3_47CB_B568_89F35E1F8C23

#include "uds/base/AbstractDiagJob.h"

namespace uds
{
class IDiagAuthenticator;

class DiagJobWithAuthentication : public AbstractDiagJob
{
public:
    explicit DiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

    explicit DiagJobWithAuthentication(
        IDiagAuthenticator const& authenticator,
        uint8_t const implementedRequest[],
        uint8_t const requestLength,
        uint8_t const prefixLength,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSessionMask const sessionMask = DiagSession::ALL_SESSIONS());

protected:
    IDiagAuthenticator const& getDiagAuthenticator() const override;

private:
    IDiagAuthenticator const& fAuthenticator;
};

} // namespace uds

#endif // GUARD_57F36AE3_ABD3_47CB_B568_89F35E1F8C23
