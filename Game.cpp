#include "Game.hpp"

using namespace std; 

Game::Game() {
	isRunning = false;
}

Game::~Game() {

}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {
	int flags = 0;
	if (fullscreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		cout << "Subsystems initialized." << endl;
		window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (window) {
			cout << "Window created." << endl;
		}
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);//set background color
			cout << "Renderer created." << endl;
		}
		isRunning = true;
	}
	else {
		isRunning = false;
	}
}

People peep;
//Environment envir;
SDL_Rect srcR, destR;
int sqdim = 15;

map<string, SDL_Texture*> texture_map;

void Game::initGameState() {
	int a = 0;
	destR.h = sqdim;
	destR.w = sqdim;
	destR.x = 0;
	destR.y = 0;
	Environment::Environment();
	People::People();

	texture_map = {
	{"pics/dirt.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/dirt.png"))},
	{"pics/human.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human.png"))},
	{"pics/human_idle.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human_idle.png"))},
	{"pics/human_eating.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human_eating.png"))},
	{"pics/human_gathering.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human_gathering.png"))},
	{"pics/human_sleeping.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human_sleeping.png"))},
	{"pics/human_dead.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human_dead.png"))},
	{"pics/berrybush.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/berrybush.png"))},
	{"pics/house.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/house.png"))},
	{"pics/sun.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sun.png"))},
	{"pics/moon.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/moon.png"))},
	{"pics/sky_day.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sky_day.png"))},
	{"pics/sky_night.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sky_night.png"))},
	};
}

void Game::handleEvents() { //I think this handles user inputs such as keyboard and mouse
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}
}


int day_count = 0; //temporary measure for counting days passed
int hour_count = 0; //temporary measure for tracking when a new day starts, 20 updates == 1 day
void Game::update() {
	hour_count++;
	if (hour_count == 20) {
		hour_count = 0;
		day_count++;
	}

	Environment::update(20, hour_count);
	peep.update_all(day_count, hour_count);

}





//this function seems inefficient, don't know if there's a better way than creating and destroying per texture every tick
//SDL_Surface* tmpSurface;
//SDL_Texture* tmpTex;
//vector<SDL_Texture*> tmpTex_list;
void Game::textureManager(string texture, SDL_Rect destRect) {//textureManager feels like an unclear name, rename this function, this function takes in the image file and the location and size (destRect) to draw it on and adds it to the game's canvass (renderer)
	//tmpSurface = IMG_Load(texture.c_str());
	//tmpTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);//would it be better to store these in an array and access them from there?
	//SDL_FreeSurface(tmpSurface); //this might not be necessary, maybe move to the game.clean() function
	
	
	//SDL_RenderCopy(renderer, tmpTex, NULL, &destRect);
	SDL_RenderCopy(renderer, texture_map[texture], NULL, &destRect);
	//SDL_DestroyTexture(tmpTex); //this might not be necessary, maybe move to the game.clean() function
}

void Game::render() {
	SDL_RenderClear(renderer);
	//resets destR for printing environment
	destR.x = 0;
	destR.y = 0;

	for (int x = 0; x < 10; x++) {
		destR.x = x * sqdim;
		destR.y = 0;
		if (Environment::Sky[x].has_sun) {
			textureManager("pics/sun.png", destR);
		}
		else if (Environment::Sky[x].has_moon) {
			textureManager("pics/moon.png", destR);
		}
		else {
			if (hour_count < 10) {
				textureManager("pics/sky_day.png", destR);
			}
			else {
				textureManager("pics/sky_night.png", destR);
			}
		}
	}

	for (int y = 0; y < 50; y++) {
		for (int x = 0; x < 100; x++) {
			destR.x = x * sqdim;
			destR.y = (y+1) * sqdim;
			if (Environment::Map[y][x].has_food) {
				textureManager("pics/berrybush.png", destR);
			}
			else {
				textureManager("pics/dirt.png", destR);
			}
			if (Environment::Map[y][x].has_tent) {
				textureManager("pics/house.png", destR);
			}
		}
	}

	for (int i = 0; i < People::people_list.size(); i++) {
		destR.x = People::people_list[i].pos.x * sqdim;
		destR.y = (People::people_list[i].pos.y + 1) * sqdim;
		textureManager(People::people_list[i].current_image, destR);
	}

	SDL_RenderPresent(renderer);
}


void Game::clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	cout << "Game cleaned." << endl;
}

bool Game::running() {
	return isRunning;
}

