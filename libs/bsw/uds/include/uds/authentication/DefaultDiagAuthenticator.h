// Copyright 2024 Accenture.

#ifndef GUARD_F746EE50_8B4F_499E_B777_4592C0DC41BD
#define GUARD_F746EE50_8B4F_499E_B777_4592C0DC41BD

#include "uds/DiagReturnCode.h"
#include "uds/authentication/IDiagAuthenticator.h"

namespace uds
{
/**
 * Subclass of IDiagAuthenticator which will always return true for
 * isAuthenticated().
 *
 *
 * This class should be used by diagnosis jobs which do not need
 * authentication.
 */
class DefaultDiagAuthenticator : public IDiagAuthenticator
{
public:
    DefaultDiagAuthenticator() = default;

    /**
     * Always returns true
     * \see IDiagAuthenticator::isAuthenticated()
     */
    bool isAuthenticated(uint16_t address) const override;
    /**
     * \see IDiagAuthenticator::getNotAuthenticatedReturnCode()
     */
    DiagReturnCode::Type getNotAuthenticatedReturnCode() const override;
};

} // namespace uds

#endif // GUARD_F746EE50_8B4F_499E_B777_4592C0DC41BD
