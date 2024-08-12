#ifndef ProjUtil_hpp
#define ProjUtil_hpp
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdexcept>
using namespace std;


//FIX THIS: need to go through project and replace separate x_pos and y_pos with Position
//Project Utility structs and functions
namespace proj_util {

	//the use of bint might have caused bloat by holding a copy of max and min per instance of plant for each attribute rather than checking against species preset, unsure if turning min and max into pointers would help or not
	struct bint {//bounded int, never increases beyond max or decreases below min, has functions to check if equal to max or min
		int n;
		int min;//might be able to keep bint but reduce bloat by keeping min and max as a pair in a global set of {min,max} so as to refer to those by index for this bint
		int max;
		//need to overload operators and add bool ismax() and ismin(). A simple way to clamp n is to use bitmask with Max and Min
		void clamp() {
			//OR the first bit to preserve the int's sign, AND the other bits with x00 upto the bit that determines max, unsure how to approach the following bits to enforce both max and min
			//Use naive approach for now
			(n > max) ? n = max : (n < min) ? n = min : n = n;
		}
		bool operator==(const bint& rhs) {
			return n == rhs.n;
		}
		bool operator!=(const bint& rhs) {
			return n != rhs.n;
		}
		bint& operator+=(const int& rhs) {//need bint+=bint and bint+=int? or can implicit conversion of bint to int suffice?
			n += rhs;
			clamp();
			return *this;
		}
		bint& operator-=(const int& rhs) {
			n -= rhs;
			clamp();
			return *this;
		}
		bint& operator++() {
			n++;
			clamp();
			return *this;
		}
		bint& operator--() {
			n--;
			clamp();
			return *this;
		}
		bint operator++(int) {//for this type, have postfix and prefix be the same
			n++;
			clamp();
			return *this;
		}
		bint operator--(int) {
			n--;
			clamp();
			return *this;
		}
		bint& operator*=(const int& rhs) {
			n *= rhs;
			clamp();
			return *this;
		}
		bint& operator/=(const int& rhs) {
			n /= rhs;
			clamp();
			return *this;
		}
		bint& operator=(const int& rhs) {
			n = rhs;
			clamp();
			return *this;
		}
		bool ismax() {
			return n == max;
		}
		bool ismin() {
			return n == min;
		}
		operator int() {//implicit conversion of bint to int
			return n;
		}
	};


	static int ox;//origin.x for use in distance(). Uses current Person/Animal/etc position
	static int oy;
	static int pu_map_x_max;//need to set in Environment class
	static int pu_map_y_max;
	struct Position {
		int x = -1;//was going to use bint to automatically keep these in range, but that would cause problems with getting a getting a position relative to another, such as adding {-1,-1} to a position to get a new position 1 tile SW of the original
		int y = -1;
		//bint x = {0,0,map_x_max-1};//automatically keeps position within Map bounds. Makes valid_position redundant? Or can that function have other use cases?
		//bint y = {0,0,map_y_max-1};
		//bool init = false;//is used in place of pos.x==-1 to determine if a Position is NULL or not
		bool operator==(Position const& pos2) {
			return this->x == pos2.x && this->y == pos2.y;
		}
		bool operator!=(Position const& pos2) {
			return this->x != pos2.x || this->y != pos2.y;
		}
		static int distance(Position pos1, Position pos2) {
			int xd = abs(pos1.x - pos2.x);
			int yd = abs(pos1.y - pos2.y);
			int max = -1;
			(xd > yd) ? max = xd : max = yd;
			return max;
		}
		
		bool operator<(Position const& pos2) const {//sorting order is according to distance from an origin, the origin being the person's current position
			int d1 = distance(*this, { ox,oy });
			int d2 = distance(pos2, { ox,oy });
			if (d1 < d2) {
				return true;
			}
			else if (d1 == d2) {
				if (this->x < pos2.x) { return true; }
				if (this->x == pos2.x && this->y < pos2.y) { return true; }
			}
			return false;
		}
		//should this include an overload of ! operator to check if pos == {-1,-1} which is the NULL equivalent?
		static bool valid_position(Position pos) {//according to world map
			bool valid_x = 0 <= pos.x && pos.x < pu_map_x_max;
			bool valid_y = 0 <= pos.y && pos.y < pu_map_y_max;
			if (valid_x && valid_y) {
				return true;
			}
			return false;
		}
		static Position make_position_valid(Position dest, int ux, int lx, int uy, int ly) {//bounds: u==upper, l==lower
			if (dest.x < lx) {
				dest.x = lx;
			}
			else if (dest.x >= ux) {
				dest.x = ux - 1;
			}
			if (dest.y < ly) {
				dest.y = 0;
			}
			else if (dest.y >= uy) {
				dest.y = uy - 1;
			}
			return dest;
		}

	};

	inline vector<int> remove_dup(vector<int> v) {//to remove duplicates from vector but preserve original order
		vector<int> v2;
		bool dupe = false;
		for (int i : v) {
			for (int j : v2) {
				if (i == j) {
					dupe = true;
					break;
				}
			}
			if (!dupe) {
				v2.push_back(i);
			}
			dupe = false;
		}
		return v2;
	}

	inline vector<vector<string>> get_data(string file_name) {
		fstream fin;// File pointer 
		fin.open(file_name, ios::in);// Open an existing file 
		vector<vector<string>> data;
		vector<string> row;// Read the Data from the file as String Vector 
		string line, word;
		while (getline(fin, line)) {// read an entire row and store it in a string variable 'line' 
			row.clear();
			stringstream s(line);// used for breaking words 
			while (getline(s, word, ',')) {// read every column data of a row and store it in a string variable, 'word' 
				row.push_back(word);// add all the column data of a row to a vector 
			}
			data.push_back(row);
		}
		return data;
	}



}
#endif