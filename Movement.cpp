#include "People.hpp"
#include "Animal.hpp"
using namespace std;
//---Any functions that regard movement:---


//Note: one part of a solution to a destination tile being occupied is to request access to the tile if the person/animal on it is idle, so that they move out of the way
//another part of the solution is either to find a different destination tile that satisfies the distance to target (ex: another adjacent tile to target) or choose another target (if the desired water tile has no empty adjacent tiles, search for another water tile) (may cause problem if next closest water tile is too far and the better course of action was to wait your turn)

//FIX: for some reason, people are going into occupied tiles under some specific condition and that removes the other person from the map and therefore also their image from the map
//need to fix any relevant function that calls move_to() such as general_search/random_walk so as to avoid going near disliked people and avoid even more and at greater distance any hated people.
//  otherwise everyone bunches up together because everyone likes someone and so they are only pull factors and no push factors in where people move_to. The only push factor right now that sort of works is campsite placement near friends.
//need to fix so that diagononal movement is the same speed as non-diagonal movement
bool Animal::move_to(Position dest, string caller) {//need to include some check so that someone moving fast can't pass through obstacles by skipping over a tile
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    bool reached = false;

    Position old_pos;
    Position new_pos;
    if (!c.mov) {
        if (c.general_search_called && caller != "general searching") {
            c.general_search_called = false;
        }

        if (c.downed || c.immobile || c.move_already || c.age < sp.MAX_INFANT_AGE) {//the image check shouldn't be necessary but I don't know why it still moves while having crafting image
            if (c.pos == dest) {
                return true;
            }
            return false;
        }
        old_pos = c.pos;
        new_pos = c.pos;

        int sign = (new_pos.x > dest.x) - (new_pos.x < dest.x);//get -1/0/+1 according to whether left_var is greater/less/equal to right_var
        new_pos.x -= sign;//add inverse sign to get new_pos.x to be 1 step closer to dest.x
        sign = (new_pos.y > dest.y) - (new_pos.y < dest.y);
        new_pos.y -= sign;

        bool empty = true;
        if (c.species == "human" && Environment::Map[new_pos.y][new_pos.x].person_id != -1) {
            empty = false;
        }
        else if (Environment::Map[new_pos.y][new_pos.x].animal_id != -1) {
            empty = false;
        }

        if (!empty) {//this is !=-1 instead of >-1 because -2 is reserved marker. If a person is blocking path, move to first empty tile in a clockwise direction
            c.current_direction = { old_pos.x - new_pos.x, old_pos.y - new_pos.y };
            Position test_pos;
            bool next = false;
            for (Position pos : c.rotations) {//still just a temp fix, need actual pathfinding algo
                if (next) {
                    test_pos = { old_pos.x + pos.x, old_pos.y + pos.y };
                    if (Position::valid_position(test_pos) && test_pos != c.pos && (c.species == "human")?Environment::Map[test_pos.y][test_pos.x].person_id == -1: Environment::Map[test_pos.y][test_pos.x].animal_id == -1) {
                        new_pos = test_pos;
                        break;
                    }
                }
                if (c.current_direction == pos) {
                    if (next) {
                        new_pos = old_pos;//no free adjacent tile, don't move
                        break;
                    }
                    next = true;
                }
            }
        }

        c.move_already = true;
        c.dest = new_pos;//save the next tile destination
        (c.species == "human") ? 
            Environment::Map[new_pos.y][new_pos.x].person_id = -2 ://reserve tile so that no other person tries to enter it
            Environment::Map[new_pos.y][new_pos.x].animal_id = -2;
        c.main_dest = dest;//save long term destination
    }

    old_pos = c.pos;
    new_pos = c.dest;

    //fix this, if speed is larger than tile pixel size, then speed must also affect movement of tiles per tick but ensuring collisions in the path are still taken into account

    for (int i = 0;i < 2;i++) {//used to reduce x,y repetition of code, cleaner code
        int& po1 = (i == 0) ? old_pos.x : old_pos.y;//oldpos
        int& po2 = (i == 0) ? new_pos.x : new_pos.y;//newpos
        int& po3 = (i == 0) ? c.px_x : c.px_y;//px
        int& po4 = (i == 0) ? c.pos.x : c.pos.y;//pos

        int sign = (po1 < po2) - (po1 > po2);

        if (sign == 0 && po3 != 0) {//if tile destination reached but px position is not zeroed, move px position to 0
            sign = (po3 < 0) - (po3 > 0);
            po3 += sign * c.speed;//movement for 2nd half is inverse direction from first half because px position is offset from normal tile position
        }
        else if (sign != 0) {
            po3 += sign * c.speed;
            if (po3 == sign * TILE_PIXEL_SIZE / 2) {//tie to new tile when reached the midway point between tiles
                po3 *= -1;//invert
                po4 += sign;
                if (i == 0) {//handling x related image   Need to create methods that can recieve relevant data to decide which image to use outside of the normal npc logic. Game logic should not have to worry about images, only provide the data for another function to decide.
                    if (sign == 1) {
                        if (c.walk_flip) {//use progress_state instead of a bool, fix
                            c.current_image = c.species+"_walking_right1";
                        }
                        else {
                            c.current_image = c.species + "_walking_right2";
                        }
                    }
                    else if (sign == -1) {
                        if (c.walk_flip) {
                            c.current_image = c.species + "_walking_left1";
                        }
                        else {
                            c.current_image = c.species + "_walking_left2";
                        }
                    }
                    c.walk_flip = !c.walk_flip;
                }
            }
        }
        c.mov = true;
    }

    if (c.pos == new_pos) {
        if (c.px_x == 0 && c.px_y == 0) {
            c.mov = false;
        }
        else {
            if (c.species == "human") {
                Environment::Map[old_pos.y][old_pos.x].person_id = -1;//remove person from Map
                Environment::Map[c.pos.y][c.pos.x].person_id = People::pl[People::p].p_id;//add person back to Map at new location
            }
            else {
                Environment::Map[old_pos.y][old_pos.x].animal_id = -1;//remove animal from Map
                Environment::Map[c.pos.y][c.pos.x].animal_id = c.a_id;//add animal back to Map at new location
            }
            int sign = (c.pos.y > old_pos.y) - (c.pos.y < old_pos.y);
            int dir_y = sign;
            sign = (c.pos.x > old_pos.x) - (c.pos.x < old_pos.x);
            int dir_x = sign;
            Environment::Tracks t = { c.species, 0, dir_x, dir_y };//holds info on the direction the person went in, so someone else can follow the tracks in the right direction
            Environment::Map[old_pos.y][old_pos.x].track = t;
        }
    }
    if (!c.mov && c.pos == c.main_dest) {
        reached = true;
        c.clean_image = true;
    }
    return reached;
}

Position Animal::walk_search_random_dest() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];
    Species& sp = species[c.species];
    Position dest = { -1,-1 };
    bool valid_dest = false;
    while (!valid_dest) {//set destination by setting a direction and duration
        int max_x = Environment::map_x_max;//unsure if this is should be map_x_max or map_x_max-1 
        int min_x = 0;
        int max_y = Environment::map_y_max;
        int min_y = 0;
        if (c.campsite_pos.x != -1) {
            max_x = c.campsite_pos.x + sp.campsite_distance_search;
            min_x = c.campsite_pos.x - sp.campsite_distance_search;
            max_y = c.campsite_pos.y + sp.campsite_distance_search;
            min_y = c.campsite_pos.y - sp.campsite_distance_search;
        }
        else if (!c.sex && c.spouse_id != -1) {//keeps near spouse if searching for new campsite, currently just has female follow male, otherwise both don't go far because they pull in different directions
            Position& spouse_pos = (c.species=="human")?People::pl[People::p_by_id(c.spouse_id)].pos:al[a_by_id(c.spouse_id)].pos;
            max_x = spouse_pos.x + sp.spouse_distance_limit;
            min_x = spouse_pos.x - sp.spouse_distance_limit;
            max_y = spouse_pos.y + sp.spouse_distance_limit;
            min_y = spouse_pos.y - sp.spouse_distance_limit;
        }
        dest.x = (rand() % (max_x - min_x)) + min_x;
        dest.y = (rand() % (max_y - min_y)) + min_y;
        dest = Position::make_position_valid(dest, max_x, min_x, max_y, min_y);
        if (!Position::valid_position(dest)) {
            valid_dest = false;
            cout << "fail\n";
        }
        else {
            valid_dest = true;
        }
    }
    return dest;
}

void Animal::general_search_walk(string target) {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];
    //walk to search
    if (c.general_search_dest.x == -1 || move_to(c.general_search_dest, "general searching")) {//initialize function object or reinitialize if reached destination
        c.general_search_dest = walk_search_random_dest();
    }//the move_to function triggers in the conditional
}









