#pragma once

#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

#include <stdint.h>

/**
 * Abstract controller state, independent of the underlying Wii U API.
 *
 * The original uMario input code reads a fixed set of "virtual buttons"
 * every frame.  We keep that model, but fill it from every attached
 * controller (GamePad, Wii Remote + Nunchuk, Classic Controller,
 * Wii U Pro Controller) instead of the PS Vita's SceCtrl API.
 */
struct PlatformPadState
{
	// Directional input (d-pad and/or analogue stick).
	bool up;
	bool down;
	bool left;
	bool right;

	// Bitmask of pressed directionals in the *current* frame.
	// Up = 1, Down = 2, Left = 4, Right = 8.
	uint32_t directionMask;

	// Action buttons.
	bool jump;  // A / 2
	bool run;   // B / 1  (also X / Y)
	bool start; // + / START
	bool select;// - / SELECT

	// High level helpers used by the menu code.
	bool confirmPressed; // start edge
	bool backPressed;    // select edge

	// True if a controller is currently connected and being read.
	bool connected;

	// Exit request from the system (only set by the platform layer).
	bool quitRequested;
};

/**
 * Initialises the input layer (VPAD + KPAD).
 */
void PlatformInput_Init(void);

/**
 * Samples every controller and merges the result into \a out.
 * Call once per frame, before the game consumes input.
 */
void PlatformInput_Poll(PlatformPadState *out);

/**
 * Shuts the input layer down again.
 */
void PlatformInput_Shutdown(void);

#endif
