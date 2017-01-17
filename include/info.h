#ifndef __INFO_H_
#define __INFO_H_

#include "osapi.h"

#if defined(DEBUG_ON)
#define INFO( format, ... ) os_printf( format, ## __VA_ARGS__ )
#else
#define INFO( format, ... )
#endif

#endif