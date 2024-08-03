#ifndef Creature_hpp
#define Creature_hpp
#define SDL_MAIN_HANDLED //this is included before the SDL.h include to resolve a LNK2019 error
#include  "SDL.h"
#include "SDL_image.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "Environment.hpp"
#include "ItemSys.hpp"
#include <stdexcept>
using namespace std;
using namespace proj_util;
//This class holds shared data structures and functions for People and Animal who both inherit from this class
class Creature {
public:

};

#endif