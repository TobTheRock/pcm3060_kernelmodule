#include "clock_generator.h"

#include "devicetree.h"
#include <linux/pwm.h>
#include <utils/logging.h>
#include <utils/conversions.h>

//#include <linux/gpio.h> // TODO move to own file

// #define CLOCK_GENERATOR_CLK_PWM_ENABLE 1
// #define CLOCK_GENERATOR_CLK_PWM_DISABLE 0
#define DUTCYCLE_PERCENT 50
#define DUTCYCLE_PERCENT_SCALE 100

static struct pwm_device* dev_pwm_sck;

int clock_generator_init(struct device *pdev, const unsigned int frequency)
{
    TRACE("");
    int ret = 0;
    unsigned int period = HZ_TO_NS(frequency);
    struct pwm_state new_sck1_state = {
        .period = period,
        .polarity = PWM_POLARITY_NORMAL
        , .enabled = 1
    };

    TRACE("Frequency %d Hz, period %d ns", frequency, period);

    if (!pdev)
    {
        ERROR("Invalid  device!");
        ret = -ENXIO;
    }
    else if ( (ret = pwm_set_relative_duty_cycle(&new_sck1_state, DUTCYCLE_PERCENT, DUTCYCLE_PERCENT_SCALE)) )
    {
        ERROR("Failed to set duty cycle");
    }
    else if (dev_pwm_sck == NULL)
    {
        INFO("Requesting PWM for SCK1");
        dev_pwm_sck = devm_pwm_get(pdev, DEVICETREE_PCM3060_SCK_PWM_NAME);
        if (IS_ERR(dev_pwm_sck))
        {
            ERROR("Requesting PWM  for SCK1 failed %d", ERR_CAST(dev_pwm_sck));
            ret = -EIO;
        }
    }

    if(!ret)
    {
        INFO("Applying new state for SCK1: period[%dns], duty cycle[%dns], polarity[%d]",
        new_sck1_state.period, new_sck1_state.duty_cycle, new_sck1_state.polarity);
        pwm_enable(dev_pwm_sck);
        ret = pwm_apply_state(dev_pwm_sck, &new_sck1_state);
        INFO("Requested PWM");
    }

    return ret;
}

// TODO rename this function
int clock_generator_cleanup(struct device *pdev)
{
    DEBUG("");
    pwm_disable(dev_pwm_sck);
    // pwm_put(dev_pwm_sck); //with this pwm device cannot be requested anymore :O -> err -19
    // dev_pwm_sck = NULL;
    return 0;
}
