#include "Animal.hpp"
using namespace std;

vector<Animal::animal> Animal::al;
vector<Animal::Message> Animal::message_list;
vector<int> Animal::Message_Map[Environment::map_y_max][Environment::map_x_max];
int a = -1;//index for accessing current animal. Using index to access instead of a pointer because list may change such as when a new animal is born or dies which invalidates pointers to pl (people_list)
ItemSys it3;//used to access member functions and variables of ItemSys
int Animal::ox = -1;
int Animal::oy = -1;
set<string> Animal::species_names = { "deer","rabbit" };
int Animal::animal_id_iterator = 0;

Animal::Animal() {};
Animal::Animal(int init) {

    animal p1 = { new_animal_id(), {45,25}, true };
    p1.age = 11;
    p1.species = "rabbit";
    p1.current_image = p1.species;
    al.push_back(p1);
    Environment::Map[p1.pos.y][p1.pos.x].animal_id = p1.id;
    animal p2 = { new_animal_id(), {46,26}, false };
    p2.age = 11;
    p2.species = "rabbit";
    p2.current_image = p2.species;
    al.push_back(p2);
    Environment::Map[p2.pos.y][p2.pos.x].animal_id = p2.id;

    animal p3 = { new_animal_id(), {45,35}, true };
    p3.age = 11;
    p3.species = "rabbit";
    p3.current_image = p3.species;
    al.push_back(p3);
    Environment::Map[p3.pos.y][p3.pos.x].animal_id = p3.id;
    animal p4 = { new_animal_id(), {44,34}, false };
    p4.age = 11;
    p4.species = "rabbit";
    p4.current_image = p4.species;
    al.push_back(p4);
    Environment::Map[p4.pos.y][p4.pos.x].animal_id = p4.id;
}

int Animal::new_animal_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    animal_id_iterator++;
    return animal_id_iterator;
}

int Animal::new_message_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    message_id_iterator++;
    return message_id_iterator;
}

int Animal::message_by_id(int id) {//uses binary search to find and return index to message in list
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

void Animal::update_all(int day_count, int hour_count, int hours_in_day) {
//    cout << al.size() << "\n";
    for (int i = 0; i < al.size(); i++) {
        a = i;
        ox = al[a].pos.x;
        oy = al[a].pos.y;
        update(day_count, hour_count, hours_in_day);
        //if (al[a].isdead) {
        //    al.erase(al.begin() + i);
        //}
    }
    //clear global message list every other update. One issue with npcs cooperating or communicating is sequence order, 
    //as in if npc1 updates before npc2, anything npc2 says or does won't be witnessed unless it is still there on the next update, NEED TO FIX
    if (message_clear_flag) {
        for (int y = 0; y < Environment::map_y_max; y++) {//unsure if this is the most efficient way to clear Message_Map
            for (int x = 0; x < Environment::map_x_max; x++) {
                Message_Map[y][x].clear();
            }
        }
    }
    message_clear_flag = !message_clear_flag;
}

bool Animal::check_death() {
    bool rabbit_on_trap = al[a].species == "rabbit" && ItemSys::item_by_id(Environment::Map[al[a].pos.y][al[a].pos.x].item_id)!=-1 && ItemSys::item_list[ItemSys::item_by_id(Environment::Map[al[a].pos.y][al[a].pos.x].item_id)].item_name == "active trap";
    bool starvation = al[a].hunger_level > 1000;
    bool death = !al[a].is_alive || starvation || rabbit_on_trap;
    if (death) {
        al[a].is_alive = false;
       al[a].current_image = al[a].species+"_dead";
        if (al[a].age < 5) {
            al[a].current_image = al[a].species+"_infant_dead";
        }
        al[a].isdead = true;//signals that death function is done
        return true;
    }
    return false;
}

void Animal::update(int day_count, int hour_count, int hours_in_day) {
    if (check_death()) {
        return;
    }
    if (hour_count == 0) { //once a day check
        if (al[a].hunger_level > 150) { //tracks for continuous hungry days at the start of every day
            al[a].hungry_time++;
        }
        else {
            al[a].hungry_time = 0;
        }
        //if (al[a].function_record.size() >= 5) {
        //    al[a].function_record.erase(al[a].function_record.begin(), al[a].function_record.end() - 5);//once a day, erases all function records except the last 5
        //}
        al[a].age++;
    }
    


    al[a].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    al[a].tired_level++; //same for tired level

    al[a].reproduction_cooldown++; //for when to find mate and create new animal

    if (al[a].clean_image) {
        al[a].current_image = al[a].species;
        al[a].clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action
    utility_function();
    al[a].found_messages.clear();
    al[a].search_results.clear();


}

bool Animal::valid_position(Position pos) {
    bool valid_x = 0 <= pos.x && pos.x < Environment::map_x_max;
    bool valid_y = 0 <= pos.y && pos.y < Environment::map_y_max;
    if (valid_x && valid_y) {
        return true;
    }
    return false;
}

//animal sometimes stops moving, need to check functions that call move_to to make sure they aren't asking to move to the same tile animal is on
bool Animal::move_to(Position dest) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per animal per update.
    //if (al[a].search_active) {//prevents function getting called more than once per update
    //    return false;
    //}

    if (!valid_position(dest)) { //for debugging, kill npc if it tries to go off map or is asked to move to the spot it is already at
        al[a].is_alive = false;
        al[a].current_image = "pics/debug.png";
        return true;
    }
    Position old_pos = al[a].pos;
    Environment::Map[al[a].pos.y][al[a].pos.x].animal_id = -1;//remove animal from Map
    if (al[a].pos.x < dest.x) {//for future optimization, see: https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
        al[a].pos.x++;
    }
    else if (al[a].pos.x > dest.x) {
        al[a].pos.x--;
    }

    if (al[a].pos.y < dest.y) {
        al[a].pos.y++;
    }
    else if (al[a].pos.y > dest.y) {
        al[a].pos.y--;
    }
    bool reached = false;
    if (Environment::Map[al[a].pos.y][al[a].pos.x].animal_id == -1) {//if no other animal on this tile
        Environment::Map[al[a].pos.y][al[a].pos.x].animal_id = al[a].id;//add animal back to animal_Map at new location
        reached = al[a].pos == dest;
    }
    else {
        al[a].pos = old_pos;//NEED TO FIX: need to handle case where someone else is blocking path or occuppying destination
        //move to a random adjacent tile. Temporary fix
        //potential fix would be to have a direction attribute for animal and a function to handle turning right or left to then simply turn perpendicular to the obstacle until no obstacle is in the way.
        Position test_pos;
        for (int y = -1;y <= 1;y++) {
            for (int x = -1;x <= 1;x++) {
                test_pos.x = old_pos.x + x;
                test_pos.y = old_pos.y + y;
                if (valid_position(test_pos) && test_pos != al[a].pos && Environment::Map[test_pos.y][test_pos.x].animal_id == -1) {
                    al[a].pos = test_pos;
                    break;//need to handle if no adjacent tile is empty, fix this
                }
            }
        }
    }
    return reached;
}

int Animal::a_by_id(int id) {//uses binary search to find and return index to animal in people list (pl)
    int low = 0;
    int high = al.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (al[mid].id == id) {
            return mid;
        }
        (al[mid].id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

bool Animal::reproduce() {//later, add marriage ceremony/customs, options for polygamy, infidelity, premarital sex, widow status, age and family restrictions on potential mates, family size limits, divorce, etc
    if (al[a].reproduction_cooldown < 10) {//function trigger
        return false;
    }
    vector<Position>& pos_list1 = al[a].search_results[al[a].species];//note: using reference (&) reduces copying
    int p2 = -1;
    for (int i = 0; i < pos_list1.size(); i++) {//filter out valid mates from people found list
        int pers_id = Environment::Map[pos_list1[i].y][pos_list1[i].x].animal_id;
        int pid = a_by_id(pers_id);
        if (al[pid].species==al[a].species && al[pid].sex != al[a].sex && al[pid].age > 10) {
            p2 = pid;
        }
    }
    bool mate_willing = false;
    if (p2 != -1) {
        if (al[p2].reproduction_cooldown > 1000 && al[p2].sex != al[a].sex) {
            mate_willing = true;
        }
        if (mate_willing && (Position::distance(al[a].pos, al[p2].pos) == 1 || move_to(al[p2].pos))) {//go to tile adjacent to p2
            //create a new human, add pregnancy later, only female creates child
            if (!al[a].sex) {
                int sex = rand() % 2;
                Position child_pos;
                if (al[a].search_results.find("no animal") != al[a].search_results.end()) {
                    if (Position::distance(al[a].search_results["no animal"][0], al[a].pos) == 1) {//if empty adjacent tile
                        child_pos = al[a].search_results["no animal"][0];
                    }
                }
                if (child_pos.x == -1) {//if no empty adjacent tile found
                    move_to(walk_search_random_dest());//move to a random adjacent tile
                    return true;//in progress
                }
                animal child = { new_animal_id(), child_pos, sex };
                child.species = al[a].species;
                Environment::Map[child.pos.y][child.pos.x].animal_id = child.id;
                al.push_back(child);
                al[a].reproduction_cooldown = 0;//reset
                al[p2].reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                
                return true;//male will simply no longer call reproduce() given the cooldown==0, so only female needs to return true
            }
            else {
                //if male, simply wait for female to update one's reproduction_cooldown
            }
        }
    }
    else {//if no mate found, walk to search
            general_search_walk("mate");
        
    }
    return true;//in progress
}

void Animal::find_all() {//returns all things (items, people, messages, etc) found, sorted according into Position lists for each thing type
    int radius_options[2] = {//all radius options
        al[a].sightline_radius, al[a].audioline_radius
    };
    if (al[a].radiusmax == -1) {//used to store result instead of calling every time, only resets if one of the radius options changes such as damaged eyesight, etc
        for (int i = 0; i < 2; i++) {//selects largest radius
            if (i == 0) {
                al[a].radiusmax = radius_options[i];
            }
            else if (al[a].radiusmax < radius_options[i]) {
                al[a].radiusmax = radius_options[i];
            }
        }
    }
    map<string, vector<Position>> search_results;
    Position o = al[a].pos;//origin
    vector<int> target_quantity_current;
    for (int radius = 0; radius <= al[a].radiusmax; radius++) { //this function checks tilemap in outward rings by checking top/bottom and left/right ring boundaries
        if (radius == 0) {//avoids double checking origin
            if (radius <= al[a].sightline_radius) {
                if (Environment::Map[o.y][o.x].item_id != -1) {
                    ItemSys::Item& item = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[o.y][o.x].item_id)];
                    search_results.insert({ item.item_name,{o} });
                }
                else {//creates list of tiles without any item, for use when placing an item on the map
                    search_results.insert({ "no item",{o} });
                }
                //don't check if animal is on origin tile, because that animal is self
            }
            if (radius <= al[a].audioline_radius) {
                //check for messages
                check_tile_messages(o);
            }
        }
        int xmin = o.x - radius;
        int xmax = o.x + radius;
        int ymin = o.y - radius + 1;//+1 and -1 to avoid double checking corners
        int ymax = o.y + radius - 1;
        for (int x = xmin, y = ymin; x <= xmax; x++, y++) {
            for (int sign = -1; sign <= 1; sign += 2) {//sign == -1, then sign == 1
                Position pos1 = { x,o.y + (sign * radius) };
                Position pos2 = { o.x + (sign * radius), y };
                //check for people
                if (valid_position(pos1)) {
                    if (Environment::Map[pos1.y][pos1.x].animal_id != -1) {
                        string species = al[a_by_id(Environment::Map[pos1.y][pos1.x].animal_id)].species;
                        if (search_results.find(species) != search_results.end()) {//check if key exists
                            //key found
                            search_results[species].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ species,{pos1} });
                        }
                    }
                    else {//creates list of tiles without any item, for use when placing an item on the map
                        if (search_results.find("no animal") != search_results.end()) {//check if key exists
                            //key found
                            search_results["no animal"].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ "no animal",{pos1} });
                        }
                    }
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        if (Environment::Map[pos2.y][pos2.x].animal_id != -1) {
                            string species = al[a_by_id(Environment::Map[pos2.y][pos2.x].animal_id)].species;
                            if (search_results.find(species) != search_results.end()) {//check if key exists
                                //key found
                                search_results[species].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ species,{pos2} });
                            }
                        }
                        else {//creates list of tiles without any item, for use when placing an item on the map
                            if (search_results.find("no animal") != search_results.end()) {//check if key exists
                                //key found
                                search_results["no animal"].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ "no animal",{pos2} });
                            }
                        }
                    }
                }
                
                //check for items
                if (valid_position(pos1)) {
                    if (Environment::Map[pos1.y][pos1.x].item_id != -1) {
                        string item_name = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos1.y][pos1.x].item_id)].item_name;
                        if (search_results.find(item_name) != search_results.end()) {//check if key exists
                            //key found
                            search_results[item_name].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ item_name,{pos1} });
                        }
                    }
                    else {//creates list of tiles without any item, for use when placing an item on the map
                        if (search_results.find("no item") != search_results.end()) {//check if key exists
                            //key found
                            search_results["no item"].push_back({ pos1 });
                        }
                        else {
                            //key not found
                            search_results.insert({ "no item",{pos1} });
                        }
                    }
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        if (Environment::Map[pos2.y][pos2.x].item_id != -1) {
                            string item_name = ItemSys::item_list[ItemSys::item_by_id(Environment::Map[pos2.y][pos2.x].item_id)].item_name;
                            if (search_results.find(item_name) != search_results.end()) {//check if key exists
                                //key found
                                search_results[item_name].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ item_name,{pos2} });
                            }
                        }
                        else {//creates list of tiles without any item, for use when placing an item on the map
                            if (search_results.find("no item") != search_results.end()) {//check if key exists
                                //key found
                                search_results["no item"].push_back({ pos2 });
                            }
                            else {
                                //key not found
                                search_results.insert({ "no item",{pos2} });
                            }
                        }
                    }
                }
                //check for messages
                if (valid_position(pos1)) {
                    check_tile_messages(pos1);
                }
                if (y <= ymax) {
                    if (valid_position(pos2)) {
                        check_tile_messages(pos2);
                    }
                }
            }
        }
    }
    al[a].search_results = search_results;
}

void Animal::check_tile_messages(Position pos) {
    //might also serve as a generic for reacting to sounds
    for (int m_id : Message_Map[pos.y][pos.x]) {//check all messages in this tile
        if (!al[a].found_messages.empty()) {
            bool repeated_message = false;
            for (int m1_id : al[a].found_messages) {
                if (m_id == m1_id) {//avoids copying messages that differ only in their location
                    repeated_message = true;
                    break;
                }
            }
            if (!repeated_message && message_list[message_by_id(m_id)].sender_id != al[a].id) {
                al[a].found_messages.push_back(m_id);
            }
        }
        else if (message_list[message_by_id(m_id)].sender_id != al[a].id) {
            al[a].found_messages.push_back(m_id);
        }
    }
}

//can this function also be folded into the find_all function somehow to further reduce for loops searching on the map?
//might also need to add some restriction so that each animal doesn't blast out too many messages at once? Or maybe that's ok?
void Animal::speak(string message_text, int receiver_id) {//if receiver_id == -1, then the message is for everyone
    //current valid messages include: need to list valid messages here
    //the outward ring method might make more sense in this function to allow certain objects such as walls to block sound, might implement later but not currently
    Message m = { new_message_id(), al[a].id, receiver_id, message_text, al[a].pos };//creates message
    for (int y = al[a].pos.y - al[a].audioline_radius; y < al[a].pos.y + al[a].audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = al[a].pos.x - al[a].audioline_radius; x < al[a].pos.x + al[a].audioline_radius; x++) {
            if (valid_position({ x,y })) {
                Message_Map[y][x].push_back(m.message_id);
                message_list.push_back(m);
            }
        }
    }
}

Animal::Position Animal::walk_search_random_dest() {
    Position dest = { -1,-1 };
    bool valid_dest = false;
    int tries = 0;//tracks how many times a destination roll has been tried, used to limit number of loops, need to find a better method
    while (!valid_dest) {//set destination by setting a direction and duration
        int max_x = Environment::map_x_max;//unsure if this is should be map_x_max or map_x_max-1 
        int min_x = 0;
        int max_y = Environment::map_y_max;
        int min_y = 0;
        

        
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

Animal::Position Animal::make_position_valid(Position dest, int ux, int lx, int uy, int ly) {//bounds: u==upper, l==lower
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

bool Animal::idle() {
    //flip between idle and default image
    int option = rand() % 100;
    if (option < 50) {
        al[a].current_image = al[a].species + "_resting";
    }
    else {
        al[a].current_image = al[a].species;
        general_search_walk("wandering");
    }
    al[a].clean_image = true;
    return true;
}

int Animal::Position::distance(Position pos1, Position pos2) {
    int xd = abs(pos1.x - pos2.x);
    int yd = abs(pos1.y - pos2.y);
    int max = -1;
    (xd > yd) ? max = xd : max = yd;
    return max;
}

void Animal::create_item(string item_type, Position pos) {
    ItemSys::Item new_item = it3.presets[item_type];
    new_item.item_id = ItemSys::new_item_id();
    ItemSys::item_list.push_back(new_item);
    
        Environment::Map[pos.y][pos.x].item_id = new_item.item_id;//create item, then place on map and global item list
    
}



void Animal::delete_item(int item_id, Position pos, int index) {
    if (item_id == -1) {//don't know what's causing this issue but need this check to work
        return;
    }
    int item_index = ItemSys::item_by_id(item_id);
    ItemSys::item_list.erase(ItemSys::item_list.begin() + item_index);//remove item from global item_list
    if (pos.x != -1) {//if pos.x == -1, then the item was not on the map and was probably in a animal's inventory from which it was deleted separately
        Environment::Map[pos.y][pos.x].item_id = -1; //removes item from map
    }
}

void Animal::general_search_walk(string target) {
    //walk to search
    if (al[a].general_search_dest.x == -1 || move_to(al[a].general_search_dest)) {//initialize function object or reinitialize if reached destination
        al[a].general_search_dest = walk_search_random_dest();
    }//the move_to function triggers in the conditional
    //al[a].search_active = true;
}

//for filtering search results by tag such as food
vector<Animal::Position> Animal::filter_search_results(string target) {
    set<Position> unique_results;//used to store unique positions only
    for (string i : it3.tags[target]) {
        if (al[a].search_results.find(i) != al[a].search_results.end()) {
            for (Position pos : al[a].search_results[i]) {
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

bool Animal::acquire(string target) {
    //add_func_record("acquiring " + target);
    //check if target is an item name or item tag
    if (it3.presets.find(target) != it3.presets.end()) {
        //target is an item name
    }
    else if (it3.tags.find(target) != it3.tags.end()) {
        //target is a tag name
        for (string item_name : it3.tags[target]) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            //cout << item_name;
            if (acquire(item_name)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    else {
        return false;//target isn't valid
    }

    //look around self for item
    if (al[a].search_results.find(target) != al[a].search_results.end()) {//key found, if key exists then at least 1 was found
        if (al[a].search_results[target].empty()) {//don't know why it was out of vector range on first element if the above had passed, temp fix
            return false;//error, fix this
        }
        Position pos = al[a].search_results[target][0];
        int item_id = Environment::Map[pos.y][pos.x].item_id;
        if (move_to(pos)) {//if item is found, move to it and pick it up
            return true;//item picked up
        }
        return false;//if still moving towards item, continue to next tick
    }
    
    //if all fails, move in search pattern. Search pattern is shared, to reduce erratic movement from various instances of search patterns
   general_search_walk(target); 
    return false;//searching
}

void Animal::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    if (sleeping()) {}
    else if (eating()) {}//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    else if (reproduce()) {}
    else { idle(); }
}

bool Animal::sleeping() {
    bool tired = al[a].tired_level > 50;//might need to cap sleep such that a animal can't ever have a tired_level over x_level as well as under y_level
    if (!(!al[a].awake || tired)) {//function trigger
        return false;
    }
    al[a].current_image = al[a].species+"_sleeping";
    al[a].awake = false;
    al[a].tired_level -= 11; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function
    if (al[a].tired_level <= 0) {//fix this, need to cap at 0, also need cap for upper limit?
        al[a].clean_image = true;
        al[a].awake = true;
        return true;//done
    }
    return true;//in progress
}

bool Animal::eating() {
    bool hungry = al[a].hunger_level > 150;
    bool has_food = false;
    if (al[a].diet == "") {
        if (al[a].species == "deer") {
            al[a].diet = "grain";
        }
        else if (al[a].species == "rabbit") {
            al[a].diet = "berrybush";
        }
    }
    vector<Position> food_pos_list = al[a].search_results[al[a].diet];

    Position food_pos;
    if (!food_pos_list.empty()) {
        if (Position::distance(food_pos_list[0], al[a].pos) <= 1) {
            has_food = true;
            food_pos = food_pos_list[0];
            //cout << "food found";
        }
    }

    if (hungry && !has_food) {
        if (acquire(al[a].diet)) {
            //done
            return true;
        }
        else {
            return true;
        }
    }

    if (!(hungry && has_food)) {//function trigger
        return false;
    }
   if (al[a].eating_progress.progress == 0) {
        al[a].current_image = al[a].species+"_eating";
    }
    if (al[a].eating_progress.progress_func()) {//makes eating take more than 1 frame
        int food_id = Environment::Map[food_pos.y][food_pos.x].item_id;
        delete_item(food_id, food_pos, -1);//delete food from game
        al[a].hunger_level -= 150; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        al[a].clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
    return true;//in progress
}

