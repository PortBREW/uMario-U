#include "header.h"
#include "Core.h"
#include "../platform/PlatformInput.h"
#include "../platform/PlatformPaths.h"
#include "../platform/PlatformCrash.h"

/* ******************************************** */

int main(int argc, char *argv[])
{
	/* Install fault reporting first, so anything that goes wrong later
	 * produces an address instead of a silent freeze. */
	PlatformCrashHandler_Init();

	/* Resolve /vol/content (or the SD-card fallback) before anything loads
	 * assets, then bring up all controller backends. */
	PlatformPaths_Init();
	PlatformInput_Init();

	/* One engine instance for the whole session. Constructed on the heap so
	 * the large Map object never lands on the (small) main stack. */
	CCore *oCore = new CCore();

	if(oCore != NULL) {
		oCore->mainLoop();
		delete oCore;
	}

	PlatformInput_Shutdown();

	return 0;
}
