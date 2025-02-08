// Copyright 2024 Accenture.

/**
 * Contains interface IFilteredCANFrameSentListener.
 * \file    IFilteredCANFrameSentListener.h
 * \ingroup    canframes
 */
#ifndef GUARD_B1CC6DFA_78C8_4A71_9631_03FBEE82C1DE
#define GUARD_B1CC6DFA_78C8_4A71_9631_03FBEE82C1DE

#include <etl/intrusive_forward_list.h>

namespace can
{
class CANFrame;
class IFilter;

/**
 * Interface for a filtered listener for sent CANFrames.
 *
 */
class IFilteredCANFrameSentListener : public ::etl::forward_link<0>
{
protected:
    IFilteredCANFrameSentListener();

public:
    IFilteredCANFrameSentListener(IFilteredCANFrameSentListener const&)            = delete;
    IFilteredCANFrameSentListener& operator=(IFilteredCANFrameSentListener const&) = delete;
    /**
     * \warning
     * This callback is called from ISR context!
     */
    virtual void canFrameSent(CANFrame const& frame)                               = 0;

    /**
     * Returns the filter for which frames the listeners wants to be notified.
     */
    virtual IFilter& getFilter() = 0;
};

inline IFilteredCANFrameSentListener::IFilteredCANFrameSentListener() : ::etl::forward_link<0>() {}

} // namespace can

#endif /*GUARD_B1CC6DFA_78C8_4A71_9631_03FBEE82C1DE*/
