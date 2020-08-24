/** @file ptr.h
 *  @brief pointer utils
 *
 *  @author Tobias Waurick
 */
#ifndef KERNELMODULE_PCM3060_UTILS_PTR_H
#define KERNELMODULE_PCM3060_UTILS_PTR_H

#include <utils/logging.h>

#define RETURN_VOID_ON_NULL(ptr)\
    if ((ptr) == NULL)\
    {\
        ERROR("%s is a nullpointer, returning", #ptr);\
        return;\
    }
#define RETURN_ON_NULL(ptr, ret)\
    if ((ptr) == NULL)\
    {\
        ERROR("%s is a nullpointer, returning", #ptr);\
        return (ret);\
    }

#define CONST_CAST(type_name) *(type_name*)&

#endif // !KERNELMODULE_PCM3060_UTILS_PTR_H