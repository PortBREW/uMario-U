#include "IMG.h"
#include "../platform/PlatformPaths.h"

/* ******************************************** */

CIMG::CIMG(void) {
	/* Textures created with the default constructor are filled in later by
	 * setIMG(); initialise the pointer so a failed load cannot leave garbage
	 * behind for ~CIMG() to free. */
	this->tIMG = NULL;
	rRect.x = rRect.y = rRect.w = rRect.h = 0;
}

CIMG::CIMG(std::string fileName, SDL_Renderer* rR) {
	/* setIMG() destroys an existing texture before replacing it, so the
	 * pointer MUST be valid before the first call. Without this the very
	 * first load runs SDL_DestroyTexture() on whatever garbage happened to
	 * be in the freshly allocated object. That is harmless while the heap
	 * still hands out zeroed pages and fatal once it starts recycling them. */
	this->tIMG = NULL;
	rRect.x = rRect.y = rRect.w = rRect.h = 0;

	setIMG(fileName, rR);
}

CIMG::~CIMG(void) {
	if(tIMG != NULL) {
		SDL_DestroyTexture(tIMG);
		tIMG = NULL;
	}
}

/* ******************************************** */

void CIMG::Draw(SDL_Renderer* rR, int iXOffset, int iYOffset) {
	rRect.x = iXOffset;
	rRect.y = iYOffset;

	SDL_RenderCopy(rR, tIMG, NULL, &rRect);
}

void CIMG::Draw(SDL_Renderer* rR, int iXOffset, int iYOffset, bool bRotate) {
	rRect.x = iXOffset;
	rRect.y = iYOffset;

	if(!bRotate) {
		SDL_RenderCopy(rR, tIMG, NULL, &rRect);
	} else {
		SDL_RenderCopyEx(rR, tIMG, NULL, &rRect, 180.0, NULL, SDL_FLIP_VERTICAL);
	}
}

void CIMG::DrawVert(SDL_Renderer* rR, int iXOffset, int iYOffset) {
	rRect.x = iXOffset;
	rRect.y = iYOffset;

	SDL_RenderCopyEx(rR, tIMG, NULL, &rRect, 180.0, NULL, SDL_FLIP_HORIZONTAL);
}

void CIMG::Draw(SDL_Renderer* rR, SDL_Rect rCrop, SDL_Rect rRect) {
	SDL_RenderCopy(rR, tIMG, &rCrop, &rRect);
}

/* ******************************************** */

void CIMG::setIMG(std::string fileName, SDL_Renderer* rR) {
	if(tIMG != NULL) {
		/* Called again on an existing texture (the logo uses the default
		 * constructor and then setIMG); release the old one first. */
		SDL_DestroyTexture(tIMG);
		tIMG = NULL;
	}

	std::string filePath = PlatformFilesPath() + "images/" + fileName + ".bmp";
	SDL_Surface* loadedSurface = SDL_LoadBMP(filePath.c_str());

	if(loadedSurface != NULL) {
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 255, 0, 255));

		tIMG = SDL_CreateTextureFromSurface(rR, loadedSurface);
		SDL_FreeSurface(loadedSurface);
	}

	if(tIMG == NULL) {
		/* Missing art must not crash the port: fall back to a fully
		 * transparent 1x1 texture so the renderer stays valid and the game
		 * keeps running with an invisible sprite instead of dying. */
		tIMG = SDL_CreateTexture(rR, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 1, 1);

		if(tIMG != NULL) {
			Uint32 transparent = 0;
			SDL_UpdateTexture(tIMG, NULL, &transparent, sizeof(Uint32));
			SDL_SetTextureBlendMode(tIMG, SDL_BLENDMODE_BLEND);
		}

		rRect.x = rRect.y = 0;
		rRect.w = rRect.h = 1;
		return;
	}

	int iWidth, iHeight;
	SDL_QueryTexture(tIMG, NULL, NULL, &iWidth, &iHeight);
	
	rRect.x  = 0;
	rRect.y = 0;
	rRect.w = iWidth;
	rRect.h = iHeight;
}

SDL_Texture* CIMG::getIMG() {
	return tIMG;
}

SDL_Rect CIMG::getRect() {
	return rRect;
}