#include "PlatformInput.h"

#include <vpad/input.h>
#include <padscore/kpad.h>
#include <padscore/wpad.h>

#include <string.h>

/* ******************************************** */

#define PLATFORM_ANALOG_DEADZONE  0.35f

static bool s_initialised = false;

/* ******************************************** */

void PlatformInput_Init(void)
{
	if (s_initialised) {
		return;
	}

	VPADInit();
	KPADInit();

	/* Both APIs give us per-frame "trigger" (newly pressed) and "release"
	 * (newly released) masks on top of the held state, which maps well onto
	 * the frame-based input model used by the game. */
	VPADSetBtnRepeat(VPAD_CHAN_0, 0.0f, 0.0f);
	KPADSetButtonProcMode(WPAD_CHAN_0, KPAD_BUTTON_PROC_MODE_TIGHT);

	s_initialised = true;
}

void PlatformInput_Shutdown(void)
{
	if (!s_initialised) {
		return;
	}

	KPADShutdown();
	VPADShutdown();

	s_initialised = false;
}

/* ******************************************** */

static inline bool axisToDirection(float value, bool positive)
{
	return positive ? (value > PLATFORM_ANALOG_DEADZONE)
	                : (value < -PLATFORM_ANALOG_DEADZONE);
}

static void applyDirection(PlatformPadState *out, bool up, bool down, bool left, bool right)
{
	out->up |= up;
	out->down |= down;
	out->left |= left;
	out->right |= right;
}

/* ******************************************** */
/* GamePad (DRC)                                */
/* ******************************************** */

static void pollGamePad(PlatformPadState *out)
{
	VPADStatus status;
	VPADReadError error = VPAD_READ_SUCCESS;
	int32_t read;

	memset(&status, 0, sizeof(status));

	read = VPADRead(VPAD_CHAN_0, &status, 1, &error);
	if (read <= 0 || error != VPAD_READ_SUCCESS) {
		return;
	}

	const uint32_t held = status.hold;

	out->connected = true;

	applyDirection(out,
		(held & (VPAD_BUTTON_UP   | VPAD_STICK_L_EMULATION_UP))   != 0,
		(held & (VPAD_BUTTON_DOWN | VPAD_STICK_L_EMULATION_DOWN)) != 0,
		(held & (VPAD_BUTTON_LEFT | VPAD_STICK_L_EMULATION_LEFT)) != 0,
		(held & (VPAD_BUTTON_RIGHT| VPAD_STICK_L_EMULATION_RIGHT)) != 0);

	/* Some pads report stick directions only through the emulated button
	 * masks depending on the system setting, so fall back to the raw axes. */
	applyDirection(out,
		axisToDirection(status.leftStick.y, true),
		axisToDirection(status.leftStick.y, false),
		axisToDirection(status.leftStick.x, false),
		axisToDirection(status.leftStick.x, true));

	out->jump   |= (held & (VPAD_BUTTON_A | VPAD_BUTTON_Y)) != 0;
	out->run    |= (held & (VPAD_BUTTON_B | VPAD_BUTTON_X)) != 0;
	out->start  |= (held & VPAD_BUTTON_PLUS)  != 0;
	out->select |= (held & VPAD_BUTTON_MINUS) != 0;

	/* Shoulder buttons double as run/jump so that holding ZR to dash feels
	 * natural and matches the original "hold a face button" behaviour. */
	out->run  |= (held & (VPAD_BUTTON_ZR | VPAD_BUTTON_R)) != 0;
	out->jump |= (held & (VPAD_BUTTON_ZL | VPAD_BUTTON_L)) != 0;

	/* Newly pressed / released edges for the current frame. */
	out->confirmPressed |= (status.trigger & VPAD_BUTTON_PLUS)  != 0;
	out->backPressed    |= (status.trigger & VPAD_BUTTON_MINUS) != 0;
}

/* ******************************************** */
/* Wii Remote / Nunchuk / Classic / Pro         */
/* ******************************************** */

static void pollWpad(PlatformPadState *out)
{
	static const int channels[] = {
		WPAD_CHAN_0, WPAD_CHAN_1, WPAD_CHAN_2, WPAD_CHAN_3
	};

	for (unsigned int i = 0; i < sizeof(channels) / sizeof(channels[0]); i++) {
		KPADStatus status;
		KPADError error = KPAD_ERROR_OK;
		int32_t read;

		memset(&status, 0, sizeof(status));

		read = KPADReadEx((KPADChan) channels[i], &status, 1, &error);
		if (read != 1 || error != KPAD_ERROR_OK) {
			continue;
		}

		switch (status.extensionType) {
			case WPAD_EXT_PRO_CONTROLLER: {
				const uint32_t held = status.pro.hold;

				out->connected = true;

				applyDirection(out,
					(held & WPAD_CLASSIC_BUTTON_UP)    != 0,
					(held & WPAD_CLASSIC_BUTTON_DOWN)  != 0,
					(held & WPAD_CLASSIC_BUTTON_LEFT)  != 0,
					(held & WPAD_CLASSIC_BUTTON_RIGHT) != 0);

				applyDirection(out,
					axisToDirection(status.pro.leftStick.y, true),
					axisToDirection(status.pro.leftStick.y, false),
					axisToDirection(status.pro.leftStick.x, false),
					axisToDirection(status.pro.leftStick.x, true));

				out->jump   |= (held & WPAD_CLASSIC_BUTTON_A) != 0;
				out->run    |= (held & WPAD_CLASSIC_BUTTON_B) != 0;
				out->jump   |= (held & WPAD_CLASSIC_BUTTON_ZR) != 0;
				out->run    |= (held & WPAD_CLASSIC_BUTTON_ZL) != 0;
				out->start  |= (held & WPAD_CLASSIC_BUTTON_PLUS)  != 0;
				out->select |= (held & WPAD_CLASSIC_BUTTON_MINUS) != 0;

				out->confirmPressed |= (status.pro.trigger & WPAD_CLASSIC_BUTTON_PLUS)  != 0;
				out->backPressed    |= (status.pro.trigger & WPAD_CLASSIC_BUTTON_MINUS) != 0;
				break;
			}
			case WPAD_EXT_CLASSIC:
			case WPAD_EXT_MPLUS_CLASSIC: {
				const uint32_t held = status.classic.hold;

				out->connected = true;

				applyDirection(out,
					(held & WPAD_CLASSIC_BUTTON_UP)    != 0,
					(held & WPAD_CLASSIC_BUTTON_DOWN)  != 0,
					(held & WPAD_CLASSIC_BUTTON_LEFT)  != 0,
					(held & WPAD_CLASSIC_BUTTON_RIGHT) != 0);

				applyDirection(out,
					axisToDirection(status.classic.leftStick.y, true),
					axisToDirection(status.classic.leftStick.y, false),
					axisToDirection(status.classic.leftStick.x, false),
					axisToDirection(status.classic.leftStick.x, true));

				out->jump   |= (held & (WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_ZR)) != 0;
				out->run    |= (held & (WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_ZL)) != 0;
				out->start  |= (held & WPAD_CLASSIC_BUTTON_PLUS)  != 0;
				out->select |= (held & WPAD_CLASSIC_BUTTON_MINUS) != 0;

				out->confirmPressed |= (status.classic.trigger & WPAD_CLASSIC_BUTTON_PLUS)  != 0;
				out->backPressed    |= (status.classic.trigger & WPAD_CLASSIC_BUTTON_MINUS) != 0;
				break;
			}
			case WPAD_EXT_NUNCHUK:
			case WPAD_EXT_MPLUS_NUNCHUK: {
				const uint32_t held    = status.hold;
				const uint32_t nHeld   = status.nunchuk.hold;

				out->connected = true;

				applyDirection(out,
					(held & WPAD_BUTTON_UP)    != 0,
					(held & WPAD_BUTTON_DOWN)  != 0,
					(held & WPAD_BUTTON_LEFT)  != 0,
					(held & WPAD_BUTTON_RIGHT) != 0);

				applyDirection(out,
					axisToDirection(status.nunchuk.stick.y, true),
					axisToDirection(status.nunchuk.stick.y, false),
					axisToDirection(status.nunchuk.stick.x, false),
					axisToDirection(status.nunchuk.stick.x, true));

				/* A + Z jump, B + C run: keeps the classic NES-ish feel while
				 * remaining comfortable on the sideways remote. */
				out->jump |= (held & (WPAD_BUTTON_A | WPAD_BUTTON_2)) != 0;
				out->run  |= (held & (WPAD_BUTTON_B | WPAD_BUTTON_1)) != 0;
				out->jump |= (nHeld & WPAD_NUNCHUK_BUTTON_Z) != 0;
				out->run  |= (nHeld & WPAD_NUNCHUK_BUTTON_C) != 0;

				out->start  |= (held & WPAD_BUTTON_PLUS)  != 0;
				out->select |= (held & WPAD_BUTTON_MINUS) != 0;

				out->confirmPressed |= (status.trigger & WPAD_BUTTON_PLUS)  != 0;
				out->backPressed    |= (status.trigger & WPAD_BUTTON_MINUS) != 0;
				break;
			}
			case WPAD_EXT_CORE:
			case WPAD_EXT_MPLUS:
			default: {
				const uint32_t held = status.hold;

				if (held == 0) {
					break;
				}

				out->connected = true;

				applyDirection(out,
					(held & WPAD_BUTTON_UP)    != 0,
					(held & WPAD_BUTTON_DOWN)  != 0,
					(held & WPAD_BUTTON_LEFT)  != 0,
					(held & WPAD_BUTTON_RIGHT) != 0);

				out->jump |= (held & (WPAD_BUTTON_A | WPAD_BUTTON_2)) != 0;
				out->run  |= (held & (WPAD_BUTTON_B | WPAD_BUTTON_1)) != 0;

				out->start  |= (held & WPAD_BUTTON_PLUS)  != 0;
				out->select |= (held & WPAD_BUTTON_MINUS) != 0;

				out->confirmPressed |= (status.trigger & WPAD_BUTTON_PLUS)  != 0;
				out->backPressed    |= (status.trigger & WPAD_BUTTON_MINUS) != 0;
				break;
			}
		}
	}
}

/* ******************************************** */

void PlatformInput_Poll(PlatformPadState *out)
{
	if (out == NULL) {
		return;
	}

	memset(out, 0, sizeof(*out));

	if (!s_initialised) {
		PlatformInput_Init();
	}

	pollGamePad(out);
	pollWpad(out);

	out->directionMask = 0;
	if (out->up)    out->directionMask |= 1u;
	if (out->down)  out->directionMask |= 2u;
	if (out->left)  out->directionMask |= 4u;
	if (out->right) out->directionMask |= 8u;

	/* If the GamePad vanishes mid-session (battery, out of range) and no
	 * other controller is present, treat the session as pad-less. */
	if (!out->connected) {
		memset(out, 0, sizeof(*out));
	}
}
