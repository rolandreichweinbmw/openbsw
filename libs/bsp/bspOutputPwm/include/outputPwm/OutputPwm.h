// Copyright 2024 Accenture.

#ifndef GUARD_5519D93B_600A_46D1_84D9_6F9A7BDC3F79
#define GUARD_5519D93B_600A_46D1_84D9_6F9A7BDC3F79

#include "io/DynamicClientCfg.h"
#include "io/Io.h"
#include "mcu/mcu.h"
#include "platform/estdint.h"

#include <etl/uncopyable.h>

namespace bios
{

class OutputPwm : public ::etl::uncopyable
{
public:
    struct tOutputPwmCfg
    {
        uint16_t defaultPwmPeriod;
        uint16_t defaultPwmDuty;
    };

#undef BSPOUTPUTPWMCONFIG
#include "bsp/io/outputPwm/outputPwmConfiguration.h"

    OutputPwm();

    // Api for all DynamicClients
    class IDynamicPwmClient
    {
    public:
        virtual bsp::BspReturnCode setDuty(uint16_t chan, uint16_t duty, bool immediateUpdate) = 0;
        virtual bsp::BspReturnCode setPeriod(uint16_t chan, uint16_t period)                   = 0;
    };

    static void init(uint8_t hwVariant = 0);
    static void shutdown();

    static OutputPwm::tOutputPwmCfg* getConfiguration(uint8_t hwVariant);
    /**
     * */
    static bsp::BspReturnCode setDuty(uint16_t chan, uint16_t duty, bool immediateUpdate = false);
    /**
     * */
    static bsp::BspReturnCode setPeriod(uint16_t chan, uint16_t period);

    static uint8_t const outputPwmStart        = _pwmDynamicStart;
    static uint8_t const outputPwmStaticNumber = _pwmStaticEnd;
    static uint8_t const outputPwmNumberAll    = _pwmChannelMax;

    /**
     * setDynamicOuputClient
     * \param outputNumber see outputConfiguration
     * \param clientOutputNumber number inside from OutputClient
     * \param see IDynamicOutputClient
     * \return see bsp.h
     */
    static bsp::BspReturnCode
    setDynamicClient(uint16_t outputNumber, uint16_t clientOutputNumber, IDynamicPwmClient* client);
    /**
     * clrDynamicOutputClient
     * \param outputNumber see outputConfiguration
     * \param see IDynamicOutputClient
     * \return see bsp.h
     */
    static bsp::BspReturnCode clrDynamicClient(uint16_t outputNumber);
    /**
     * @ get Pwm name for Debug
     */
    static char const* getName(uint16_t channel);
    /**
     * @ MAX_DUTY_CYCLE 100% in 0,01 %
     */
    static uint32_t const MAX_DUTY_CYCLE = 10000;

private:
    static void cleanDynamicClients();

    using dynamicClientType = uint16_t;
    static uint8_t const outputNumberDynamic
        = (((outputPwmNumberAll - outputPwmStaticNumber) > 0)
               ? (outputPwmNumberAll - outputPwmStaticNumber)
               : 1);

    static dynamicClient<dynamicClientType, IDynamicPwmClient, 4, outputNumberDynamic>
        dynamicPwmOutputCfg;
};

} /* namespace bios */

#endif /* GUARD_5519D93B_600A_46D1_84D9_6F9A7BDC3F79 */
