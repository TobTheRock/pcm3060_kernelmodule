
#ifndef KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H
#define KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H

#include <linux/device.h>

int clock_generator_init(struct device *pdev, const unsigned int frequency);
int clock_generator_cleanup(struct device *pdev);
// int clock_generator_reset(const unsigned int frequency);

#endif // !KERNELMODULE_PCM3060_PERIPHERY_CLOCK_GENERATOR_H

//     pwm_left = pwm_get(&pdev->dev, NULL);
//     if (IS_ERR(pwm_left)){
//         printk("Requesting PWM failed %d", ERR_CAST(pwm_left));
//         return -EIO;
//     }
//     printk("Requested PWM");
//     return 0;
// }