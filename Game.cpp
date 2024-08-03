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
		
		//Initialize PNG loading		unsure if this and ttf init are necessary or if SDL_INIT_EVERYTHING already handles it
		int imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags))
		{
			printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		}
		//Initialize SDL_ttf
		if (TTF_Init() == -1)
		{
			printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
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
int hours_in_day = 24*4;

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
	{"pics/human.png", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/improved images/16x16 size/human/human_standing.png"))},
	{"human_walk_right", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/improved images/16x16 size/human/human_walking_right.png"))},
	{"human_walk_left", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/improved images/16x16 size/human/human_walking_left.png"))},
	{"human_walk_up", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/improved images/16x16 size/human/human_walking_up.png"))},
	{"human_walk_down", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/improved images/16x16 size/human/human_walking_down.png"))},



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
	{"head", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/head.png"))},
	{"human_female", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_female.png"))},
	{"fire", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/fire.png"))},
	{"rain", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/rain1.png"))},
	{"tracks", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/tracks.png"))},
	{"playing_trumpet", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_playing_trumpet.png"))},
	{"bathing", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_bathing.png"))},
	{"fighting", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/human/human_fighting.png"))},
	{"darkness", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/darkness_filter.png"))},


	{"a", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/text/a.png"))},
	{"b", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/text/b.png"))},
	{"c", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/text/c.png"))},
	{"blacksq", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/blacksq.png"))},

	{"menu_color", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/menu_background.png"))},
	{"player", SDL_CreateTextureFromSurface(renderer, IMG_Load("pics/player/player.png"))},
	};


	//create_human();
}

int menu_num = 0;
bool pause_game = false;
//Important: does handleEvents interrupt other update functions or does it execute between update functions? If it interrupts then it will cause problems given it shares variables with other functions.
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
		case SDLK_p:
			pause_game = !pause_game;
			break;
		case SDLK_n:
			menu_num++;
			menu_num %= 6;//6 menu screens
			break;
		case SDLK_c:
			player.toggle_set_and_remove_camp_pc();
			break;
		case SDLK_s:
			player.toggle_speed_pc();
			break;
		case SDLK_z:
			player.pick_up_item_pc();
			break;
		case SDLK_r:
			player.sleep_pc();
			break;
		case SDLK_b:
			player.bathe();
			break;
		case SDLK_t:
			player.play_trumpet();
			break;
		case SDLK_q:
			player.drink_pc();
			break;
		case SDLK_l:
			player.valence = !player.valence;
			break;
		case SDLK_f:
			player.fight_mode = !player.fight_mode;
			break;
		default:
			break;
		}
		break;
		
	case SDL_MOUSEMOTION:
		SDL_GetMouseState(&mouse_x, &mouse_y);//Get mouse position
		break;
	case SDL_MOUSEBUTTONDOWN:
		mousedown_left = (event.button.button == SDL_BUTTON_LEFT);
		mousedown_right = (event.button.button == SDL_BUTTON_RIGHT);
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
	

	
	if (!pause_game) {
		hour_count++;
		if (hour_count == hours_in_day) {
			hour_count = 0;
			day_count++;
		}

		Environment::update(hours_in_day, hour_count, day_count);
		peep.update_all(day_count, hour_count, hours_in_day);
		anim.update_all(day_count, hour_count, hours_in_day);
		player.update();
	}
	
}






const SDL_Point *c;
void Game::textureManager(string texture, SDL_Rect destRect, int angle, SDL_Point center) {
	if (center.x == -1) {
		c = NULL;
	}
	else{
		c = &center;
	}
	
	SDL_RenderCopyEx(renderer, texture_map[texture], NULL, &destRect, angle, c, SDL_FLIP_NONE);
	//(if the parameter for center (c) is NULL, rotation will be done around dstrect.w / 2, dstrect.h / 2).
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

bool Game::mouse_in_tile(int x, int y) {
	if (mouse_x > x * sqdim && mouse_x<(x + 1) * sqdim && mouse_y>(y + 1) * sqdim && mouse_y < (y + 2) * sqdim) {//y is offset due to Sky row
		return true;
	}
	return false;
}

bool Game::mouse_in_rect(SDL_Rect posR) {
	if (mouse_x > posR.x && mouse_x<posR.x+posR.w && mouse_y>posR.y && mouse_y < posR.y+posR.h) {
		return true;
	}
	return false;
}

void Game::render() {
	SDL_RenderClear(renderer);
	//resets destR for printing environment
	destR.x = 0;
	destR.y = 0;
	destR.w = sqdim;
	destR.h = sqdim;
	//animation_testing();


	
	SDL_Rect mouseR;
	mouseR.x = -1;
	string item_name_moused;
	
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

			if (!pause_game && mousedown_left && mouse_in_tile(x,y)) {
				player.move_to_pc({ x,y });
				mousedown_left = false;
				cout << x;
			}

			bool no_item = false;
			int item_id = Environment::Map[y][x].item_id;
			if (item_id == -1) {
				no_item = true;
			}
			if (!no_item) {
				ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(item_id)];
				textureManager(item.image, destR);
				if (!pause_game && mouse_in_tile(x,y)) {
					mouseR = destR;
					mouseR.y = y * sqdim;
					item_name_moused = item.item_name;
				}
			}
			else {
				if (Environment::Map[y][x].terrain == "dirt") {
					textureManager("pics/dirt.png", destR);
				}
				else if (Environment::Map[y][x].terrain == "water") {
					textureManager("pics/shallow_water.png", destR);
				}
			}
			if (Environment::Map[y][x].track.track_age != -1) {
				textureManager("tracks", destR);
			}
			if (Environment::Map[y][x].has_fire) {//fire should be last thing drawn, needs to be moved to the end of render() FIX THIS
				textureManager("fire", destR);
			}
			if (Environment::Map[y][x].has_rain) {//same issue regarding draw order as fire, fix this
				textureManager("rain", destR);
			}
			//this (darkness) doesn't seem to work, fix
			if (Environment::Map[y][x].light_level <= 3) {//this would be better as a for loop, fix
				textureManager("darkness", destR);
				if (Environment::Map[y][x].light_level <= 2) {
					textureManager("darkness", destR);
					if (Environment::Map[y][x].light_level <= 1) {
						textureManager("darkness", destR);
						if (Environment::Map[y][x].light_level == 0) {
							textureManager("darkness", destR);
						}
					}
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

		if (mousedown_right && mouse_in_rect(destR)) {
			if (player.fight_mode) {
				player.attack_person(People::pl[i].id);
			}
			else {
				player.chat_pc(People::pl[i].id);
			}
			
			mousedown_right = false;
		}

		//temp adjustment, should be kept inside People class
		if (People::pl[i].current_image == "pics/human.png" && People::pl[i].sex==false) {
			People::pl[i].current_image = "human_female";
		}

		textureManager(People::pl[i].current_image, destR);

		if (People::pl[i].current_image == "human_female") {
			People::pl[i].current_image = "pics/human.png";
		}
	}
	if (mouseR.x != -1) {
		textManager(item_name_moused, 8, mouseR.x, mouseR.y);
	}
	
	if (pause_game) {
		destR.x = 10 * sqdim;
		destR.y = 10 * sqdim;
		destR.w = sqdim * 80;
		destR.h = sqdim * 30;
		textureManager("menu_color", destR);

		if (menu_num == 0) {
			vector<string> stats = player.view_own_data();//should menus be moved inside of Player class?
			int x = 16;
			int y = 12;
			textManager("Player Stats: ", 24, x * sqdim, y * sqdim);
			y++;//because Player Stats is larger and so needs more vertical room
			for (string s : stats) {
				y++;
				textManager(s, 12, x * sqdim, y * sqdim);
			}
		}
		else if (menu_num == 1) {
			vector<string> inventory = player.view_inventory();//should menus be moved inside of Player class?
			int x = 16;
			int y = 12;
			textManager("Player Inventory: ", 24, x* sqdim, y* sqdim);
			y++;
			for (int inv_i = 0; inv_i < inventory.size(); inv_i++) {
				y++;
				SDL_Rect text_box = textManager(inventory[inv_i], 12, x* sqdim, y* sqdim);
				if (mousedown_left && mouse_in_rect(text_box)) {//y-1 becuase it is screen position and so must ignore Sky offset
					player.drop_item_pc(inv_i);//pass index of item
					mousedown_left = false;
					//should mousedown_right also be reset to false just in case?
				}
				else if (mousedown_right && mouse_in_rect(text_box)) {
					player.equip_pc(inv_i);//if item is food, it is eaten (for now) instead of equipped, only clothing/tools can be equipped for now
					mousedown_right = false;
				}
			}
		}
		else if (menu_num == 2) {
			vector<string> equipment = player.view_equipment();//should menus be moved inside of Player class?
			int x = 16;
			int y = 12;
			textManager("Player Equipment: ", 24, x * sqdim, y * sqdim);
			y++;
			for (int inv_i = 0; inv_i < equipment.size(); inv_i++) {
				y++;
				SDL_Rect text_box = textManager(equipment[inv_i], 12, x * sqdim, y * sqdim);
				if (mousedown_left && mouse_in_rect(text_box)) {//y-1 becuase it is screen position and so must ignore Sky offset
					player.unequip_pc(inv_i);//pass index of item in equipped (have to iterate through to index given equipment is a map<>
					mousedown_left = false;
					//should mousedown_right also be reset to false just in case?
				}
			}
		}
		else if (menu_num == 3) {
			vector<string> dispositions = player.view_dispositions();//should menus be moved inside of Player class?
			int x = 16;
			int y = 12;
			textManager("Player Dispositions: ", 24, x * sqdim, y * sqdim);
			y++;
			for (string s : dispositions) {
				y++;
				textManager(s, 12, x * sqdim, y * sqdim);
			}
		}
		else if (menu_num == 4) {
			vector<string> craftable = player.view_craftable();//should menus be moved inside of Player class?
			int x = 16;
			int y = 12;
			textManager("Craftable Items: ", 24, x* sqdim, y* sqdim);
			y++;
			for (int inv_i = 0; inv_i < craftable.size(); inv_i++) {
				y++;
				SDL_Rect text_box = textManager(craftable[inv_i], 12, x * sqdim, y * sqdim);
				if (mousedown_left && mouse_in_rect(text_box)) {//y-1 becuase it is screen position and so must ignore Sky offset
					player.craft_pc(craftable[inv_i]);//pass name of item
					mousedown_left = false;
				}
			}
		}
		else if (menu_num == 5) {
			int x = 16;
			int y = 12;
			textManager("Menu 6: ", 24, x* sqdim, y* sqdim);
		}
	}
	


	/* For when I want to use my own custom font
	string test = "abccba";

	destR.x = 50 * sqdim;
	destR.y = 25 * sqdim;
	for (auto &c : test) {
		string s(1,c);
		textureManager(s, destR);
		destR.x/=sqdim;
		destR.x++;
		destR.x *= sqdim;
	}
	*/

	SDL_RenderPresent(renderer);
}

//fix this: need to cache text constants such as menu titles, labels, etc and only update variable text such as numbers and names, etc.
SDL_Rect Game::textManager(string text, int size, int x, int y) {
	SDL_Rect tempR = destR;
	TTF_Font* font = TTF_OpenFont("pics/text/LEMONMILK-Bold.otf", size);
	SDL_Color color = { 0, 0, 0 };//black
	const char* text1 = text.c_str();
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text1, color);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	destR.w = text_surface->w;
	destR.h = text_surface->h;
	SDL_FreeSurface(text_surface);//to prevent memory leak?
	destR.x = x;
	destR.y = y;
	SDL_RenderCopy(renderer, text_texture, NULL, &destR);
	SDL_Rect returnR = destR;
	destR = tempR;
	return returnR;
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

/*
void Game::create_human() {//animation should be moved to its own class, fix this
	// for testing new poses
	bone h = { "head", 0,0, 1,1, 0, {-1,0} };
	bone ra = { "stick", 0.5,0.8,	1,1, 35, {0,32 / 2} };
	bone rl = { "stick", 0.5,1.5,	1,1, 55, {0,32 / 2} };
	bone t = { "stick", 0,1, 1,1, 90, {-1,0} };
	bone la = { "stick", 0.5,0.8,	1,1, 145, {0,32 / 2} };
	bone ll = { "stick", 0.5,1.5,	1,1, 125, {0,32 / 2} };
	


	skeleton human = { 32, {15,5} };//scale and center point x,y
	//NEED TO FIX the model/animation breaks at any scale other than 32
	

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
*/

int Game::scale=100;

Game::bone Game::join(bone b1, bone b2) {
	double t = (M_PI / 180) * b1.r;
	int xp = cos(t) - b1.length * sin(t);
	int yp = sin(t) + b1.length * cos(t);
	xp += b1.x;
	yp += b1.y;
	bone ret = { b2.image,b2.length,b1.r+b2.r,xp,yp };
	return ret;
}

//float offset = 0.5;
int rd = 1;
int ld = 1;
bool flip = false;

int r2 = 0;
int anchor_x = 40 * sqdim;
int anchor_y = 15 * sqdim;
void Game::animation_testing() {

	//	2nd attempt at 2d rigging. Closer but it breaks below scale=50, which is a total height of 150
	scale = 16*4;

	SDL_Rect head;
	bone head_bone = { "blacksq",scale,r2,anchor_x,anchor_y};
	head.w = scale;
	head.h = scale;
	head.x = head_bone.x - (head.w / 2);//attach head to b1, centered
	head.y = head_bone.y;
	
	bone torso_bone = { "blacksq",scale,0};
	torso_bone = join(head_bone, torso_bone);

	bone right_arm_bone = { "blacksq",scale,0};
	right_arm_bone = join(head_bone, right_arm_bone);
	right_arm_bone.r -=45;

	bone left_arm_bone = { "blacksq",scale,0 };
	left_arm_bone = join(head_bone, left_arm_bone);
	left_arm_bone.r += 45;

	bone right_leg_bone = { "blacksq",scale,0 };
	right_leg_bone = join(torso_bone, right_leg_bone);
	right_leg_bone.r -= 45;

	bone left_leg_bone = { "blacksq",scale,0 };
	left_leg_bone = join(torso_bone, left_leg_bone);
	left_leg_bone.r += 45;
	
	skeleton2 skelly = { {head_bone,torso_bone,right_arm_bone,left_arm_bone,right_leg_bone,left_leg_bone} };
	skelly.render_skeleton();
	//r2+=5;
	textureManager("head", head);

	







	//models[0].render_skeleton();

	//if (flip) {
	//	if (models[0].pose_transform(models[0].pose_walk_sideways2, 2)) {
	//		flip = !flip;
	//	}
	//}
	//else {
	//	if (models[0].pose_transform(models[0].pose_walk_sideways1, 2)) {
	//		flip = !flip;
	//	}
	//}

	//human.current_pose["right_arm"].r += 6;
	//human.current_pose["left_arm"].r -= 6;
	//human.current_pose["right_leg"].r += 6;
	//human.current_pose["left_leg"].r -= 6;


	//human.current_pose["right_arm"].x -= 0.1;
	//if (human.current_pose["right_arm"].x < human.resting_pose["right_arm"].x - 2*32) {
	//	human.current_pose["right_arm"].x = human.resting_pose["right_arm"].x;
	//}
}