
#ifndef KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H
#define KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H

#include "config.h"
#include <linux/platform_device.h>
#include <linux/pwm.h>

int clock_generator_init(struct device *pdev);
int clock_generator_cleanup(struct device *pdev);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H

//     pwm_left = pwm_get(&pdev->dev, NULL);
//     if (IS_ERR(pwm_left)){
//         printk("Requesting PWM failed %d", ERR_CAST(pwm_left));
//         return -EIO;
//     }
//     printk("Requested PWM");
//     return 0;
// }