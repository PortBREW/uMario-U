#include "CFG.h"

/* ******************************************** */

CCFG::CCFG(void) { }

CCFG::~CCFG(void) {
	delete oText;
	delete oMM;
	delete tSMBLOGO;
}
/* ******************************************** */

int CCFG::GAME_WIDTH  = 960;
int CCFG::GAME_HEIGHT = 544;

Text* CCFG::oText = new Text();
CIMG* CCFG::tSMBLOGO = new CIMG();

MenuManager* CCFG::oMM = new MenuManager();

/* Music is created lazily: constructing it loads ~22 MB of WAV data, which
 * must not happen during static initialisation (before SDL audio and the
 * content mount are ready). */
Music* CCFG::oMusic = NULL;

bool CCFG::keySpace = false;

int CCFG::keyIDA = 0;
int CCFG::keyIDD = 0;
int CCFG::keyIDS = 0;
int CCFG::keyIDSpace = 0;
int CCFG::keyIDShift = 0;

bool CCFG::canMoveBackward = true;

/* ******************************************** */

void CCFG::destroyAll() {
	/* Music holds SDL_mixer handles; free them before the audio device goes. */
	if(oMusic != NULL) {
		delete oMusic;
		oMusic = NULL;
	}

	/* CIMG instances own SDL_Textures tied to the current renderer. */
	if(tSMBLOGO != NULL) {
		delete tSMBLOGO;
		tSMBLOGO = NULL;
	}

	if(oText != NULL) {
		delete oText;
		oText = NULL;
	}

	if(oMM != NULL) {
		delete oMM;
		oMM = NULL;
	}
}

/* ******************************************** */

Text* CCFG::getText() {
	return oText;
}

MenuManager* CCFG::getMM() {
	return oMM;
}

Music* CCFG::getMusic() {
	if(oMusic == NULL) {
		oMusic = new Music();
	}

	return oMusic;
}

CIMG* CCFG::getSMBLOGO() {
	return tSMBLOGO;
}

std::string CCFG::getKeyString(int keyID) {	if(keyID >= 97 && keyID <= 122) {
		return std::string(1, '0' + (keyID - 32) - 48);
	}

	if(keyID >= 48 && keyID <= 57) {
		return std::string(1, '0' + (keyID - 32) - 48);
	}

	//key translations
	/* PlayStation Vita button mappings removed; Wii U uses a fixed pad
	 * layout handled by the platform input layer. */

	//SDL_Keycode keycode;


	switch(keyID) {
		case SDLK_ESCAPE:
			return "ESCAPE";
		case SDLK_SPACE:
			return "SPACE";
		case SDLK_LSHIFT:
			return "LSHIFT";
		case SDLK_RSHIFT:
			return "RSHIFT";
		case SDLK_UP:
			return "UP";
		case SDLK_DOWN:
			return "DOWN";
		case SDLK_RIGHT:
			return "RIGHT";
		case SDLK_LEFT:
			return "LEFT";
		case SDLK_LCTRL:
			return "LCTRL";
		case SDLK_RCTRL:
			return "RCTRL";
	}

	return "NONE";
}