#include "PlatformPaths.h"

#include <stdio.h>
#include <string.h>

#include <whb/sdcard.h>
#include <coreinit/filesystem.h>

/* ******************************************** */

static char s_root[256] = { 0 };
static bool s_initialised = false;
static bool s_sdMounted = false;

/* ******************************************** */

static bool fileExists(const char *path)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) {
		return false;
	}

	fclose(file);
	return true;
}

/**
 * Probes a candidate directory for the engine's marker asset. The font is the
 * first thing Map() loads, so its presence means the tree is complete.
 */
static bool probeRoot(const char *root)
{
	char probe[512];

	snprintf(probe, sizeof(probe), "%s/uNext/files/images/font.bmp", root);
	if (fileExists(probe)) {
		return true;
	}

	/* Also accept a flattened layout where "files" sits directly in root. */
	snprintf(probe, sizeof(probe), "%s/files/images/font.bmp", root);
	if (fileExists(probe)) {
		return true;
	}

	return false;
}

/* ******************************************** */

bool PlatformPaths_Init(void)
{
	if (s_initialised) {
		return s_root[0] != 0;
	}

	s_initialised = true;

	/* The SD card is not mounted by default; mount it so the SD-card install
	 * layout works for a bare .rpx launched from the Homebrew Launcher. */
	s_sdMounted = WHBMountSdCard() ? true : false;

	/* Build the candidate list dynamically so the SD mount point comes from
	 * wut rather than a hardcoded path. */
	char sdApp[256]   = { 0 };
	char sdAppSp[256] = { 0 };
	char sdRoot[256]  = { 0 };

	if (s_sdMounted) {
		const char *mount = WHBGetSdCardMountPath();
		if (mount == NULL) {
			mount = "/vol/external01";
		}

		snprintf(sdRoot,  sizeof(sdRoot),  "%s", mount);
		snprintf(sdApp,   sizeof(sdApp),   "%s/wiiu/apps/uMarioU",  mount);
		snprintf(sdAppSp, sizeof(sdAppSp), "%s/wiiu/apps/uMario U", mount);
	}

	const char *candidates[6];
	unsigned int count = 0;

	/* .wuhb bundle layout first. */
	candidates[count++] = "/vol/content";

	if (s_sdMounted) {
		candidates[count++] = sdApp;
		candidates[count++] = sdAppSp;
		candidates[count++] = sdRoot;
	}

	for (unsigned int i = 0; i < count; i++) {
		if (candidates[i][0] == 0) {
			continue;
		}

		if (probeRoot(candidates[i])) {
			snprintf(s_root, sizeof(s_root), "%s", candidates[i]);
			return true;
		}
	}

	/* Nothing found: default to the .wuhb layout so that error messages show
	 * sensible paths, and let the individual loads fail loudly. */
	snprintf(s_root, sizeof(s_root), "%s", "/vol/content");
	return false;
}

const char *PlatformPaths_Root(void)
{
	if (!s_initialised) {
		PlatformPaths_Init();
	}

	return s_root;
}

/* ******************************************** */

std::string PlatformContentPath(const std::string &relative)
{
	std::string path = PlatformPaths_Root();

	if (!relative.empty() && relative[0] != '/') {
		path += '/';
	}

	path += relative;
	return path;
}

std::string PlatformFilesPath()
{
	std::string root = PlatformPaths_Root();

	/* Two layouts are supported:
	 *   <root>/uNext/files/...   (mirrors the original repository)
	 *   <root>/files/...         (flattened content directory) */
	{
		std::string nested = root + "/uNext/files/images/font.bmp";
		FILE *file = fopen(nested.c_str(), "rb");
		if (file != NULL) {
			fclose(file);
			return root + "/uNext/files/";
		}
	}

	return root + "/files/";
}
