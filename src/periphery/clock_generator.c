#include "clock_generator.h"

#include "devicetree.h"
#include <linux/pwm.h>
#include <utils/logging.h>
#include <utils/conversions.h>

//#include <linux/gpio.h> // TODO move to own file

#define CLOCK_GENERATOR_CLK_PWM_ENABLE 1
#define DUTCYCLE_PERCENT 50
#define DUTCYCLE_PERCENT_SCALE 100



int clock_generator_init(struct device *pdev, const unsigned int frequency)
{
    int ret = 0,
        period = HZ_TO_NS(frequency);
    struct pwm_device* pwm_sck1_ptr;
    struct pwm_state new_sck1_state = {
        .period = period,
        .polarity = PWM_POLARITY_NORMAL,
        .enabled = CLOCK_GENERATOR_CLK_PWM_ENABLE
    };

    TRACE("Frequency %d Hz", frequency);

    if (!pdev)
    {
        ERROR("Invalid  device!");
        ret = -ENXIO;
    }
    else if ( (ret = pwm_set_relative_duty_cycle(&new_sck1_state, DUTCYCLE_PERCENT, DUTCYCLE_PERCENT_SCALE)) )
    {
        ERROR("Failed to set duty cycle");
    }
    else
    {
        INFO("Requesting PWM for SCK1");
        pwm_sck1_ptr = devm_pwm_get(pdev, DEVICETREE_PCM3060_SCK1_PWMNAME);
        if (IS_ERR(pwm_sck1_ptr))
        {
            ERROR("Requesting PWM  for SCK1 failed %p", ERR_CAST(pwm_sck1_ptr));
            ret = -EIO;
        }
    }

    if(!ret)
    {
        INFO("Applying new state for SCK1: period[%dns], duty cycle[%dns], polarity[%d], enable [%d]",
        new_sck1_state.period, new_sck1_state.duty_cycle, new_sck1_state.polarity, new_sck1_state.enabled);
        ret = pwm_apply_state(pwm_sck1_ptr, &new_sck1_state);
        INFO("Requested PWM");
    }

    return ret;
}

int clock_generator_cleanup(struct device *pdev)
{
    DEBUG("");
    return 0;
}
