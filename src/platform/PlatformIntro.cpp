#include "PlatformIntro.h"

#include <SDL2/SDL_image.h>

#include <stdio.h>

/* ******************************************** */

#define INTRO_FADE_IN_MS   600
#define INTRO_HOLD_MS      1000
#define INTRO_FADE_OUT_MS  500

/* The logo should never dominate the screen; cap it to a fraction of the
 * available height so it looks the same on TV and GamePad. */
#define INTRO_LOGO_MAX_HEIGHT_FRACTION 0.62f

/* ******************************************** */

/**
 * Pumps the event queue during the intro. System events (quit / background)
 * are honoured so the console never becomes unresponsive, but game input is
 * deliberately ignored: the intro must not interact with gameplay.
 */
static bool introPumpEvents(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				return false;
			default:
				break;
		}
	}

	return true;
}

static void introPresentBlack(SDL_Renderer *rR)
{
	SDL_SetRenderDrawColor(rR, 0, 0, 0, 255);
	SDL_RenderClear(rR);
	SDL_RenderPresent(rR);
}

/* ******************************************** */

void PlatformIntro_Play(SDL_Renderer *rR, const char *logoPath)
{
	if (rR == NULL) {
		return;
	}

	/* -- Start on a black screen so the first visible frame is a fade. */
	int outWidth = 960, outHeight = 544;
	SDL_GetRendererOutputSize(rR, &outWidth, &outHeight);
	if (outWidth <= 0 || outHeight <= 0) {
		outWidth = 960;
		outHeight = 544;
	}

	introPresentBlack(rR);

	/* -- Load the logo. If anything goes wrong we simply skip the intro
	 *    rather than blocking the game. */
	SDL_Texture *logo = NULL;
	int logoWidth = 0, logoHeight = 0;

	if (logoPath != NULL) {
		SDL_Surface *surface = IMG_Load(logoPath);
		if (surface != NULL) {
			logoWidth = surface->w;
			logoHeight = surface->h;

			logo = SDL_CreateTextureFromSurface(rR, surface);
			SDL_FreeSurface(surface);
		}
	}

	if (logo == NULL || logoWidth <= 0 || logoHeight <= 0) {
		if (logo != NULL) {
			SDL_DestroyTexture(logo);
		}
		return;
	}

	SDL_SetTextureBlendMode(logo, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(logo, 255);

	/* -- Fit the logo while preserving its aspect ratio. */
	const float maxHeight = (float) outHeight * INTRO_LOGO_MAX_HEIGHT_FRACTION;
	const float maxWidth  = (float) outWidth  * 0.70f;

	float scale = maxHeight / (float) logoHeight;
	if ((float) logoWidth * scale > maxWidth) {
		scale = maxWidth / (float) logoWidth;
	}

	SDL_Rect dst;
	dst.w = (int) ((float) logoWidth  * scale);
	dst.h = (int) ((float) logoHeight * scale);
	dst.x = (outWidth  - dst.w) / 2;
	dst.y = (outHeight - dst.h) / 2;

	/* -- Timeline. */
	const uint32_t start = SDL_GetTicks();
	const uint32_t fadeInEnd  = start + INTRO_FADE_IN_MS;
	const uint32_t holdEnd    = fadeInEnd + INTRO_HOLD_MS;
	const uint32_t fadeOutEnd = holdEnd + INTRO_FADE_OUT_MS;

	bool running = true;

	while (running) {
		const uint32_t now = SDL_GetTicks();
		int logoAlpha = 255;
		int blackAlpha = 0;

		if (now < fadeInEnd) {
			/* Fade the logo in over black. */
			const uint32_t elapsed = now - start;
			logoAlpha = (int) ((uint64_t) 255 * elapsed / INTRO_FADE_IN_MS);
		} else if (now < holdEnd) {
			logoAlpha = 255;
		} else if (now < fadeOutEnd) {
			/* Fade the whole image to black, logo stays fully opaque. */
			const uint32_t elapsed = now - holdEnd;
			blackAlpha = (int) ((uint64_t) 255 * elapsed / INTRO_FADE_OUT_MS);
		} else {
			break;
		}

		if (logoAlpha < 0)   logoAlpha = 0;
		if (logoAlpha > 255) logoAlpha = 255;
		if (blackAlpha < 0)   blackAlpha = 0;
		if (blackAlpha > 255) blackAlpha = 255;

		SDL_SetRenderDrawColor(rR, 0, 0, 0, 255);
		SDL_RenderClear(rR);

		SDL_SetTextureAlphaMod(logo, (Uint8) logoAlpha);
		SDL_RenderCopy(rR, logo, NULL, &dst);

		if (blackAlpha > 0) {
			SDL_SetRenderDrawBlendMode(rR, SDL_BLENDMODE_BLEND);
			SDL_SetRenderDrawColor(rR, 0, 0, 0, (Uint8) blackAlpha);
			SDL_RenderFillRect(rR, NULL);
			SDL_SetRenderDrawBlendMode(rR, SDL_BLENDMODE_NONE);
		}

		SDL_RenderPresent(rR);

		if (!introPumpEvents()) {
			running = false;
		}

		/* Roughly 60 Hz so the fade is smooth without burning CPU. */
		SDL_Delay(16);
	}

	SDL_DestroyTexture(logo);

	/* -- Guarantee a clean black frame before the game draws its own first
	 *    frame, so there is no flash of stale content. */
	introPresentBlack(rR);
}
