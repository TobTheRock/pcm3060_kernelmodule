/** @file sync.c
 *  @brief multithreading sync utilities
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_SYNC_H
#define KERNELMODULE_PCM3060_UTILS_SYNC_H

#include <linux/mutex.h>
#include <linux/types.h>

#define THREAD_BARRIER_START(spinlock, thread_cnt)\
{\
    if (atomic_inc_return(&(thread_cnt)) == 1)\
    {\
        spinlock_lock(&(spinlock));\
    }\

#define THREAD_BARRIER_END(spinlock, thread_cnt)\
    if (atomic_dec_and_test(&(thread_cnt))\
    {\
        spinlock_unlock(&(spinlock));\
    }\
    else\
    {\
        spinlock_lock(&(spinlock));\
    }\
}\

#endif // !KERNELMODULE_PCM3060_UTILS_SYNC_H