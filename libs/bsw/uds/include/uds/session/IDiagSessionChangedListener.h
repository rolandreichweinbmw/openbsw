// Copyright 2024 Accenture.

#ifndef GUARD_1D16CA52_0D50_482A_A390_EEAA69ECB2CF
#define GUARD_1D16CA52_0D50_482A_A390_EEAA69ECB2CF

#include <etl/intrusive_forward_list.h>

#include <platform/estdint.h>

namespace uds
{
class DiagSession;

/**
 * Interface for listeners to change of diagnosis session
 *
 *
 * \see IDiagSessionManager
 */
class IDiagSessionChangedListener : public ::etl::forward_link<0>
{
public:
    virtual void diagSessionChanged(DiagSession const& session) = 0;
    virtual void diagSessionResponseSent(uint8_t responseCode)  = 0;
};

inline bool
operator==(IDiagSessionChangedListener const& lhs, IDiagSessionChangedListener const& rhs)
{
    return &lhs == &rhs;
}

} // namespace uds

#endif // GUARD_1D16CA52_0D50_482A_A390_EEAA69ECB2CF
