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

map<string, SDL_Texture*> texture_map;
void Game::load_images_from_csv() {
	//read from csv into a vector of structs
	fstream fin;// File pointer 
	fin.open("Images.csv", ios::in);// Open an existing file 
	vector<string> row;// Read the Data from the file as String Vector 
	string line, word;
	bool firstrowdone = false;
	bool start_count_tags = false;//for counting the max number of tag columns
	int count_tags = 0;
	bool start_count_ingredients = false;//for counting the max number of ingredient columns
	int count_ingredients = 0;
	while (getline(fin, line)) {// read an entire row and store it in a string variable 'line' 
		row.clear();
		stringstream s(line);// used for breaking words 
		while (getline(s, word, ',')) {// read every column data of a row and store it in a string variable, 'word' 
			row.push_back(word);// add all the column data of a row to a vector 
		}
		texture_map.insert({row[0], SDL_CreateTextureFromSurface(renderer, IMG_Load(row[1].c_str()))});//insert name as key and convert filename into SDL texture
	}
	
}

People peep;
Animal anim;
Player player;
//Environment envir;
SDL_Rect srcR, destR;

int day_count = 0; //temporary measure for counting days passed
int hour_count = 0; //temporary measure for tracking when a new day starts, 20 updates == 1 day
int hours_in_day = 24*4;

int sqdim = 16;

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

	load_images_from_csv();

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
		case SDLK_m:
			zoom_level++;
			zoom_level %= 3;//3 zoom levels currently
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
	People::TILE_PIXEL_SIZE = sqdim;//affects npc speed given that movement between tiles is on a per pixel basis
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
	if (zoom_level == 0) {
		sqdim = 16;
	}
	else if (zoom_level == 1) {
		sqdim = 32;
	}
	else if (zoom_level == 2) {
		sqdim = 64;
	}

	//peep.TILE_PIXEL_SIZE = sqdim;//this doesn't seem to be working. I don't know why changing this variable causes npc's and player to go off map

	SDL_RenderClear(renderer);
	//resets destR for printing environment
	destR.x = 0;
	destR.y = 0;
	destR.w = sqdim;
	destR.h = sqdim;
	//animation_testing();

	//Need to break up render() into more smaller functions or simplify

	//game rendering
	
	SDL_Rect mouseR = {-1,-1,-1,-1};
	string item_name_moused;

	int screen_width;
	int screen_height;
	SDL_GetRendererOutputSize(renderer, &screen_width, &screen_height);
	int max_x_screen_tiles = screen_width / sqdim;
	int max_y_screen_tiles = screen_height / sqdim;

	int center_x;
	int center_y;
	if (player.pcindex == -1) {
		center_x = Environment::map_x_max/2;
		center_y = Environment::map_y_max/2;
	}
	else {
		if (People::Position::distance({0, player.pl[player.pcindex].pos.y }, { 0,Environment::map_y_max - 1 }) < (max_y_screen_tiles / 2)) {
			center_y = Environment::map_y_max - (max_y_screen_tiles / 2);
		}
		else if (player.pl[player.pcindex].pos.y < (max_y_screen_tiles / 2)) {
			center_y = max_y_screen_tiles / 2;
		}
		else {
			center_y = player.pl[player.pcindex].pos.y;
		}
		if (People::Position::distance({ player.pl[player.pcindex].pos.x,0 }, { Environment::map_x_max - 1,0 }) < (max_x_screen_tiles / 2)) {
			center_x = Environment::map_x_max - (max_x_screen_tiles / 2);
		}
		else if (player.pl[player.pcindex].pos.x < (max_x_screen_tiles / 2)) {
			center_x = max_x_screen_tiles / 2;
		}
		else {
			center_x = player.pl[player.pcindex].pos.x;
		}
	}

	int max_x = center_x + (max_x_screen_tiles / 2);
	int min_x = center_x - (max_x_screen_tiles / 2);
	int max_y = center_y + (max_y_screen_tiles / 2);
	int min_y = center_y - (max_y_screen_tiles / 2);
	
	if (max_y > Environment::map_y_max) { max_y = Environment::map_y_max; }
	if (max_x > Environment::map_x_max) { max_x = Environment::map_x_max; }
	if (min_y < 0) { min_y = 0; }
	if (min_x < 0) { min_x = 0; }

	bool keep_player_centered_x = min_x != 0 && max_x != Environment::map_x_max;
	bool keep_player_centered_y = min_y != 0 && max_y != Environment::map_y_max;
	

	render_map(mouseR, item_name_moused, min_x, max_x, min_y, max_y, keep_player_centered_x, keep_player_centered_y);
	
	render_entities(min_x, max_x, min_y, max_y, keep_player_centered_x, keep_player_centered_y);


	if (mouseR.x != -1) {
		textManager(item_name_moused, 8, mouseR.x, mouseR.y);
	}
	
	render_menus();
	
	
	//frame around the map, currently used to hide issue I need to fix regarding map rendering and camera movement at the map edges
	SDL_Rect frame = { 0,sqdim,screen_width,sqdim / 2 };
	textureManager("blacksq", frame);
	frame.y = screen_height - (sqdim / 2);
	textureManager("blacksq", frame);
	frame = { 0,sqdim,sqdim / 2,screen_height };
	textureManager("blacksq", frame);
	frame.x = screen_width - (sqdim / 2);
	textureManager("blacksq", frame);

	//sky is rendered last for the same reason as the black frame
	render_sky();

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


void Game::render_sky(){
	for (int x = 0; x < hours_in_day / 2; x++) {//need to adjust according to screen zoom such that the whole sky is always visible
		destR.x = x * sqdim;
		destR.y = 0;
		if (Environment::Sky[x].has_sun) {
			textureManager("pics/sun.png", destR);
		}
		else if (Environment::Sky[x].has_moon) {
			textureManager("pics/moon.png", destR);
		}
		else {
			if (hour_count < hours_in_day / 2) {
				textureManager("pics/sky_day.png", destR);
			}
			else {
				textureManager("pics/sky_night.png", destR);
			}
		}
	}
}
void Game::render_map(SDL_Rect mouseR, string item_name_moused, int min_x, int max_x, int min_y, int max_y, bool keep_player_centered_x, bool keep_player_centered_y){
	for (int y = min_y, sy = 0; y < max_y; y++, sy++) {
		for (int x = min_x, sx = 0; x < max_x; x++, sx++) {

			if (keep_player_centered_x) {
				destR.x = (sx)*sqdim - player.pl[player.pcindex].px_x;
			}
			else {
				destR.x = (sx)*sqdim;
			}
			if (keep_player_centered_y) {
				destR.y = (sy + 1) * sqdim - player.pl[player.pcindex].px_y;
			}
			else {
				destR.y = (sy + 1) * sqdim;
			}



			if (!pause_game && mousedown_left && mouse_in_rect(destR)) {
				player.move_to_pc({ x,y });
				mousedown_left = false;
			}

			bool no_item = false;
			int item_id = Environment::Map[y][x].item_id;
			if (item_id == -1) {
				no_item = true;
			}
			if (!no_item) {
				ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(item_id)];
				textureManager(item.image, destR);
				if (!pause_game && mouse_in_rect(destR)) {
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


			//end Map rendering
		}
	}
}
void Game::render_entities(int min_x, int max_x, int min_y, int max_y, bool keep_player_centered_x, bool keep_player_centered_y){
	for (int y = min_y, sy = 0; y < max_y; y++, sy++) {
		for (int x = min_x, sx = 0; x < max_x; x++, sx++) {

			if (keep_player_centered_x) {
				destR.x = (sx)*sqdim - player.pl[player.pcindex].px_x;
			}
			else {
				destR.x = (sx)*sqdim;
			}
			if (keep_player_centered_y) {
				destR.y = (sy + 1) * sqdim - player.pl[player.pcindex].px_y;
			}
			else {
				destR.y = (sy + 1) * sqdim;
			}

			//people and animals need to be drawn after whole map is drawn given that people and later animals move between tiles, and if drawn with tiles then that means other tiles will be drawn over the people/animals
			if (Environment::Map[y][x].animal_id != -1) {
				Animal::animal& a = Animal::al[anim.a_by_id(Environment::Map[y][x].animal_id)];
				textureManager(a.current_image, destR);
			}
			if (Environment::Map[y][x].person_id > -1) {
				People::Person& p = People::pl[peep.p_by_id(Environment::Map[y][x].person_id)];

				if (player.pl[player.pcindex].id != p.id) {
					destR.x += p.px_x;
					destR.y += p.px_y;
				}
				else {
					if (!keep_player_centered_x) {
						destR.x += p.px_x;
					}
					else {
						destR.x = (sx)*sqdim;
					}
					if (!keep_player_centered_y) {
						destR.y += p.px_y;
					}
					else {
						destR.y = (sy + 1) * sqdim;
					}
				}

				if (mousedown_right && mouse_in_rect(destR)) {
					if (player.fight_mode) {
						player.attack_person(p.id);
					}
					else {
						player.chat_pc(p.id);
					}

					mousedown_right = false;
				}

				//temp adjustment, should be kept inside People class
				if (p.current_image == "pics/human.png" && p.sex == false) {
					p.current_image = "human_female";
				}

				textureManager(p.current_image, destR);

				if (p.current_image == "human_female") {
					p.current_image = "pics/human.png";
				}
			}
		}
	}
}
void Game::render_menus(){
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
			textManager("Player Inventory: ", 24, x * sqdim, y * sqdim);
			y++;
			for (int inv_i = 0; inv_i < inventory.size(); inv_i++) {
				y++;
				SDL_Rect text_box = textManager(inventory[inv_i], 12, x * sqdim, y * sqdim);
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
			textManager("Craftable Items: ", 24, x * sqdim, y * sqdim);
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
			textManager("Menu 6: ", 24, x * sqdim, y * sqdim);
		}
	}
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
	scale = 32;

	SDL_Rect scale_sq = { 0,0,32,32 };
	textureManager("blacksq", scale_sq);

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


