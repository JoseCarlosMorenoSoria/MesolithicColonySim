#include "People.hpp"
using namespace std;

//Simple functions that provide services to main People functions.


int People::new_person_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    people_id_iterator++;
    return people_id_iterator;
}

int People::new_message_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    message_id_iterator++;
    return message_id_iterator;
}

int People::message_by_id(int id) {//uses binary search to find and return index to message in list
    int low = 0;
    int high = message_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (message_list[mid].message_id == id) {
            return mid;
        }
        (message_list[mid].message_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

void People::add_func_record(string s) {
    if (pl[p].function_record.empty()) {
        pl[p].function_record.push_back(s);
        pl[p].function_record_nums.push_back(1);
    }
    else {
        if (pl[p].function_record.back() == s) {
            pl[p].function_record_nums.back()++;
        }
        else {
            pl[p].function_record.push_back(s);
            pl[p].function_record_nums.push_back(1);
        }
    }
}

int People::p_by_id(int id) {//uses binary search to find and return index to person in people list (pl)
    int low = 0;
    int high = pl.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (pl[mid].id == id) {
            return mid;
        }
        (pl[mid].id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

void People::find_all() {//returns all things (items, people, messages, etc) found, sorted according into Position lists for each thing type
    auto find_all_helper = [&](Position pos, string type) {//lamda function to avoid having helper functions in the general People scope
        if (!valid_position(pos)) { return; }
        string key;
        if (type == "temperature") {
            key = to_string(Environment::Map[pos.y][pos.x].temperature);
        }
        else if (type == "tracks") {
            if (Environment::Map[pos.y][pos.x].track.track_age != -1) {
                if (Environment::Map[pos.y][pos.x].track.creature == "human") {
                    key = "human tracks";
                }
                else {
                    key = "animal tracks";
                }
            }
        }
        else if (type == "people") {
            if (pl[p].lastpos == pos) {
                throw std::invalid_argument("my error");
            }
            pl[p].lastpos = pos;
            if (Environment::Map[pos.y][pos.x].person_id > -1) { key = "people"; }//the id must be >-1 because -2 is a reserved marker for move to a tile in move_to()
            else if (Environment::Map[pos.y][pos.x].person_id == -2) { key = "reserved for movement"; }
            else { key = "no people"; }
        }
        else if (type == "item") {
            if (Environment::Map[pos.y][pos.x].item_id != -1) { key = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos.y][pos.x].item_id)].item_name; }
            else { key = "no item"; }
        }
        else if (type == "animal") {
            if (Environment::Map[pos.y][pos.x].animal_id != -1) { key = Animal::al[anim1.a_by_id(Environment::Map[pos.y][pos.x].animal_id)].species; }
            else { key = "no animal"; }
        }
        else if (type == "terrain") { key = Environment::Map[pos.y][pos.x].terrain; }

        if (pl[p].search_results.find(key) != pl[p].search_results.end()) {//check if key exists
            pl[p].search_results[key].push_back(pos);
        }//key found
        else { pl[p].search_results.insert({ key,{pos} }); }//key not found
    };

    const int NUM_OF_RADII = 2;//why not use a vector instead of an array and just get size()?
    int radius_options[NUM_OF_RADII] = {//all radius options
        pl[p].sightline_radius, pl[p].audioline_radius
    };
    if (pl[p].radiusmax == -1) {//used to store result instead of calling every time, only resets if one of the radius options changes such as damaged eyesight, etc
        for (int i = 0; i < NUM_OF_RADII; i++) {//selects largest radius
            if (i == 0) {
                pl[p].radiusmax = radius_options[i];
            }
            else if (pl[p].radiusmax < radius_options[i]) {
                pl[p].radiusmax = radius_options[i];
            }
        }
    }
    Position o = pl[p].pos;//origin
    vector<int> target_quantity_current;
    for (int radius = 0; radius <= pl[p].radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            if (radius <= pl[p].sightline_radius) {
                vector<string> types = { "item","animal","terrain", "tracks", "temperature" };
                for (string s : types) {
                    find_all_helper(o, s);
                }
            }
            if (radius <= pl[p].audioline_radius) {
                //check for messages
                check_tile_messages(o);
            }
            continue;
        }
        int xmin = o.x - radius;
        if (xmin < 0) { xmin = 0; }//these reduce iterations when near edges of map
        int xmax = o.x + radius;
        if (xmax > Environment::map_x_max - 1) { xmax = Environment::map_x_max - 1; }
        int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
        if (ymin < 0) { ymin = 0; }
        int ymax = o.y + radius - 1;
        if (ymax > Environment::map_y_max - 1) { ymax = Environment::map_y_max - 1; }
        for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
            for (int sign = -1; sign <= 1; sign += 2) {//sign == -1, then sign == 1
                Position pos1 = { x,o.y + (sign * radius) };
                Position pos2 = { o.x + (sign * radius), y };

                //if (pos1==pos2) {
                //        throw std::invalid_argument("my error2");
                 //   }

                if (radius <= pl[p].sightline_radius) {
                    vector<string> types = { "people","item","animal","terrain", "tracks", "temperature" };
                    for (string s : types) {
                        find_all_helper(pos1, s);
                        if (y <= ymax && pos1 != pos2) {//need to figure out why pos1 sometimes == pos2 and rewrite for loops to avoid this
                            find_all_helper(pos2, s);
                        }
                    }
                }
                if (radius <= pl[p].audioline_radius) {
                    //check for messages
                    if (valid_position(pos1)) {
                        check_tile_messages(pos1);
                    }
                    if (y <= ymax && pos1 != pos2) {
                        if (valid_position(pos2)) {
                            check_tile_messages(pos2);
                        }
                    }
                }
            }
        }
    }
}

void People::check_tile_messages(Position pos) {
    //might also serve as a generic for reacting to sounds
    for (int m_id : Message_Map[pos.y][pos.x]) {//check all messages in this tile
        if (!pl[p].found_messages.empty()) {
            bool repeated_message = false;
            for (int m1_id : pl[p].found_messages) {
                if (m_id == m1_id) {//avoids copying messages that differ only in their location
                    repeated_message = true;
                    break;
                }
            }
            if (!repeated_message && message_list[message_by_id(m_id)].sender_id != pl[p].id) {
                pl[p].found_messages.push_back(m_id);
            }
        }
        else if (message_list[message_by_id(m_id)].sender_id != pl[p].id) {
            pl[p].found_messages.push_back(m_id);
        }
    }
}

vector<int> People::inventory_has(string target) {//return list of indexes of matching items in inventory if found, empty list if not found
    vector<int> indexes;
    if (pl[p].item_inventory.empty()) {
        return indexes;
    }
    for (int i = 0; i < pl[p].item_inventory.size(); i++) {
        int item_index = ItemSys::item_by_id(pl[p].item_inventory[i]);
        if (item_index == -1) {//this shouldn't be happening, need to figure out why, temp fix. fix this
            return indexes;
        }
        if (ItemSys::item_list[item_index].item_name == target) {
            indexes.push_back(i);
        }
        for (int j = 0; j < ItemSys::item_list[item_index].tags.size(); j++) {
            if (ItemSys::item_list[item_index].tags[j] == target) {
                indexes.push_back(i);
                break;
            }
        }
    }
    return indexes;
}

void People::create_item(string item_type, Position pos) {
    ItemSys::Item new_item = it2.presets[item_type];
    new_item.item_id = ItemSys::new_item_id();
    ItemSys::item_list.push_back(new_item);
    if (pos.x == -1) {
        pl[p].item_inventory.push_back(new_item.item_id);
    }
    else {
        Environment::Map[pos.y][pos.x].item_id = new_item.item_id;//create item, then place on map and global item list
    }
}

void People::pick_up_item(int item_id, Position pos) {
    if (Environment::Map[pos.y][pos.x].item_id == -1) {
        cout << "pick up item error";
    }
    pl[p].current_image = "pics/human_gathering.png";
    pl[p].clean_image = true;
    pl[p].item_inventory.push_back(Environment::Map[pos.y][pos.x].item_id); //add item's item_id to inventory
    Environment::Map[pos.y][pos.x].item_id = -1; //remove item from map
}

void People::delete_item(int item_id, Position pos, int inventory_index) {
    if (item_id == -1) {//don't know what's causing this issue but need this check to work
        return;
    }
    int item_index = ItemSys::item_by_id(item_id);
    ItemSys::item_list.erase(ItemSys::item_list.begin() + item_index);//remove item from global item_list
    if (pos.x != -1) {//if pos.x == -1, then the item was not on the map and was probably in a Person's inventory from which it was deleted separately
        Environment::Map[pos.y][pos.x].item_id = -1; //removes item from map
    }
    if (inventory_index != -1) {
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + inventory_index);//delete item from inventory
    }
}

bool People::drop_item(int index) {
    //if dropping item use search_results["no item"] to find an empty tile
    Position dropsite;
    if (pl[p].search_results.find("no item") != pl[p].search_results.end()) {
        dropsite = pl[p].search_results["no item"][0];
        if (Position::distance(dropsite, pl[p].pos) > 1) {//only drop on adjacent tile or one's own tile
            return false;
        }
        Environment::Map[dropsite.y][dropsite.x].item_id = pl[p].item_inventory[index];//place on map
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from inventory
        pl[p].dropsite = dropsite;
        return true;
    }
    else {
        return false;
    }
    //if dropping an infant, use "no people"    Not yet implemented, fix this
    /*
         Position child_pos;
            if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
                if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                    child_pos = pl[p].search_results["no people"][0];
                }
            }
            if (child_pos.x == -1) {//if no empty adjacent tile found
                move_to(walk_search_random_dest());//move to a random adjacent tile
                return true;//in progress
            }
    */

}

//for filtering search results by tag such as food
vector<Position> People::filter_search_results(string target) {
    set<Position> unique_results;//used to store unique positions only
    for (string i : it2.tags[target]) {
        if (pl[p].search_results.find(i) != pl[p].search_results.end()) {
            for (Position pos : pl[p].search_results[i]) {
                unique_results.insert(pos);
            }
        }
    }
    vector<Position> final_results;//unsure if it's worth converting to a vector or whether it should return a set
    for (Position pos : unique_results) {
        final_results.push_back(pos);
    }
    return final_results;
}

void People::pickup_infants() {//FIX THIS DUE TO CHANGES IN RENDERER
    //if have infants, carry them
    if (!pl[p].children_id.empty()) {//if the game renderer stops running through the whole people list, this breaks because infant is no longer on map. Therefore need to create a better function for carrying and dropping both infants and general items/people. Currently also carries all infants in the same spot, so they overlap. Need to fix this.
        for (int i = 0; i < pl[p].children_id.size(); i++) {
            int kid_index = p_by_id(pl[p].children_id[i]);
            if (pl[kid_index].age < MAX_INFANT_AGE && !pl[kid_index].being_carried) {
                if (Position::distance(pl[kid_index].pos, pl[p].pos) == 1 || move_to(pl[kid_index].pos, "to infant")) {
                    pl[kid_index].being_carried = true;
                    pl[kid_index].carried_by_id = pl[p].id;
                    Environment::Map[pl[kid_index].pos.y][pl[kid_index].pos.x].person_id = -1;
                    for (int j = 0; j < pl[p].search_results["people"].size(); j++) {//ensure integrity of search_results (due to possible vector out of bounds error source)
                        if (pl[p].search_results["people"][j] == pl[kid_index].pos) {
                            pl[p].search_results["people"].erase(pl[p].search_results["people"].begin() + j);
                        }
                    }
                    pl[kid_index].pos = { -1,-1 };
                    cout << "carrying kid";
                }
            }
        }
    }
}

bool People::drop_infants() {
    //if have infants, drop them
    if (!pl[p].children_id.empty()) {
        for (int i = 0; i < pl[p].children_id.size(); i++) {
            int kid_index = p_by_id(pl[p].children_id[i]);
            if (pl[kid_index].being_carried && pl[kid_index].carried_by_id == pl[p].id) {

                Position child_pos;
                if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
                    if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                        child_pos = pl[p].search_results["no people"][0];
                    }
                }
                if (child_pos.x == -1) {//if no empty adjacent tile found
                    pl[p].general_search_called = true;
                    return false;//in progress
                }

                pl[kid_index].pos = child_pos;
                Environment::Map[child_pos.y][child_pos.x].person_id = pl[kid_index].id;
                pl[kid_index].being_carried = false;
                pl[kid_index].carried_by_id = -1;
            }
        }
    }
    return true;//done
}

vector<int> People::remove_dup(vector<int> v) {
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


