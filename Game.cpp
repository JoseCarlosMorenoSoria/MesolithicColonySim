#include "Game.hpp"

using namespace std; 
SDL_Renderer* Game::renderer;
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
Player player;
//Environment envir;
SDL_Rect srcR, destR;
int sqdim = 16;

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
	Player::Player(a);
	Environment::Environment(hours_in_day);
	People::People(initint);
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
	{"rabbit_meat", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rabbit_meat.png"))},
	{"pics/shallow_water.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/shallow_water.png"))},
	{"tree", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/tree.png"))},
	{"wood", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/wood.png"))},
	{"monument", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/monument.png"))},
	{"stick", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/stick.png"))},
	{"head", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/head.png"))}
	};


	create_human();
}

void Game::handleEvents() { //this handles user inputs such as keyboard and mouse
	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
		
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_LEFT:
			player.move("W");
			break;
		case SDLK_RIGHT:
			player.move("E");
			break;
		case SDLK_UP:
			player.move("N");
			break;
		case SDLK_DOWN:
			player.move("S");
			break;
		default:
			break;
		}
		break;
		
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}

	/*
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	if (keystates[SDL_SCANCODE_RIGHT]) {
		player.move("E");
	}
	if (keystates[SDL_SCANCODE_LEFT]) {
		player.move("W");
	}
	if (keystates[SDL_SCANCODE_UP]) {
		player.move("N");
	}
	if (keystates[SDL_SCANCODE_DOWN]) {
		player.move("S");
	}
	*/


}



void Game::update() {
	



	hour_count++;
	if (hour_count == hours_in_day) {
		hour_count = 0;
		day_count++;
	}

	//Environment::update(hours_in_day, hour_count, day_count);
	//peep.update_all(day_count, hour_count, hours_in_day);
	//anim.update_all(day_count, hour_count, hours_in_day);
	//player.update();
}





//this function seems inefficient, don't know if there's a better way than creating and destroying per texture every tick
//SDL_Surface* tmpSurface;
//SDL_Texture* tmpTex;
//vector<SDL_Texture*> tmpTex_list;
const SDL_Point *c;
void Game::textureManager(string texture, SDL_Rect destRect, int angle, SDL_Point center) {//textureManager feels like an unclear name, rename this function, this function takes in the image file and the location and size (destRect) to draw it on and adds it to the game's canvass (renderer)
	//tmpSurface = IMG_Load(texture.c_str());
	//tmpTex = SDL_CreateTextureFromSurface(renderer, tmpSurface);//would it be better to store these in an array and access them from there?
	//SDL_FreeSurface(tmpSurface); //this might not be necessary, maybe move to the game.clean() function
	
	if (center.x == -1) {
		c = NULL;
	}
	else{
		c = &center;
	}
	
	SDL_RenderCopyEx(renderer, texture_map[texture], NULL, &destRect, angle, c, SDL_FLIP_NONE);
	//(if the parameter for center (c) is NULL, rotation will be done around dstrect.w / 2, dstrect.h / 2).

	//r += 10;
	//r = 90;
			//SDL_RenderCopy(renderer, tmpTex, NULL, &destRect);
	//SDL_RenderCopy(renderer, texture_map[texture], NULL, &destRect);
			//SDL_DestroyTexture(tmpTex); //this might not be necessary, maybe move to the game.clean() function
}



//float offset = 0.5;





int rd = 1;
int ld = 1;
bool flip = false;
void Game::render() {
	SDL_RenderClear(renderer);
	//resets destR for printing environment
	destR.x = 0;
	destR.y = 0;

	models[0].render_skeleton();
	
	if (flip) {
		if (models[0].pose_transform(models[0].pose_walk_sideways2,2)) {
			flip = !flip;
		}
	}
	else {
		if (models[0].pose_transform(models[0].pose_walk_sideways1,2)) {
			flip = !flip;
		}

	}

	//human.current_pose["right_arm"].r += 6;
	//human.current_pose["left_arm"].r -= 6;
	//human.current_pose["right_leg"].r += 6;
	//human.current_pose["left_leg"].r -= 6;


	//human.current_pose["right_arm"].x -= 0.1;
	//if (human.current_pose["right_arm"].x < human.resting_pose["right_arm"].x - 2*32) {
	//	human.current_pose["right_arm"].x = human.resting_pose["right_arm"].x;
	//}


	/*
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
				if (Environment::Map[y][x].terrain == "dirt") {
					textureManager("pics/dirt.png", destR);
				}
				else if (Environment::Map[y][x].terrain == "water") {
					textureManager("pics/shallow_water.png", destR);
				}
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
		destR.x += People::pl[i].px_x;
		destR.y += People::pl[i].px_y;
		textureManager(People::pl[i].current_image, destR);
	}
	*/


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

void Game::create_human() {//animation should be moved to its own class, fix this
	/* for testing new poses
	bone h = { "head", 0,0, 1,1, 0, {-1,0} };
	bone ra = { "stick", 0.5,0.8,	1,1, 35, {0,32 / 2} };
	bone rl = { "stick", 0.5,1.5,	1,1, 55, {0,32 / 2} };
	bone t = { "stick", 0,1, 1,1, 90, {-1,0} };
	bone la = { "stick", 0.5,0.8,	1,1, 145, {0,32 / 2} };
	bone ll = { "stick", 0.5,1.5,	1,1, 125, {0,32 / 2} };
	*/
	skeleton human = { 32, {10,10} };//scale and center point x,y

	

	//resting pose
	
	bone h = { "head", 0,0, 1,1, 0, {-1,0} };
	bone ra = { "stick", 0.5,0.8,	1,1, 45, {0,32 / 2} };
	bone rl = { "stick", 0.5,1.5,	1,1, 75, {0,32 / 2} };
	bone t = { "stick", 0,1, 1,1, 90, {-1,0} };
	bone la = { "stick", 0.5,0.8,	1,1, 135, {0,32 / 2} };
	bone ll = { "stick", 0.5,1.5,	1,1, 105, {0,32 / 2} };
	
	human.resting_pose = { {"head", h}, {"right_arm",ra}, {"right_leg",rl}, {"torso",t}, {"left_arm",la}, {"left_leg",ll} };

	
	//walk_sideways1
	 h = { "head", 0,0, 1,1, 0, {-1,0} };
	 ra = { "stick", 0.5,0.8,	1,1, 45, {0,32 / 2} };
	 rl = { "stick", 0.5,1.5,	1,1, 55, {0,32 / 2} };
	 t = { "stick", 0,1, 1,1, 90, {-1,0} };
	 la = { "stick", 0.5,0.8,	1,1, 135, {0,32 / 2} };
	 ll = { "stick", 0.5,1.5,	1,1, 125, {0,32 / 2} };
	

	human.pose_walk_sideways1 = { {"head", h}, {"right_arm",ra}, {"right_leg",rl}, {"torso",t}, {"left_arm",la}, {"left_leg",ll} };

	
	//walk_sideways2
	 h = { "head", 0,0, 1,1, 0, {-1,0} };
	 ra = { "stick", 0.5,0.8,	1,1, 135, {0,32 / 2} };
	 rl = { "stick", 0.5,1.5,	1,1, 125, {0,32 / 2} };
	 t = { "stick", 0,1, 1,1, 90, {-1,0} };
	 la = { "stick", 0.5,0.8,	1,1, 45, {0,32 / 2} };
	 ll = { "stick", 0.5,1.5,	1,1, 55, {0,32 / 2} };
	
	human.pose_walk_sideways2 = { {"head", h}, {"right_arm",ra}, {"right_leg",rl}, {"torso",t}, {"left_arm",la}, {"left_leg",ll} };


	models.push_back(human);
}