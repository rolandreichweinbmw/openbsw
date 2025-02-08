// Copyright 2024 Accenture.

/**
 * Contains interface ICANFrameListener.
 * \file ICANFrameListener.h
 * \ingroup framemgmt
 */
#ifndef GUARD_3991843A_AE0E_493A_8A61_E4A780891238
#define GUARD_3991843A_AE0E_493A_8A61_E4A780891238

#include <etl/intrusive_forward_list.h>

namespace can
{
class CANFrame;
class IFilter;

/**
 * CANFrameListener interface
 *
 *
 * An ICANFrameListener subclass is a class interested in the reception
 * of CANFrames. Therefore it needs to register at an ICanTransceiver.
 */
class ICANFrameListener : public ::etl::forward_link<0>
{
public:
    ICANFrameListener();
    ICANFrameListener(ICANFrameListener const&)            = delete;
    ICANFrameListener& operator=(ICANFrameListener const&) = delete;

    /**
     * This method notifies the listener of a CANFrame reception.
     */
    virtual void frameReceived(CANFrame const& canFrame) = 0;

    /**
     * Returns the ICANFrameListeners filter.
     */
    virtual IFilter& getFilter() = 0;
};

/*
 * inline implementation
 */
inline ICANFrameListener::ICANFrameListener() : ::etl::forward_link<0>() {}

} // namespace can

#endif // GUARD_3991843A_AE0E_493A_8A61_E4A780891238
