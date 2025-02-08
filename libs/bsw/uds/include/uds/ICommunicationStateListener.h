// Copyright 2024 Accenture.

#ifndef GUARD_899F066E_9209_43DF_B727_D4C75179C021
#define GUARD_899F066E_9209_43DF_B727_D4C75179C021

#include <etl/intrusive_forward_list.h>
#include <etl/uncopyable.h>

namespace uds
{
class ICommunicationStateListener
: public ::etl::forward_link<0>
, public ::etl::uncopyable
{
public:
    enum CommunicationState
    {
        ENABLE_NORMAL_MESSAGE_TRANSMISSION,
        DISABLE_NORMAL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        ENABLE_NN_MESSAGE_TRANSMISSION,
        DISABLE_NM_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_NM_SEND_TRANSMISSION,
        ENABLE_ALL_MESSAGE_TRANSMISSION,
        DISABLE_ALL_MESSAGE_TRANSMISSION,
        ENABLE_REC_DISABLE_ALL_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NORMAL_MESSAGE_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_NM_SEND_TRANSMISSION,
        DISABLE_REC_ENABLE_ALL_SEND_TRANSMISSION
    };

    ICommunicationStateListener() {}

    virtual void communicationStateChanged(CommunicationState newState) = 0;
};

inline bool
operator==(ICommunicationStateListener const& lhs, ICommunicationStateListener const& rhs)
{
    return &lhs == &rhs;
}

} // namespace uds

#endif // GUARD_899F066E_9209_43DF_B727_D4C75179C021
