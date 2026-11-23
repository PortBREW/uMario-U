#pragma once

#ifndef PLATFORM_INTRO_H
#define PLATFORM_INTRO_H

#include <SDL2/SDL.h>

/**
 * Plays the PortBREW startup card: black screen, fade the logo in, hold
 * briefly, fade out, then return. Purely visual, takes no input, and must be
 * called before the game loop starts.
 *
 * \param rR      Renderer that will be used for the whole session.
 * \param logoPath Absolute path to the logo PNG (transparency preserved).
 */
void PlatformIntro_Play(SDL_Renderer *rR, const char *logoPath);

#endif
