#include "Game.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

Game game; //rather than use pointers like the tutorial, I am using only instances to avoid memory leaks


int main(int argc, const char* argv[]) {
	srand((unsigned int)time(NULL));//set seed for rand()

	//frame rate variables
	const int FPS = 12; //normally 60 FPS, currently less for testing. use 12FPS?
	const int frameDelay = 1000 / FPS;
	Uint32 frameStart;
	int frameTime;

	game = Game();
	game.init("Game Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (32*48), (32*27), false);
	game.initGameState();

	while (game.running()) {
		frameStart = SDL_GetTicks();//for capping frame rate below
		game.handleEvents(); 
		int speed = 1; //controls how many updates before a new frame is rendered. Useful for speeding up time/debugging
		for (int i = 0; i < speed; i++) {
			srand((unsigned int)time(NULL));//ensures random numbers don't repeat every program run
			game.update();
		}
		game.render();
		//cout << "frame" << endl;
		//caps the frame rate		might need to implement this differently (or remove it?) as it might also slow down background calculations, idk
		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime) {
			SDL_Delay(frameDelay - frameTime);
		}
	}
	game.clean();

	return 0;
}


