#ifndef Game_hpp
#define Game_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <map>
#include <cmath>

#include "People.hpp"
#include "Environment.hpp"
#include "ItemSys.hpp"
#include "Animal.hpp"
#include "Player.hpp"
#include "Plants.hpp"
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

	static void textureManager(string texture, SDL_Rect destRect, int angle, SDL_Point center);
	static void textureManager(string texture, SDL_Rect destRect);
	static SDL_Rect textManager(string text, int size, int x, int y);//returns rectangle of text that was printed

	bool isRunning;
	SDL_Window* window; //window and renderer have to(?) be pointers because they aren't instanced until the Game::init function runs.
	static SDL_Renderer* renderer; //should this be made static and public?

	TTF_Font* gFont = NULL;
	SDL_Texture* mTexture;

	void load_images_from_csv();

	void render_sky();
	void render_map(SDL_Rect mouseR, string item_name_moused, int min_x, int max_x, int min_y, int max_y, bool keep_player_centered_x, bool keep_player_centered_y);
	void render_entities(int min_x, int max_x, int min_y, int max_y, bool keep_player_centered_x, bool keep_player_centered_y);
	void render_menus();

	int mouse_x=-1;
	int mouse_y=-1;
	bool mousedown_left = false;
	bool mousedown_right = false;

	int zoom_level = 0;

	bool mouse_in_tile(int x, int y);
	bool mouse_in_rect(SDL_Rect posR);

	//animation - move to another class
	void animation_testing();
	

	static int scale;
	struct bone {
		string image;
		int length;//this can lengthen or shorten as needed.
		int r=0;//rotation angle
		int x=0;
		int y=0;

		//keep these constant
		int w = scale/10;//keep this constant for creating rig
		SDL_Point c= { 10 / 2,0 } ;//center. Keep this constant as well

		void render_bone() {
			if (w < 1) { w = 1; }
			SDL_Rect rect = {this->x,this->y,this->w,this->length};//length==height
			textureManager(this->image, rect, this->r, this->c);
		}
	};

	bone join(bone b1, bone b2);//returns b2 attached to end of b1

	struct skeleton2 {
		vector<bone> bones;

		void render_skeleton() {
			for (auto b : bones) {
				b.render_bone();
			}
		}
	};




	struct skeleton {//a better but more complex method would be to set and render points with distance and rotation constraints, then adding lines and images from one point to another. Do much later.
		int scale;
		SDL_Point center;//every x and y in every bone is the offset from this point

		//fix this, make these poses a map<>	the center of rotation, offset, etc are generally fixed with only the rotations being different between poses, so it might make more sense to save poses as a map of <bone name, rotation> instead of a whole bone
		map<string, bone> resting_pose;//this is the part of the skeleton initialization, it's the default pose
		map<string, bone> pose_walk_sideways1;
		map<string, bone> pose_walk_sideways2;//can right and left be turned into a bool to flip image horizontally?
		map<string, bone> pose_walk_up;
		map<string, bone> pose_walk_down;//need to implement diagonal poses
		//idle pose (just slight movement of arms, etc)
		//sleeping pose
		//shooting pose
		//bow draw pose
		//punch pose
		//slash pose
		//thrust pose
		//pick up pose
		//craft pose
		string destination_pose;


		map<string, bone> current_pose;//this gets rendered. only this gets adjusted
		bool adjusted = false;
		bool pose_transform(map<string,bone> dest_pose, int speed) {//returns true when done	one problem with this is which direction does rotation move in? clockwise or counterclockwise? Should it take the shortest path always?
			//move current pose to destination pose
			bool done = true;
			for (auto const& b : current_pose) {
				if (current_pose[b.first].r < dest_pose[b.first].r) {
					current_pose[b.first].r+=speed;
					done = false;
				}
				else if (current_pose[b.first].r > dest_pose[b.first].r) {
					current_pose[b.first].r-=speed;
					done = false;
				}
			}
			return done;
		}
		void render_skeleton() {
			adjust_pos_by_scale();
			for (auto const& b : current_pose) {
				current_pose[b.first].render_bone();

			}
		}
		void adjust_pos_by_scale() {
			if (this->adjusted) { return; }//if any x or y change, then must readjust
			//go through each bone and multiply x and y by scale
			current_pose = resting_pose;
			for (auto const& b : current_pose) {
				current_pose[b.first].x += this->center.x;
				current_pose[b.first].y += this->center.y;
				current_pose[b.first].x *= this->scale;
				current_pose[b.first].y *= this->scale;
				current_pose[b.first].length *= this->scale;
				current_pose[b.first].w *= this->scale;

			}
			this->adjusted = true;
		}
	};

	void create_human();

	vector<skeleton> models;//later make this a map<>, don't know why it's (using map<>) not working right now

};

#endif
