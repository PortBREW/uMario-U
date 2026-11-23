#pragma once

#ifndef PLATFORM_CRASH_H
#define PLATFORM_CRASH_H

/**
 * Installs hardware exception callbacks that report the faulting instruction
 * address. On Wii U an unhandled exception silently stops the process, so
 * without this a memory fault looks exactly like a freeze.
 */
void PlatformCrashHandler_Init(void);

#endif
