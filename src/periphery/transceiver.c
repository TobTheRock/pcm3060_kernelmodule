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
#include <utils/functions.h>

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>

#define _BCK_LRCK_RATIO ((CONFIG_RATIO_BCK_FS_HZ)/(CONFIG_RATIO_LRCK_FS_HZ))

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
    static bool current_bck_val = 0;
    static bool current_lrck_val = 0;
    static unsigned int current_lrck_cnt = 0;
    static unsigned char data_byte_mask = 0b00000001;


    // TRACE("Transceiver bit bang timer callback triggered, BCK %d ", _current_bck_val);


     /* do your timer stuff here */
    // printk(KERN_INFO "Timer Callback function Called \n");

    // Toggle bck clock
    gpio_set_value(_gpio_num_bck, current_bck_val);
    current_bck_val = !current_bck_val;

    //toggle lrck clock respectevily
    if (!(++current_lrck_cnt % _BCK_LRCK_RATIO))
    {
        gpio_set_value(_gpio_num_lrck, current_lrck_val);
        current_lrck_val = !current_lrck_val;
        current_lrck_cnt = 0;// should be no need for that
    }

    data_byte_mask = CYCLE_SHIFT(data_byte_mask, 1);
    TRACE("byte mask %d", data_byte_mask);

    hrtimer_forward_now(timer, ktime_set(0,_T_bck_ns));
    return HRTIMER_RESTART;
}

static int _get_and_set_gpio(struct device *pdev, int* gpio_num, const char* name, bool is_output)
{
    TRACE("Requesting GPIO %s", name);
    if (get_gpio(pdev, name , gpio_num))
    {
        ERROR("Failed to aquire GPIO %s ", name);
        return -1;
    }

    if (is_output)
    {
        if (gpio_direction_output(*gpio_num, 0))
        {
            ERROR("Failed to set the gpio pin as output!");
            return -1;
        }
    }
    else
    {
        if (gpio_direction_input(*gpio_num))
        {
            ERROR("Failed to set the gpio pin as input!");
            return -1;
        }
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

    TRACE("Triggering BCK at %lu hz", _T_bck_ns);

    if (_get_and_set_gpio(pdev, &_gpio_num_lrck, DEVICETREE_PCM3060_GPIO_LRCK_NAME, true))
    {
        ERROR("Failed to get the gpio pin for LRCK clock!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_bck, DEVICETREE_PCM3060_GPIO_BCK_NAME, true))
    {
        ERROR("Failed to get the gpio pin for BCK clock!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_din, DEVICETREE_PCM3060_GPIO_DIN_NAME, true))
    {
        ERROR("Failed to get the gpio pin for DIN input of PCM3060!");
        return -1;
    }
    else if (_get_and_set_gpio(pdev, &_gpio_num_dout, DEVICETREE_PCM3060_GPIO_DOUT_NAME, false))
    {
        ERROR("Failed to get the gpio pin for DOUT output of PCM3060!");
        return -1;
    }
    INFO("GPIO IDs for:\n LRCK:%d\n BCK:%d\n  DIN:%d\n DOUT:%d\n", _gpio_num_lrck, _gpio_num_bck, _gpio_num_din, _gpio_num_dout);

    ktime = ktime_set(0, _T_bck_ns);
    hrtimer_init(&etx_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    etx_hr_timer.function = &timer_callback;
    hrtimer_start(&etx_hr_timer, ktime, HRTIMER_MODE_REL);

    return 0;
}

int tx_cleanup(struct device *pdev)
{
    hrtimer_cancel(&etx_hr_timer);
    gpio_free(_gpio_num_lrck);
    gpio_free(_gpio_num_bck);
    gpio_free(_gpio_num_din);
    gpio_free(_gpio_num_dout);
    
    return 0;
}