/** @file logging.h
 *  @brief Logging definitions
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_LOGGING_H
#define KERNELMODULE_PCM3060_UTILS_LOGGING_H

#include <linux/printk.h>
#include <config.h>

#ifndef CONFIG_DEBUG_LOGGING_ENABLE
    #define INFO(...)
    #define DEBUG(...)
    #define TRACE(...)
#else
    //#warning Kernel debug logging enabled!
    #define INFO(fmt,args...) printk(KERN_INFO CONFIG_LOGGING_NAME ": [%s] "fmt"\n", __FUNCTION__, ##args)
    #define DEBUG(fmt,args...) printk(KERN_INFO CONFIG_LOGGING_NAME ": [%s] "fmt"\n", __FUNCTION__, ##args) // todo change this for KERN_DEBUG later
    #define TRACE(fmt,args...) printk(KERN_INFO CONFIG_LOGGING_NAME ": [%s] "fmt"\n", __FUNCTION__, ##args) // todo change this for KERN_TRACE later
#endif

#define WARNING(fmt,args...) printk(KERN_WARNING CONFIG_LOGGING_NAME ": [%s] "fmt"\n", __FUNCTION__, ##args)
#define ERROR(fmt,args...) printk(KERN_ERR CONFIG_LOGGING_NAME ": [%s] "fmt"\n", __FUNCTION__, ##args)

#endif // !KERNELMODULE_PCM3060_UTILS_LOGGING_H
