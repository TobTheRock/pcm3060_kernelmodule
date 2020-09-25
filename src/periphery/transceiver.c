/** @file transceiver.c
 *  @brief transceive data
 *
 *  @author Tobias Waurick
 */

#include "transceiver.h"
#include "devicetree.h"
#include <config.h>
#include <utils/ptr.h>
#include <utils/logging.h>
#include <utils/conversions.h>

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>

static struct hrtimer etx_hr_timer;

#include <linux/delay.h> // todo RM
#include <linux/jiffies.h> // todo RM

static duplex_ring_end_t* _leftchan_buf;
static duplex_ring_end_t* _rightchan_buf;
static int _gpio_num_lrck;
static int _gpio_num_bck;
static int _gpio_num_din;
static int _gpio_num_dout;
static  unsigned long _T_bck_ns;

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
     /* do your timer stuff here */
    printk(KERN_INFO "Timer Callback function Called \n");
    hrtimer_forward_now(timer,ktime_set(0,_T_bck_ns));
    return HRTIMER_RESTART;
}

static int _get_and_set_gpio(struct device *pdev, int* gpio_num, const char* name, bool is_output)
{
    TRACE("Requesting GPIO %s", name)
    if (get_gpio(pdev, name , gpio_num))
    {
        ERROR("Failed to aquire GPIO %s ", name);
        return -1;
    }

    if (is_output && gpio_direction_output(*gpio_num, 0))
    {
        ERROR("Failed to set the gpio pin as output!");
        return -1;
    }
    else if (gpio_direction_input(*gpio_num))
    {
        ERROR("Failed to set the gpio pin as input!");
        return -1;
    }

    return 0;
}

int tx_init(struct device *pdev, duplex_ring_end_t* leftchan_buf, duplex_ring_end_t* rightchan_buf, const unsigned long f_bck)
{
    ktime_t ktime;
    TRACE();

    if ((pdev == NULL) || (leftchan_buf == NULL) || (rightchan_buf == NULL))
    {
        ERROR("Invalid (nullptr) arguments!");
        return -1;
    }
    _leftchan_buf = leftchan_buf;
    _rightchan_buf = rightchan_buf;

    _T_bck_ns = HZ_TO_NS(f_bck);

    if (_get_and_set_gpio(pdev, &_gpio_num_lrck, DEVICETREE_PCM3060_GPIO_LRCK_NAME, 1))
    {
        ERROR("Failed to get the gpio pin for LRCK clock!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_bck, DEVICETREE_PCM3060_GPIO_BCK_NAME, 1))
    {
        ERROR("Failed to get the gpio pin for BCK clock!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_din, DEVICETREE_PCM3060_GPIO_DIN_NAME, 1))
    {
        ERROR("Failed to get the gpio pin for DIN input of PCM3060!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_dout, DEVICETREE_PCM3060_GPIO_DOUT_NAME, 0))
    {
        ERROR("Failed to get the gpio pin for DOUT output of PCM3060!");
        return -1;
    }

    INFO("GPIO IDs for:\n LRCK:%d\n BCK:%d\n,  DIN:%d\n, DOUT:%d\n", _gpio_num_lrck, _gpio_num_bck, _gpio_num_din, _gpio_num_dout);

    ktime = ktime_set(0, _T_bck_ns);
    hrtimer_init(&etx_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    etx_hr_timer.function = &timer_callback;
    hrtimer_start(&etx_hr_timer, ktime, HRTIMER_MODE_REL);

    return 0;
}

int tx_cleanup(struct device *pdev)
{
    hrtimer_cancel(&etx_hr_timer);
    
    return 0;
}