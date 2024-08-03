#include "People.hpp"
using namespace std;


//Any functions that regard movement for People


//FIX: for some reason, people are going into occupied tiles under some specific condition and that removes the other person from the map and therefore also their image from the map
//need to fix any relevant function that calls move_to() such as general_search/random_walk so as to avoid going near disliked people and avoid even more and at greater distance any hated people.
//  otherwise everyone bunches up together because everyone likes someone and so they are only pull factors and no push factors in where people move_to. The only push factor right now that sort of works is campsite placement near friends.
//need to fix so that diagnonal movement is the same speed as non-diagonal movement
bool People::move_to(Position dest, string caller) {//need to include some check so that someone moving fast can't pass through obstacles by skipping over a tile
    bool reached = false;

    Position old_pos;
    Position new_pos;
    if (!pl[p].mov) {
        if (pl[p].general_search_called && caller != "general searching") {
            pl[p].general_search_called = false;
        }

        if (pl[p].downed || pl[p].immobile || pl[p].move_already || pl[p].age < MAX_INFANT_AGE) {//the image check shouldn't be necessary but I don't know why it still moves while having crafting image
            if (pl[p].pos == dest) {
                return true;
            }
            return false;
        }
        old_pos = pl[p].pos;
        new_pos = pl[p].pos;

        int sign = (new_pos.x > dest.x) - (new_pos.x < dest.x);//get -1/0/+1 according to whether left_var is greater/less/equal to right_var
        new_pos.x -= sign;//add inverse sign to get new_pos.x to be 1 step closer to dest.x
        sign = (new_pos.y > dest.y) - (new_pos.y < dest.y);
        new_pos.y -= sign;

        if (Environment::Map[new_pos.y][new_pos.x].person_id != -1) {//this is !=-1 instead of >-1 because -2 is reserved marker. If a person is blocking path, move to first empty tile in a clockwise direction
            pl[p].current_direction = { old_pos.x - new_pos.x, old_pos.y - new_pos.y };
            Position test_pos;
            bool next = false;
            for (Position pos : pl[p].rotations) {//still just a temp fix, need actual pathfinding algo
                if (next) {
                    test_pos = { old_pos.x + pos.x, old_pos.y + pos.y };
                    if (valid_position(test_pos) && test_pos != pl[p].pos && Environment::Map[test_pos.y][test_pos.x].person_id == -1) {
                        new_pos = test_pos;
                        break;
                    }
                }
                if (pl[p].current_direction == pos) {
                    if (next) {
                        new_pos = old_pos;//no free adjacent tile, don't move
                        break;
                    }
                    next = true;
                }
            }
        }

        pl[p].move_already = true;
        pl[p].dest = new_pos;//save the next tile destination
        Environment::Map[new_pos.y][new_pos.x].person_id = -2;//reserve tile so that no other person tries to enter it
        pl[p].main_dest = dest;//save long term destination
    }


    old_pos = pl[p].pos;
    new_pos = pl[p].dest;

    //fix this, if speed is larger than tile pixel size, then speed must also affect movement of tiles per tick but ensuring collisions in the path are still taken into account

    for (int i = 0;i < 2;i++) {//used to reduce x,y repetition of code, cleaner code
        int& po1 = (i == 0) ? old_pos.x : old_pos.y;//oldpos
        int& po2 = (i == 0) ? new_pos.x : new_pos.y;//newpos
        int& po3 = (i == 0) ? pl[p].px_x : pl[p].px_y;//px
        int& po4 = (i == 0) ? pl[p].pos.x : pl[p].pos.y;//pos

        int sign = (po1 < po2) - (po1 > po2);

        if (sign == 0 && po3 != 0) {//if tile destination reached but px position is not zeroed, move px position to 0
            sign = (po3 < 0) - (po3 > 0);
            po3 += sign * pl[p].speed;//movement for 2nd half is inverse direction from first half because px position is offset from normal tile position
        }
        else if (sign != 0) {
            po3 += sign * pl[p].speed;
            if (po3 == sign * TILE_PIXEL_SIZE / 2) {//tie to new tile when reached the midway point between tiles
                po3 *= -1;//invert
                po4 += sign;
                if (i == 0) {//handling x related image   Need to create methods that can recieve relevant data to decide which image to use outside of the normal npc logic. Game logic should not have to worry about images, only provide the data for another function to decide.
                    if (sign == 1) {
                        if (pl[p].walk_flip) {//use progress_state instead of a bool, fix
                            pl[p].current_image = "human_walking_right1";
                        }
                        else {
                            pl[p].current_image = "human_walking_right2";
                        }
                    }
                    else if (sign == -1) {
                        if (pl[p].walk_flip) {
                            pl[p].current_image = "human_walking_left1";
                        }
                        else {
                            pl[p].current_image = "human_walking_left2";
                        }
                    }
                    pl[p].walk_flip = !pl[p].walk_flip;
                }
            }
        }
        pl[p].mov = true;
    }

    if (pl[p].pos == new_pos) {
        if (pl[p].px_x == 0 && pl[p].px_y == 0) {
            pl[p].mov = false;
        }
        else {
            Environment::Map[old_pos.y][old_pos.x].person_id = -1;//remove person from Map
            Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = pl[p].id;//add person back to Map at new location

            int sign = (pl[p].pos.y > old_pos.y) - (pl[p].pos.y < old_pos.y);
            int dir_y = sign;
            sign = (pl[p].pos.x > old_pos.x) - (pl[p].pos.x < old_pos.x);
            int dir_x = sign;
            Environment::Tracks t = { "human", 0, dir_x, dir_y };//holds info on the direction the person went in, so someone else can follow the tracks in the right direction
            Environment::Map[old_pos.y][old_pos.x].track = t;
        }
    }
    if (!pl[p].mov && pl[p].pos == pl[p].main_dest) {
        reached = true;
        pl[p].clean_image = true;
    }
    return reached;
}


People::Position People::walk_search_random_dest() {
    Position dest = { -1,-1 };
    bool valid_dest = false;
    int tries = 0;//tracks how many times a destination roll has been tried, used to limit number of loops, need to find a better method
    while (!valid_dest) {//set destination by setting a direction and duration
        int max_x = Environment::map_x_max;//unsure if this is should be map_x_max or map_x_max-1 
        int min_x = 0;
        int max_y = Environment::map_y_max;
        int min_y = 0;
        if (pl[p].campsite_pos.x != -1) {
            max_x = pl[p].campsite_pos.x + campsite_distance_search;
            min_x = pl[p].campsite_pos.x - campsite_distance_search;
            max_y = pl[p].campsite_pos.y + campsite_distance_search;
            min_y = pl[p].campsite_pos.y - campsite_distance_search;
        }
        else if (!pl[p].sex && pl[p].spouse_id != -1) {//keeps near spouse if searching for new campsite, currently just has female follow male, otherwise both don't go far because they pull in different directions
            int spouse_index = p_by_id(pl[p].spouse_id);
            max_x = pl[spouse_index].pos.x + spouse_distance_limit;
            min_x = pl[spouse_index].pos.x - spouse_distance_limit;
            max_y = pl[spouse_index].pos.y + spouse_distance_limit;
            min_y = pl[spouse_index].pos.y - spouse_distance_limit;
        }
        dest.x = (rand() % (max_x - min_x)) + min_x;
        dest.y = (rand() % (max_y - min_y)) + min_y;
        dest = make_position_valid(dest, max_x, min_x, max_y, min_y);
        if (!valid_position(dest)) {
            valid_dest = false;
            tries++;//might no longer be necessary, the destination should be valid on the first try
        }
        else {
            valid_dest = true;
        }
    }
    return dest;
}


void People::general_search_walk(string target) {
    //walk to search
    if (pl[p].general_search_dest.x == -1 || move_to(pl[p].general_search_dest, "general searching")) {//initialize function object or reinitialize if reached destination
        pl[p].general_search_dest = walk_search_random_dest();
    }//the move_to function triggers in the conditional
}









