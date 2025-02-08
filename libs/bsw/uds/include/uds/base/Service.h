// Copyright 2024 Accenture.

#ifndef GUARD_73BFB590_8BCF_453B_8CCB_2A72CEC3E971
#define GUARD_73BFB590_8BCF_453B_8CCB_2A72CEC3E971

#include "uds/base/AbstractDiagJob.h"

namespace uds
{
/**
 * Helper base class for UDS services.
 *
 *
 * \see AbstractDiagJob
 */
class Service : public AbstractDiagJob
{
public:
    Service(uint8_t const service, DiagSession::DiagSessionMask sessionMask);

    Service(
        uint8_t const service,
        uint8_t const requestPayloadLength,
        uint8_t const responseLength,
        DiagSession::DiagSessionMask sessionMask);

protected:
    /**
     * \see AbstractDiagJob::enableSuppressPositiveResponse()
     */
    using AbstractDiagJob::enableSuppressPositiveResponse;

    /**
     * \see AbstractDiagJob::verify()
     */
    DiagReturnCode::Type verify(uint8_t const request[], uint16_t requestLength) override;

private:
    void init(uint8_t service);

    uint8_t fService[1];
};

} // namespace uds

#endif // GUARD_73BFB590_8BCF_453B_8CCB_2A72CEC3E971
