#include "header.h"
#include "Core.h"
#include "IMG.h"
#include "CFG.h"
#include "Text.h"
#include "SDL2/SDL_mixer.h"
#include "../platform/PlatformInput.h"
#include "../platform/PlatformPaths.h"
#include "../platform/PlatformIntro.h"

/* ******************************************** */

// Controller state sampled once per frame and shared by the input handlers.
static PlatformPadState g_pad;

Map* CCore::oMap = new Map();
bool CCore::mouseLeftPressed = false;
bool CCore::mouseRightPressed = false;
int CCore::mouseX = 0;
int CCore::mouseY = 0;
bool CCore::quitGame = false;

bool CCore::movePressed = false;
bool CCore::keyMenuPressed = false;
bool CCore::keyS = false;
bool CCore::keyW = false;
bool CCore::keyA = false;
bool CCore::keyD = false;
bool CCore::keyShift = false;
bool CCore::keyAPressed = false;
bool CCore::keyDPressed = false;

float menuKeyDelay = 30.f;

CCore::CCore(void) {
	this->quitGame = false;
	this->iFPS = 0;
	this->iNumOfFPS = 0;
	this->lFPSTime = 0;

	/* Joystick/controller input is read directly through VPAD/KPAD in the
	 * platform layer, so SDL's OWN joystick backend must stay disabled.
	 * Enabling it would make SDL and this app read the same VPAD samples,
	 * which can drop input on hardware. */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO);
	
	/* The game renders at a fixed 960x544 internal resolution. GX2 scan-out
	 * scales that buffer to the TV (720p/1080p/480p) and to the GamePad
	 * (854x480), and SDL presents to both screens by default. */
	window = SDL_CreateWindow("uMario U", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, CCFG::GAME_WIDTH, CCFG::GAME_HEIGHT, SDL_WINDOW_SHOWN);

	if(window == NULL) {
		quitGame = true;
	}

	/* Pixel art must never be smoothed; set before the renderer is created so
	 * the window texture picks it up. */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	rR = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if(rR == NULL) {
		quitGame = true;
	}

	mainEvent = new SDL_Event();
	mainEvent->type = SDL_FIRSTEVENT;

	SDL_RenderSetLogicalSize(rR, CCFG::GAME_WIDTH, CCFG::GAME_HEIGHT);

	/* Required for PNG decoding of the intro card. */
	IMG_Init(IMG_INIT_PNG);
	
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	
	oMap = new Map(rR);
	CCFG::getMM()->setActiveOption(rR);
	CCFG::getSMBLOGO()->setIMG("super_mario_bros", rR);

	CCFG::getMusic()->PlayMusic();

	this->keyMenuPressed = this->movePressed = this->keyS = this->keyW = this->keyA = this->keyD = this->keyShift = false;

	this->keyAPressed = this->keyDPressed = this->firstDir = false;

	this->mouseX = this->mouseY = 0;

	CCFG::keyIDA = SDLK_a; //left
	CCFG::keyIDS = SDLK_s; //down
	CCFG::keyIDD = SDLK_d; //right
	CCFG::keyIDSpace = SDLK_SPACE; //jump
	CCFG::keyIDShift = SDLK_LSHIFT; //run
}

CCore::~CCore(void) {
	/* Order matters: free engine objects that own SDL_Textures and
	 * SDL_mixer handles while the renderer and audio device still exist,
	 * then tear the renderer down. This also prevents the static CCFG
	 * singletons from being destroyed after SDL_Quit() at process exit. */
	delete oMap;
	oMap = NULL;

	CCFG::destroyAll();

	delete mainEvent;
	mainEvent = NULL;

	if(rR != NULL) {
		SDL_DestroyRenderer(rR);
		rR = NULL;
	}

	if(window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}

	Mix_CloseAudio();
	IMG_Quit();
	SDL_Quit();
}

/* ******************************************** */

void CCore::inputLoop(){
	/* Sample every attached controller once per frame. */
	PlatformInput_Poll(&g_pad);

	if(g_pad.quitRequested) {
		quitGame = true;
	}

	/* The original Vita loop used one SDL event type as a proxy for "some
	 * button is active this frame"; keep that contract so the rest of the
	 * input code is untouched. */
	const bool anyButton = g_pad.up || g_pad.down || g_pad.left || g_pad.right ||
	                       g_pad.jump || g_pad.run || g_pad.start || g_pad.select;

	mainEvent->type = anyButton ? SDL_KEYDOWN : SDL_KEYUP;

	if (g_pad.jump){
		mainEvent->type = SDL_KEYDOWN;
	}
	else if (!g_pad.jump){
		if(CCFG::keySpace) {
			CCFG::keySpace = false;
		}
	}

	if (g_pad.run){
		mainEvent->type = SDL_KEYDOWN;
	}
	else if (!g_pad.run){
		if(keyShift) {
			oMap->getPlayer()->resetRun();
			keyShift = false;
		}
	}

	
	if (g_pad.up){
		mainEvent->type = SDL_KEYDOWN;
	}
	if (g_pad.down){
		mainEvent->type = SDL_KEYDOWN;
	}
	if (g_pad.left){
		mainEvent->type = SDL_KEYDOWN;
	}
	if (g_pad.right){
		mainEvent->type = SDL_KEYDOWN;
	}

	if (!g_pad.up){
		if(keyW){
			keyW = false;
		}
	}
	if (!g_pad.down){
		if (keyS){
			oMap->getPlayer()->setSquat(false);
			keyS = false;
		}
	}
	if (!g_pad.left){
		if(keyAPressed){
			keyAPressed = false;
		}
	}
	if (!g_pad.right){
		if(keyDPressed){
			keyDPressed = false;
		}
	}

	if (menuKeyDelay <= 0.f){
		keyMenuPressed = false;
		menuKeyDelay = 20.f;
	}
	else if (keyMenuPressed){
		menuKeyDelay -= 1.f;
	}

	Input();
}

void CCore::mainLoop() {
	/* PortBREW startup card: black -> fade in -> hold -> fade out -> game.
	 * Runs before the first gameplay frame and takes no input. */
	PlatformIntro_Play(rR, PlatformContentPath("imagery/portbrew.png").c_str());

	lFPSTime = SDL_GetTicks();

	while(!quitGame && mainEvent->type != SDL_QUIT) {
		frameTime = SDL_GetTicks();

		/* Drain the queue fully so background/resume events are not missed.
		 * SDL_QUIT (sent by ProcUI when the user exits) ends the loop. */
		SDL_Event event;
		while(SDL_PollEvent(&event)) {
			if(event.type == SDL_QUIT) {
				quitGame = true;
			}
		}

		if(quitGame) {
			break;
		}

		SDL_RenderClear(rR);

		CCFG::getMM()->setBackgroundColor(rR);
		SDL_RenderFillRect(rR, NULL);


		inputLoop();
		MouseInput();
		Update();
		Draw();
		

		SDL_RenderPresent(rR);
		
		if(SDL_GetTicks() - frameTime < MIN_FRAME_TIME) {
			SDL_Delay(MIN_FRAME_TIME - (SDL_GetTicks () - frameTime));
		}
	}
}

void CCore::Input() {
	switch(CCFG::getMM()->getViewID()) {
		case 2: case 7:
			if(!oMap->getInEvent()) {
				InputPlayer();
			} else {
				resetMove();
			}
			break;
		default:
			InputMenu();
			break;
	}
}

void CCore::InputMenu() {
	
	if(mainEvent->type == SDL_KEYDOWN) {
		if (g_pad.up){
			if(!keyMenuPressed) {
				CCFG::getMM()->keyPressed(0);
				keyMenuPressed = true;
			}
		}
		if (g_pad.down){
			if(!keyMenuPressed) {
				CCFG::getMM()->keyPressed(2);
				keyMenuPressed = true;
			}
		}
		if (g_pad.start || g_pad.jump){
			if(!keyMenuPressed) {
				CCFG::getMM()->enter();
				keyMenuPressed = true;
			}
		}
		if (g_pad.select || g_pad.run){
			if(!keyMenuPressed) {
				CCFG::getMM()->escape();
				keyMenuPressed = true;
			}
		}
		if (g_pad.left){
			if(!keyMenuPressed) {
				CCFG::getMM()->keyPressed(3);
				keyMenuPressed = true;
			}
		}
		if (g_pad.right){
			if(!keyMenuPressed) {
				CCFG::getMM()->keyPressed(1);
				keyMenuPressed = true;
			}
		}
	}

	if(mainEvent->type == SDL_KEYUP) {
		keyMenuPressed = false;
	}
}

void CCore::InputPlayer() {
	/* Pausing happens explicitly through Start/Select below; the original
	 * "focus lost" path relied on desktop window events which do not exist
	 * here, and the system handles its own backgrounding. */

	if(mainEvent->type == SDL_KEYUP) {
			if(!g_pad.right) {
				if(firstDir) {
					firstDir = false;
				}

				keyDPressed = false;
			}

			if(!g_pad.down) {
				oMap->getPlayer()->setSquat(false);
				keyS = false;
			}
		
			if(!g_pad.left) {
				if(!firstDir) {
					firstDir = true;
				}

				keyAPressed = false;
			}
		
			if(!g_pad.jump) {
				CCFG::keySpace = false;
			}
		
			if(!g_pad.run) {
				if(keyShift) {
					oMap->getPlayer()->resetRun();
					keyShift = false;
				}
			}

			keyMenuPressed = false;
	}

	if(mainEvent->type == SDL_KEYDOWN) {
		if(g_pad.right) {
			keyDPressed = true;
			if(!keyAPressed) {
				firstDir = true;
			}
		}

		if(g_pad.down) {
			if(!keyS) {
				keyS = true;
				if(!oMap->getUnderWater() && !oMap->getPlayer()->getInLevelAnimation()) oMap->getPlayer()->setSquat(true);
			}
		}
		
		if(g_pad.left) {
			keyAPressed = true;
			if(!keyDPressed) {
				firstDir = false;
			}
		}
		
		if(g_pad.jump) {
			if(!CCFG::keySpace) {
				oMap->getPlayer()->jump();
				CCFG::keySpace = true;
			}
		}
		
		if(g_pad.run) {
			if(!keyShift) {
				oMap->getPlayer()->startRun();
				keyShift = true;
			}
		}

		if(g_pad.start) {
			/* On Wii U "+" is the Start button. In gameplay it pauses, which
			 * matches the NES original; the engine's legacy Start binding
			 * toggled a debug grid overlay instead. */
			if(!keyMenuPressed && CCFG::getMM()->getViewID() == CCFG::getMM()->eGame) {
				CCFG::getMM()->resetActiveOptionID(CCFG::getMM()->ePasue);
				CCFG::getMM()->setViewID(CCFG::getMM()->ePasue);
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cPASUE);
				CCFG::getMusic()->PauseMusic();
				keyMenuPressed = true;
			}
		}

		if(g_pad.select) {
			if(!keyMenuPressed && CCFG::getMM()->getViewID() == CCFG::getMM()->eGame) {
				CCFG::getMM()->resetActiveOptionID(CCFG::getMM()->ePasue);
				CCFG::getMM()->setViewID(CCFG::getMM()->ePasue);
				CCFG::getMusic()->PlayChunk(CCFG::getMusic()->cPASUE);
				CCFG::getMusic()->PauseMusic();
				keyMenuPressed = true;
			}
		}
	}

	if(keyAPressed) {
		if(!oMap->getPlayer()->getMove() && firstDir == false && !oMap->getPlayer()->getChangeMoveDirection() && !oMap->getPlayer()->getSquat()) {
			oMap->getPlayer()->startMove();
			oMap->getPlayer()->setMoveDirection(false);
		} else if(!keyDPressed && oMap->getPlayer()->getMoveSpeed() > 0 && firstDir != oMap->getPlayer()->getMoveDirection()) {
			oMap->getPlayer()->setChangeMoveDirection();
		}
	}

	if(keyDPressed) {
		if(!oMap->getPlayer()->getMove() && firstDir == true && !oMap->getPlayer()->getChangeMoveDirection() && !oMap->getPlayer()->getSquat()) {
			oMap->getPlayer()->startMove();
			oMap->getPlayer()->setMoveDirection(true);
		} else if(!keyAPressed && oMap->getPlayer()->getMoveSpeed() > 0 && firstDir != oMap->getPlayer()->getMoveDirection()) {
			oMap->getPlayer()->setChangeMoveDirection();
		}
	}

	if(oMap->getPlayer()->getMove() && !keyAPressed && !keyDPressed) {
		oMap->getPlayer()->resetMove();
	}
}

void CCore::MouseInput() {
	switch(mainEvent->type) {
		case SDL_MOUSEBUTTONDOWN: {
			switch (mainEvent->button.button) {
				case SDL_BUTTON_LEFT:
					mouseLeftPressed = true;
					break;
				case SDL_BUTTON_RIGHT:
					mouseRightPressed = true;
					break;
			}
			break;
		}
		case SDL_MOUSEMOTION: {
			
			SDL_GetMouseState(&mouseX, &mouseY);
			//CCFG::getMM()->getConsole()->print("x:" + std::to_string(mouseX));
			//CCFG::getMM()->getConsole()->print("y:" + std::to_string(mouseY));
			break;
		}
		case SDL_MOUSEBUTTONUP: {
			switch (mainEvent->button.button) {
				case SDL_BUTTON_LEFT:
					mouseLeftPressed = false;
					break;
				case SDL_BUTTON_RIGHT:
					mouseRightPressed = false;
					break;
			}
			break;
		}
		case SDL_MOUSEWHEEL:
			if(mainEvent->wheel.timestamp > SDL_GetTicks() - 2) {
				//CCFG::getMM()->getLE()->mouseWheel(mainEvent->wheel.y);
			}
			break;
	}
}

void CCore::resetKeys() {
	movePressed = keyMenuPressed = keyS = keyW = keyA = keyD = CCFG::keySpace = keyShift = keyAPressed = keyDPressed = false;
}

void CCore::Update() {
	CCFG::getMM()->Update();
}


void CCore::Draw() {
	CCFG::getMM()->Draw(rR);
}

/* ******************************************** */

void CCore::resetMove() {
	this->keyAPressed = this->keyDPressed = false;
}

Map* CCore::getMap() {
	return oMap;
}