#pragma once

#if defined(TARGET_UARM)
#include <libuarm.h>
#include <arch.h>
#include <uARMtypes.h>
#elif defined(TARGET_UMPS)
#include <libumps.h>
#include <arch.h>
#include <types.h>
#else
#error "Unknown arch"
#endif
