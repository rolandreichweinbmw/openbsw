// Copyright 2024 Accenture.

#ifndef GUARD_DEDB6DC7_3B13_4ECC_84D1_D6EC188F58E4
#define GUARD_DEDB6DC7_3B13_4ECC_84D1_D6EC188F58E4

#include <etl/intrusive_forward_list.h>
#include <etl/uncopyable.h>

namespace uds
{
/**
 * listener class to derive from in case the communication
 * control is aimed for sub-bus segment to diagnostic-only
 * scheduling mode. see the iso 14229-1 for more information.
 */
class ICommunicationSubStateListener : public ::etl::forward_link<0>
{
public:
    enum CommunicationEnhancedState
    {
        ENABLE_REC_DISABLE_ENHANCED_SEND_TRANSMISSION,
        ENABLE_ENHANCED_TRANSMISSION
    };

    ICommunicationSubStateListener() {}

    /**
     * when service CommunicationControl is called with EnhancedAddressInformation controlType
     * all registered substate listener will be called.
     *
     * \param newState new state to be set
     * \param nodeId identifies the node in the subnetwork
     * \return true in case subnetwork node is known, false otherwise.
     */
    virtual bool communicationStateChanged(CommunicationEnhancedState newState, uint16_t /*nodeId*/)
        = 0;
};

inline bool
operator==(ICommunicationSubStateListener const& lhs, ICommunicationSubStateListener const& rhs)
{
    return &lhs == &rhs;
}

} // namespace uds

#endif // GUARD_DEDB6DC7_3B13_4ECC_84D1_D6EC188F58E4
