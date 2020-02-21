#pragma once

#if defined(TARGET_UARM)
#include <uarm/libuarm.h>
#include <uarm/arch.h>
#include <uarm/uARMtypes.h>
#elif defined(TARGET_UMPS)
#include <umps/libumps.h>
#include <umps/arch.h>
#include <umps/types.h>
#else
#error "Unknown arch"
#endif

#if !defined(NULL)
#define NULL ((void *) 0)
#endif
