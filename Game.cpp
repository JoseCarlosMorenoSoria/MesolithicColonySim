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
Animal anim;
//Environment envir;
SDL_Rect srcR, destR;
int sqdim = 15;

map<string, SDL_Texture*> texture_map;

int day_count = 0; //temporary measure for counting days passed
int hour_count = 0; //temporary measure for tracking when a new day starts, 20 updates == 1 day
int hours_in_day = 40;

int initint = 1;
void Game::initGameState() {
	int a = 0;
	destR.h = sqdim;
	destR.w = sqdim;
	destR.x = 0;
	destR.y = 0;
	Environment::Environment(hours_in_day);
	People::People();
	ItemSys::ItemSys();
	Animal::Animal(initint);//to avoid unintended execution of the constructor

	texture_map = {
	{"pics/dirt.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/dirt.png"))},
	{"pics/human.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human.png"))},
	{"pics/human_idle.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_idle.png"))},
	{"pics/human_eating.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_eating.png"))},
	{"pics/human_gathering.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_gathering.png"))},
	{"pics/human_sleeping.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_sleeping.png"))},
	{"pics/human_dead.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_dead.png"))},
	{"pics/berrybush.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/berrybush.png"))},
	{"pics/house.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/house.png"))},
	{"pics/sun.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sun.png"))},
	{"pics/moon.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/moon.png"))},
	{"pics/sky_day.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sky_day.png"))},
	{"pics/sky_night.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/sky_night.png"))},
	{"pics/debug.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/debug.png"))},
	{"pics/human_giving_food.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_giving_food.png"))},
	{"human_infant", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_infant.png"))},
	{"human_infant_dead", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_infant_dead.png"))},
	{"grain", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/grain.png"))},
	{"bread", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/bread.png"))},
	{"human_crafting", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_crafting.png"))},
	{"rock", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rock.png"))},
	{"mortar and pestle", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/mortar_pestle.png"))},
	{"deer", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer/deer.png"))},
	{"deer_resting", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer/deer_resting.png"))},
	{"deer_sleeping", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer/deer_sleeping.png"))},
	{"deer_dead", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer/deer_dead.png"))},
	{"deer_eating", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer/deer_eating.png"))},
	{"rabbit", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit/rabbit.png"))},
	{"rabbit_eating", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit/rabbit_eating.png"))},
	{"rabbit_dead", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit/rabbit_dead.png"))},
	{"rabbit_sleeping", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit/rabbit_sleeping.png"))},
	{"rabbit_resting", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit/rabbit_resting.png"))},
	{"trap", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/trap.png"))},
	{"deer_meat", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/deer_meat.png"))},
	{"rabbit_meat", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit_meat.png"))}
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



void Game::update() {
	hour_count++;
	if (hour_count == hours_in_day) {
		hour_count = 0;
		day_count++;
	}

	Environment::update(hours_in_day, hour_count, day_count);
	peep.update_all(day_count, hour_count, hours_in_day);
	anim.update_all(day_count, hour_count, hours_in_day);

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

	for (int x = 0; x < hours_in_day/2; x++) {
		destR.x = x * sqdim;
		destR.y = 0;
		if (Environment::Sky[x].has_sun) {
			textureManager("pics/sun.png", destR);
		}
		else if (Environment::Sky[x].has_moon) {
			textureManager("pics/moon.png", destR);
		}
		else {
			if (hour_count < hours_in_day/2) {
				textureManager("pics/sky_day.png", destR);
			}
			else {
				textureManager("pics/sky_night.png", destR);
			}
		}
	}

	for (int y = 0; y < Environment::map_y_max; y++) {
		for (int x = 0; x < Environment::map_x_max; x++) {
			destR.x = x * sqdim;
			destR.y = (y+1) * sqdim;
			bool no_item = false;
			int item_id = Environment::Map[y][x].item_id;
			if (item_id == -1) {
				no_item = true;
			}
			if (!no_item) {
				textureManager(ItemSys::item_list[ItemSys::item_by_id(item_id)].image, destR);
			}
			else {
				textureManager("pics/dirt.png", destR);
			}
		}
	}

	for (int i = 0; i < Animal::al.size(); i++) {
		destR.x = Animal::al[i].pos.x * sqdim;
		destR.y = (Animal::al[i].pos.y + 1) * sqdim;
		textureManager(Animal::al[i].current_image, destR);
	}

	for (int i = 0; i < People::pl.size(); i++) {
		destR.x = People::pl[i].pos.x * sqdim;
		destR.y = (People::pl[i].pos.y + 1) * sqdim;
		textureManager(People::pl[i].current_image, destR);
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

