#pragma once

/**
 * Stores the current state of the CPU suspending its computation, then it handles
 * the interrupt according to the device that caused it afterwards it resumes the
 * computation loading the correct state of the CPU.
 *
 * @attention Calling this function while no interrupt has been raised causes a kernel PANIC.
 */
extern void handlers_interruptHandler(void);

/**
 * Stores the current state of the CPU suspending its computation, then it executes
 * the specified system call and afterwards it resumes the computation loading
 * the correct state of the CPU.
 *
 * @attention Calling this function while no system call has been called is UB.
 */
extern void handlers_sysbkHandler(void);

/**
 * Calls a custom handler for TLBs.
 *
 * @attention If no custom handler is defined for the current process, this causes a kernel PANIC. 
 */
extern void handlers_TLBHandler(void);

/**
 * Calls a custom handler for traps.
 *
 * @attention If no custom handler is defined for the current process, this causes a kernel PANIC. 
 */
extern void handlers_trapHandler(void);
