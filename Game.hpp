#ifndef Game_hpp
#define Game_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <map>

#include "People.hpp"
#include "Environment.hpp"
#include "ItemSys.hpp"
using namespace std;

class Game {
public:
	Game();
	~Game();

	void init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
	void initGameState();
	void handleEvents();
	void update();
	void render();
	void clean();

	bool running();

	void textureManager(string texture, SDL_Rect destRect);


	bool isRunning;
	SDL_Window* window; //window and renderer have to(?) be pointers because they aren't instanced until the Game::init function runs.
	SDL_Renderer* renderer; //should this be made static and public?

};

#endif
