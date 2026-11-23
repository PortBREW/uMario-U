#include "header.h"

#include <coreinit/exception.h>
#include <coreinit/context.h>
#include <coreinit/debug.h>
#include <whb/log.h>
#include <whb/log_console.h>
#include <whb/proc.h>

/* ******************************************** */

/*
 * Hardware fault reporting.
 *
 * On Wii U an unhandled exception does not show a crash screen: the process
 * simply stops, the front buffer is never presented again, and the console
 * appears to be frozen. That makes a memory fault indistinguishable from a
 * genuine hang, and pressing HOME does nothing because nothing is servicing
 * ProcUI anymore.
 *
 * Installing a callback at least reports the faulting instruction address,
 * which is what turns "it froze" into something actionable.
 */

static const char *exceptionName(OSExceptionType type)
{
	switch (type) {
		case OS_EXCEPTION_TYPE_MACHINE_CHECK:      return "MACHINE_CHECK";
		case OS_EXCEPTION_TYPE_DSI:                return "DSI (bad memory access)";
		case OS_EXCEPTION_TYPE_ISI:                return "ISI";
		case OS_EXCEPTION_TYPE_ALIGNMENT:          return "ALIGNMENT";
		case OS_EXCEPTION_TYPE_PROGRAM:            return "PROGRAM (illegal instruction)";
		case OS_EXCEPTION_TYPE_FLOATING_POINT:     return "FLOATING_POINT";
		case OS_EXCEPTION_TYPE_DECREMENTER:        return "DECREMENTER";
		case OS_EXCEPTION_TYPE_SYSTEM_CALL:        return "SYSTEM_CALL";
		default:                                   return "UNKNOWN";
	}
}

static BOOL platformFaultHandler(OSContext *context)
{
	/* This runs on the faulting thread with the system in an unknown state,
	 * so keep it to logging only: no allocation, no SDL calls. */
	OSReport("[uMario U] FAULT: %s\n", exceptionName(OS_EXCEPTION_TYPE_DSI));

	if (context != NULL) {
		OSReport("  srr0 (faulting PC) = 0x%08X\n", context->srr0);
		OSReport("  srr1 (cause)       = 0x%08X\n", context->srr1);
		OSReport("  lr   (return addr) = 0x%08X\n", context->lr);
		OSReport("  r3=0x%08X r4=0x%08X r5=0x%08X\n",
		         context->gpr[3], context->gpr[4], context->gpr[5]);
	}

	WHBLogPrintf("[uMario U] FAULT at 0x%08X", context ? context->srr0 : 0);
	WHBLogConsoleDraw();

	return FALSE;
}

void PlatformCrashHandler_Init(void)
{
	OSSetExceptionCallback(OS_EXCEPTION_TYPE_DSI, platformFaultHandler);
	OSSetExceptionCallback(OS_EXCEPTION_TYPE_ISI, platformFaultHandler);
	OSSetExceptionCallback(OS_EXCEPTION_TYPE_PROGRAM, platformFaultHandler);
	OSSetExceptionCallback(OS_EXCEPTION_TYPE_ALIGNMENT, platformFaultHandler);
}
