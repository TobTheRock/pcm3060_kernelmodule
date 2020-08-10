#include "clock_generator.h"


#include <utils/logging.h>

//#include <linux/gpio.h> // TODO move to own file
//ADC
#define CLOCK_GENERATOR_ADC_CLOCK_SCK1_T_NS ((1000000000)/(CONFIG_ADC_CLOCK_SCK1_F_HZ)) // SCK clock period in nanoseconds
#define CLOCK_GENERATOR_ADC_SCK1_BCK1_RATIO ((CONFIG_ADC_RATIO_SCK1_FS_HZ)/(CONFIG_ADC_RATIO_BCK1_FS_HZ))
#define CLOCK_GENERATOR_ADC_SCK1_LRCK_RATIO ((CONFIG_ADC_RATIO_SCK1_FS_HZ)/(CONFIG_ADC_RATIO_LRCK_FS_HZ))
//DAC
#define CLOCK_GENERATOR_DAC_CLOCK_SCK2_T_NS ((1000000000)/(CONFIG_DAC_CLOCK_SCK2_F_HZ)) // SCK clock period in nanoseconds
#define CLOCK_GENERATOR_DAC_SCK2_BCK2_RATIO ((CONFIG_DAC_RATIO_SCK2_FS_HZ)/(CONFIG_DAC_RATIO_BCK2_FS_HZ))
#define CLOCK_GENERATOR_DAC_SCK2_LRCK_RATIO ((CONFIG_DAC_RATIO_SCK2_FS_HZ)/(CONFIG_DAC_RATIO_LRCK_FS_HZ))

#define CLOCK_GENERATOR_ADC_CLK_PWM_DUTY_CYCLE_NS CLOCK_GENERATOR_ADC_CLOCK_SCK1_T_NS/2
#define CLOCK_GENERATOR_DAC_CLK_PWM_DUTY_CYCLE_NS CLOCK_GENERATOR_DAC_CLOCK_SCK2_T_NS/2
#define CLOCK_GENERATOR_CLK_PWM_ENABLE 1


int clock_generator_init(struct device *pdev)
{
    int ret = 0;
    struct pwm_device* pwm_sck1_ptr;
    struct pwm_state new_sck1_state = {
        CLOCK_GENERATOR_DAC_CLOCK_SCK2_T_NS,
        CLOCK_GENERATOR_ADC_CLK_PWM_DUTY_CYCLE_NS,
        PWM_POLARITY_NORMAL,
        CLOCK_GENERATOR_CLK_PWM_ENABLE
    };


    if (!pdev)
    {
        ERROR("Invalid  device!");
        ret = -ENXIO;
    }
    else
    {
        INFO("Requesting PWM for SCK1");
        pwm_sck1_ptr = devm_pwm_get(pdev, CONFIG_DT_PCM3060_SCK1_PWMNAME);
        if (IS_ERR(pwm_sck1_ptr))
        {            ERROR("Requesting PWM  for SCK2 failed %p", ERR_CAST(pwm_sck1_ptr));
            ret = -EIO;
        }
    }

    if(!ret)
    {
        INFO("Applying new state for SCK1: period[%dns], duty cycle[%d %%], polarity[%d], enable [%d]",
            new_sck1_state.period, new_sck1_state.duty_cycle, new_sck1_state.polarity, new_sck1_state.enabled);
        ret = pwm_apply_state(pwm_sck1_ptr, &new_sck1_state);
        INFO("Requested PWM");
    }

    return ret;
}

void clock_generator_cleanup(void)
{
    
    return;
}
