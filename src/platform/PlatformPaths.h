#pragma once

#ifndef PLATFORM_PATHS_H
#define PLATFORM_PATHS_H

#include <string>

/**
 * Root of the writable/readable data volume.
 *
 * - A .wuhb mounts its bundled content directory at /vol/content.
 * - A bare .rpx launched from the Homebrew Launcher has no /vol/content, so
 *   we fall back to the SD card.
 */
std::string PlatformContentPath(const std::string &relative);

/** Directory that holds the game's "files" tree, with trailing slash. */
std::string PlatformFilesPath();

/**
 * Resolves the mount root once at startup and reports which one is active.
 * Returns true when a usable content root was found.
 */
bool PlatformPaths_Init(void);

/** Human readable name of the active content root (for logs / about screen). */
const char *PlatformPaths_Root(void);

#endif
