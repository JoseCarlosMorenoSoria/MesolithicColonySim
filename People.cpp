#include "People.hpp"
using namespace std;

vector<People::Person> People::pl; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<People::Message> People::message_list;
vector<int> People::Message_Map[Environment::map_y_max][Environment::map_x_max];
int People::p = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
ItemSys People::it2;//used to access member functions and variables of ItemSys
int People::ox = -1;
int People::oy = -1;
int People::pday_count;
int People::phour_count;
int People::people_id_iterator = 0;
Animal anim1;//to access Animal class functions

//Need to unit test every function and also test the frequency that each function executes, average value for all variables, and different scenarios (resource/people density, size, environment, etc)

//Currently, an npc wins the game on average in 750 ticks, nearly 8 days. Might be less such as 600 or more such as 950. Only ran 3 tests to check tick amount.

//note: readability might be increased by creating null constants for things like int (-1) and Position (-1,-1), etc.
//could also maybe just overload and use the operator! as a return true or false if null function

//TO DO TODAY
//update Animal class with current People class plus the unique Animal variables/functions
//implement player functionality and UI
//then implement more simple programmer art, animations and items/terrain/cosmetics (like weather), etc.


People::People(){}

People::People(int initint) {
    
    Person p1 = { new_person_id(), {50,25}, true};
    p1.age = 11;
    pl.push_back(p1);
    Environment::Map[p1.pos.y][p1.pos.x].person_id = p1.id;

    
    Person p2 = { new_person_id(), {51,26}, false};
    p2.age = 11;
    pl.push_back(p2);
    Environment::Map[p2.pos.y][p2.pos.x].person_id = p2.id;
    
    Person p3 = { new_person_id(), {50,24}, true };
    p3.age = 11;
    pl.push_back(p3);
    Environment::Map[p3.pos.y][p3.pos.x].person_id = p3.id;
    Person p4 = { new_person_id(), {52,23}, true };
    p4.age = 11;
    pl.push_back(p4);
    Environment::Map[p4.pos.y][p4.pos.x].person_id = p4.id;
    Person p5 = { new_person_id(), {53,22}, false };
    p5.age = 11;
    pl.push_back(p5);
    Environment::Map[p5.pos.y][p5.pos.x].person_id = p5.id;
    Person p6 = { new_person_id(), {54,21}, false };
    p6.age = 11;
    pl.push_back(p6);
    Environment::Map[p6.pos.y][p6.pos.x].person_id = p6.id;
    
}

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

int ticks = 0;
void People::update_all(int day_count, int hour_count, int hours_in_day) {
    //if (game_won) {//for debugging, checking if game is winnable
    //    cout << "Number of ticks: " << ticks << "\n";
    //    return;
   // }

    //ticks++;

    pday_count = day_count;
    phour_count = hour_count;

    Environment::people_in_stealth.clear();//clears every tick for simpler implementation. This needs to be rewritten in a better method instead of using Environment to communicate with Animal

    int top_auth = 0;
    for(int i = 1; i < pl.size(); i++) {//i starts at 1 because for now, the first Person in the list is reserved for player control
        p = i;
        ox = pl[p].pos.x;
        oy = pl[p].pos.y;
        update(day_count, hour_count, hours_in_day);
        if (pl[p].in_stealth) {
            Environment::people_in_stealth.push_back(pl[p].id);
        }
        if (pl[p].authority > top_auth) {
            top_auth = pl[p].authority;
        }
    }
    //cout << top_auth << "\n";
    
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
    //for debugging
    if (day_count == -1) {//kill all people. set to -1 to prevent execution
        for (Person& pip : pl) {
            pip.is_alive = false;
        }
    }
    /*
    if (day_count % 5 == 0 && hour_count==0) {
        for (int i = 0; i < pl.size(); i++) {
            cout << "id: " << pl[i].id << " -- \n";
            for (auto const& d : pl[i].dispositions) {
                cout << "pers: " << d.first << " disp:" << d.second << "\n";
            }
            cout << "----------------------------\n\n\n";
        }
    }
    */
}

bool People::check_death() {
    //for testing winnability, no death at all
    return false;

    bool starvation = pl[p].hunger_level > STARVATION_LEVEL;
    bool dehydration = pl[p].thirst_level > DEHYDRATION_LEVEL;
    bool old_age = pl[p].age > MAX_AGE;
    bool freeze_death = pl[p].my_temperature <= COLD_DEATH_TEMPERATURE;
    bool heat_death = pl[p].my_temperature >= HEAT_DEATH_TEMPERATURE;
    bool fatal_injury = pl[p].injured_time == INJURED_TIME_DEATH;
    bool fatal_sickness = pl[p].sick_time == SICK_TIME_DEATH;

    bool death = !pl[p].is_alive || starvation || dehydration || old_age || freeze_death || heat_death || fatal_injury || fatal_sickness;
    if (death) {
        pl[p].is_alive = false;
        add_func_record("dead");
        pl[p].current_image = "pics/human_dead.png";
        if (pl[p].age < MAX_INFANT_AGE) {
            pl[p].current_image = "human_infant_dead";
        }
        //if have spouse, free spouse to remarry, need a more realistic way to handle this rather than instant long distance unlinking
        if (pl[p].spouse_id != -1) {
            pl[p_by_id(pl[p].spouse_id)].spouse_id = -1;//unlink from spouse
        }
        //set these to -1 to prevent others referencing them
        pl[p].hunger_level = -1;
        pl[p].thirst_level = -1;
        pl[p].hungry_time = -1;
        pl[p].reproduction_cooldown = -1;
        if(!pl[p].printed){
            //print record of function history for debugging
            cout << "\n______\n"<<pl[p].id<<" ~ ";
            for (int i = 0; i < pl[p].function_record.size(); i++) {
                cout << pl[p].function_record[i] << " - " << pl[p].function_record_nums[i] << " ::: ";
            }
            cout<<"\n"
            <<"\nsex: " << pl[p].sex
            << "\nhunger_level: " << pl[p].hunger_level
            << "\nhungry_time: " << pl[p].hungry_time
            << "\ntired_level: " << pl[p].tired_level
            << "\nitem_inventory size: " << pl[p].item_inventory.size()
            << "\nCampsite pos: x- " << pl[p].campsite_pos.x <<" y-"<<pl[p].campsite_pos.y
            << "\ncampsite_age: " << pl[p].campsite_age
            << "\nreproduction_cooldown: " << pl[p].reproduction_cooldown
            << "\nfound_messages size: " << pl[p].found_messages.size();

            pl[p].printed = true;
        }
        return true;
    }
    return false;
}

void People::update(int day_count, int hour_count, int hours_in_day) {
    if (check_death()) {
        return;
    }
	if (hour_count == 0) { //once a day check
		if (pl[p].hunger_level > HUNGRY_LEVEL) { //tracks for continuous hungry days at the start of every day
			pl[p].hungry_time++;
		}
		else {
			pl[p].hungry_time = 0;
		}
        //if (pl[p].function_record.size() >= 5) {
        //    pl[p].function_record.erase(pl[p].function_record.begin(), pl[p].function_record.end() - 5);//once a day, erases all function records except the last 5
        //}
        pl[p].age++;
    }
    if (pl[p].age < MAX_INFANT_AGE) {//is infant. Currently that means it doesn't do anything except get hungry and needs to be fed
        pl[p].current_image = "human_infant";
        pl[p].hunger_level++;
        if (pl[p].hunger_level > HUNGRY_LEVEL) {
            speak("requesting food",-1);
        }
        if (pl[p].being_carried) {//if being carried, then position is the position of the carrier offset by 1
            pl[p].pos = pl[p_by_id(pl[p].carried_by_id)].pos;
            pl[p].pos.x += 1;
        }
        pl[p].immobile = true;
        eating();
        return;
    }
    else {
        pl[p].immobile = false;
        pl[p].clean_image = true;
    }

    

    pl[p].hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    pl[p].tired_level++; //same for tired level
    pl[p].campsite_age++;
    pl[p].reproduction_cooldown++; //for when to find mate and create new person
    pl[p].thirst_level++;
    pl[p].recreation_level++;
    pl[p].dirtiness++;

    if (pl[p].am_injured) {
        pl[p].injured_time++;
    }
    if (pl[p].am_sick) {
        pl[p].sick_time++;
    }

    int insulation_cold = 0;//unsure if insulation from heat makes sense, except from hats and maybe light white clothing? Only insulation from cold used for now.
    for (auto const& i : pl[p].equipped.equipment) {
        if (i.second != -1) {
            insulation_cold += ItemSys::item_list[ItemSys::item_by_id(i.second)].insulation_from_cold;
        }
    }
    if (Environment::Map[pl[p].pos.y][pl[p].pos.x].temperature > pl[p].my_temperature) {//like other needs, having this update every tick is not ideal and should be changed.
        pl[p].my_temperature++;//fix this, need to make increase/decrease a percent of the difference between my_temp and tile_temp. Such that a large difference causes a large change in my_temp but a small one does not. Do the same for the clothing insulation below.
        
    }
    else if (Environment::Map[pl[p].pos.y][pl[p].pos.x].temperature < pl[p].my_temperature) {
        if (insulation_cold == 0) {
            pl[p].my_temperature--; //for now, insulation is essentially a bool that prevents getting colder than already are. Need to implement a more thought out system of person temp change and insulation. Fix.
        }
    }
    



    if (pl[p].clean_image) {
        pl[p].current_image = "pics/human.png";
        pl[p].clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action

    utility_function();
    if (!pl[p].move_already && pl[p].general_search_called) {
        general_search_walk("");//ensures this function only executes once per update and also only after all other move_to's have been considered. This prioritizes intentional movement (moving to a target) rather than random movement
    }

    pl[p].found_messages.clear();
    pl[p].search_results.clear();

    //check to ensure that spouses share campsite
    if (!pl[p].sex && !pl[p].adopt_spouse_campsite && pl[p].spouse_id != -1) {
        pl[p].campsite_pos = pl[p_by_id(pl[p].spouse_id)].campsite_pos;
    }

    pl[p].move_already = false;
    pl[p].general_search_called = false;

    authority_calc();
    build_monument();
    if (hour_count == 0) {//once a day
        for (auto& i : pl[p].dispositions) {//dispositions tend toward returning to neutral over time if no other factor involved
            if (i.second > 0) {
                change_disposition(i.first, -1, "neutral drift");
            }
            else if (i.second < 0) {
                change_disposition(i.first, 1, "neutral drift");
            }
        }
    }
}

bool People::valid_position(Position pos) {
    bool valid_x = 0 <= pos.x && pos.x < Environment::map_x_max;
    bool valid_y = 0 <= pos.y && pos.y < Environment::map_y_max;
    if (valid_x && valid_y) {
        return true;
    }
    return false;
}

//need to fix any relevant function that calls move_to() such as general_search/random_walk so as to avoid going near disliked people and avoid even more and at greater distance any hated people.
//  otherwise everyone bunches up together because everyone likes someone and so they are only pull factors and no push factors in where people move_to. The only push factor right now that sort of works is campsite placement near friends.
//need to fix so that diagnonal movement is the same speed as non-diagonal movement
bool People::move_to(Position dest, string caller) {//need to add speed of moving from one tile to another and how many tiles at a time. Also need to add a check to prevent it being called more than once per person per update.
    bool reached = false;

    if (pl[p].pos != pl[p].last_position) {
        pl[p].last_position = pl[p].pos;
    }//unsure if this is needed or if old_pos works

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
        //cout << pl[p].id << " - " << caller << "\n";
        old_pos = pl[p].pos;
        new_pos = pl[p].pos;
        if (new_pos.x < dest.x) {//for future optimization, see: https://stackoverflow.com/questions/14579920/fast-sign-of-integer-in-c
            new_pos.x++;
        }
        else if (new_pos.x > dest.x) {
            new_pos.x--;
        }

        if (new_pos.y < dest.y) {
            new_pos.y++;
        }
        else if (new_pos.y > dest.y) {
            new_pos.y--;
        }

        if (!(Environment::Map[new_pos.y][new_pos.x].person_id == -1)) {//if a person is blocking path, move to first empty tile in a clockwise direction
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
        pl[p].dest = new_pos;
        Environment::Map[new_pos.y][new_pos.x].person_id = -2;//reserve tile so that no other person tries to enter it
    }

    
    old_pos = pl[p].pos;
    new_pos = pl[p].dest;

    //fix this, need to adjust such that A. the change in px position adjusts to any changes in zoom level to maintain same movement rate and B. if speed is larger than 16, then speed must also affect movement of tiles per tick but ensuring collisions in the path are still taken into account
    if (old_pos.x<new_pos.x) {//fix this, later need to modify to tie to new tile when it hits the midway point between 2 tiles
        pl[p].px_x += 1 * pl[p].speed;
        
        pl[p].mov = true;
        if (pl[p].px_x == (sqdim1-1)) {
            pl[p].mov = false;
            pl[p].px_x += 1;
            pl[p].pos.x++;
            pl[p].px_x = 0;
            //pl[p].px_x *= -1;
        }
    }
    else if (old_pos.x > new_pos.x) {
        pl[p].px_x -= 1 * pl[p].speed;
        
        pl[p].mov = true;
        if (pl[p].px_x == -1*(sqdim1 - 1)) {
            pl[p].mov = false;
            pl[p].px_x -= 1;
            pl[p].pos.x--;
            pl[p].px_x = 0;
            //pl[p].px_x *= -1;
        }
    }
    if (old_pos.y > new_pos.y) {
        pl[p].px_y -= 1 * pl[p].speed;
        
        pl[p].mov = true;
        if (pl[p].px_y == -1* (sqdim1 - 1)) {
            pl[p].mov = false;
            pl[p].pos.y--;
            pl[p].px_y -= 1;
            pl[p].px_y = 0;
            //pl[p].px_y *= -1;
        }
    }
    else if (old_pos.y < new_pos.y) {
        pl[p].px_y += 1 * pl[p].speed;
       
        pl[p].mov = true;
        if (pl[p].px_y == (sqdim1 - 1)) {
            pl[p].mov = false;
            pl[p].pos.y++;
            pl[p].px_y += 1;
            pl[p].px_y = 0;
            //pl[p].px_y *= -1;
        }
    }

    if (pl[p].pos == new_pos) {
        Environment::Map[old_pos.y][old_pos.x].person_id = -1;//remove person from Map
        Environment::Map[pl[p].pos.y][pl[p].pos.x].person_id = pl[p].id;//add person back to Map at new location
    }
    reached = pl[p].pos == dest;

    if (pl[p].pos != pl[p].last_position) {
        int x = 0;
        int y = 0;
        if (pl[p].pos.x < pl[p].last_position.x) {
            x = -1;
        }
        else if (pl[p].pos.x > pl[p].last_position.x) {
            x = 1;
        }
        if (pl[p].pos.y < pl[p].last_position.y) {
            y = -1;
        }
        else if (pl[p].pos.y > pl[p].last_position.y) {
            y = 1;
        }
        Position dir = { x,y };
        create_tracks(dir);
    }
    return reached;
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

int People::p_by_id(int id){//uses binary search to find and return index to person in people list (pl)
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

bool People::child_birth() {
    if (pl[p].sex) {
        return false;//am male
    }
    if (pl[p].pregnancy.progress == 0) {
        return false;//not pregnant
    }
    if (pl[p].pregnancy.progress_func()) {//advance pregnancy until done, if done create child
        int sex = rand() % 2;
        Position child_pos;
        if (pl[p].search_results.find("no people") != pl[p].search_results.end()) {
            if (Position::distance(pl[p].search_results["no people"][0], pl[p].pos) == 1) {//if empty adjacent tile
                child_pos = pl[p].search_results["no people"][0];
            }
        }
        if (child_pos.x == -1) {//if no empty adjacent tile found
            pl[p].general_search_called = true;
            return true;//in progress
        }
        Person child = { new_person_id(), child_pos, sex };
        Environment::Map[child.pos.y][child.pos.x].person_id = child.id;
        pl.push_back(child);
        pl[p].children_id.push_back(child.id);
        pl[p_by_id(pl[p].spouse_id)].children_id.push_back(child.id);
        return true;//done
    }
}

bool People::reproduce() {//later, add marriage ceremony/customs, options for polygamy, infidelity, premarital sex, widow status, age and family restrictions on potential mates, family size limits, divorce, etc
    if (!(pl[p].reproduction_cooldown > REPRODUCTION_TRIGGER)) {//function trigger
        return false;
    }
    add_func_record("reproduce");
    vector<Position> &pos_list1 = pl[p].search_results["people"];//note: using reference (&) reduces copying
    int p2 = -1;
    for (int i = 0; i < pos_list1.size(); i++) {//filter out valid mates from people found list
        int pers_id = Environment::Map[pos_list1[i].y][pos_list1[i].x].person_id;
        if (pers_id == -2) {
            cout << "error: pid==-2\n";//don't know why this is happening, already checked find_all() but it's the only place it could be inserted
            return true;//try again
        }
        int pid = p_by_id(pers_id);
        if (pl[pid].sex != pl[p].sex && pl[pid].age > MIN_ADULT_AGE && pl[pid].is_alive) {
            bool is_my_child = false;
            for (int i = 0; i < pl[p].children_id.size(); i++) {
                if (pl[p].children_id[i] == pl[pid].id) {
                    is_my_child = true;
                    break;
                }
            }
            if (!is_my_child && ((pl[pid].spouse_id == -1 && pl[p].spouse_id == -1) || (pl[pid].spouse_id == pl[p].id && pl[p].spouse_id == pl[pid].id))) {//if not my child AND both unmarried or if married to each other
                p2 = p_by_id(pers_id);//mate found
                break;
            }
        }
    }
    bool mate_willing = false;
    if (p2!=-1) {
        if (pl[p2].reproduction_cooldown > REPRODUCTION_TRIGGER && pl[p2].sex != pl[p].sex) {
            mate_willing = true;
        }
        if (mate_willing && (Position::distance(pl[p].pos, pl[p2].pos)==1 || move_to(pl[p2].pos,"going to mate"))) {//go to tile adjacent to p2
            //create a new human, add pregnancy later, only female creates child
            if (!pl[p].sex) {//if female
                pl[p].pregnancy.progress = 1;//am now pregnant
                pl[p].reproduction_cooldown = 0;//reset
                pl[p2].reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                if (pl[p].spouse_id == -1 && pl[p2].spouse_id == -1) {//if don't have spouse, set as spouse
                    pl[p].spouse_id = pl[p2].id;
                    pl[p2].spouse_id = pl[p].id;
                }
                //remove campsite and adopt male's campsite as own.
                pl[p].adopt_spouse_campsite = true;
                return true;//male will simply no longer call reproduce() given the cooldown==0, so only female needs to return true
            }
            else {
                //if male, simply wait for female to update one's reproduction_cooldown
            }
        }
    }
    else {//if no mate found, walk to search
        pl[p].general_search_called = true;
    }
    return true;//in progress
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
    if(pl[p].radiusmax == -1){//used to store result instead of calling every time, only resets if one of the radius options changes such as damaged eyesight, etc
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
                vector<string> types = { "item","animal","terrain", "tracks", "temperature"};
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
        if (xmax > Environment::map_x_max-1) { xmax = Environment::map_x_max-1; }
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
                    vector<string> types = { "people","item","animal","terrain", "tracks", "temperature"};
                    for (string s : types) {
                        find_all_helper(pos1, s);
                        if (y <= ymax && pos1!=pos2) {//need to figure out why pos1 sometimes == pos2 and rewrite for loops to avoid this
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

//can this function also be folded into the find_all function somehow to further reduce for loops searching on the map?
//might also need to add some restriction so that each person doesn't blast out too many messages at once? Or maybe that's ok?
void People::speak(string message_text, int receiver_id) {//if receiver_id == -1, then the message is for everyone
    //current valid messages include: need to list valid messages here
    //the outward ring method might make more sense in this function to allow certain objects such as walls to block sound, might implement later but not currently
    Message m = { new_message_id(), pl[p].id, receiver_id, message_text, pl[p].pos };//creates message
    for (int y = pl[p].pos.y - pl[p].audioline_radius; y < pl[p].pos.y + pl[p].audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = pl[p].pos.x - pl[p].audioline_radius; x < pl[p].pos.x + pl[p].audioline_radius; x++) {
            if (valid_position({ x,y })) {
                Message_Map[y][x].push_back(m.message_id);
                message_list.push_back(m);
            }
        }
    }
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

People::Position People::make_position_valid(Position dest, int ux, int lx, int uy, int ly) {//bounds: u==upper, l==lower
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

//fix this, this function has magic numbers but they are random percents, need to decide how to handle them
bool People::idle() {
    add_func_record("idle");
    int option = rand() % 100;
    if (pl[p].dispositions.empty()) {
        option = 51;
    }
    bool have_no_friends = false;
    do {
        if (option > 50) {
            //flip between idle and default image
            if (pl[p].current_image == "pics/human.png") {
                pl[p].current_image = "pics/human_idle.png"; //need to make image, just have human with raised hands
            }
            else {
                pl[p].current_image = "pics/human.png";
            }
            have_no_friends = false;
        }
        else {//should this instead be a social need or is it good here in idle?
            int sum = 0;
            for (auto const& i : pl[p].dispositions) {
                if (i.second > 0) {
                    sum += i.second;
                }
            }
            int dice = (rand() % (sum + 1)) - 1;//+1,-1 is to avoid division by 0
            sum = 0;
            int p2id = -1;
            for (auto const& i : pl[p].dispositions) {//selects a person one likes with higher chance the more someone is liked, to go to socialize with
                if (i.second > 0) {
                    sum += i.second;
                    if (sum > dice) {
                        p2id = i.first;
                        break;
                    }
                }
            }
            if (p2id != -1) {
                move_to(pl[p_by_id(p2id)].pos, "to socialize - idle");//to search out liked people to encourage clique congregation and disposition sharing
            }
            else {
                have_no_friends = true;//causes execution of other idle option
                option = 51;
            }
        }
    } while (have_no_friends);
    return true;
}

int People::Position::distance(Position pos1, Position pos2) {
    int xd = abs(pos1.x - pos2.x);
    int yd = abs(pos1.y - pos2.y);
    int max = -1;
    (xd > yd) ? max = xd : max = yd;
    return max;
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

bool People::craft(string product) {//later add station requirements such as campfire/stove/oven/furnace
    //if inventory has product.ingredients then craft product (consumes non tool ingredients) and place in inventory
    int num_of_ingredients = it2.presets[product].ingredients.size();
    vector<string> missing_ingredients;//fix this, this should be a single string given that every update rechecks if it can craft the item and only 1 ingredient is sought per tick. Unless a more detailed method is implemented such as checking if any missing ingredient is craftable rather than doing them strictly in order.
    vector<int> temp;
    for (int i = 0; i < num_of_ingredients; i++) {
        temp = inventory_has(it2.presets[product].ingredients[i]);
        if (temp.empty()) {
            missing_ingredients.push_back(it2.presets[product].ingredients[i]);
        }
    }
    if (missing_ingredients.empty()) {//have all items, therefore craft product
        add_func_record("crafting " + product);
        pl[p].current_image = "human_crafting";
        //pl[p].immobile = true;//prevents moving while crafting?? doesn't work?
        if (pl[p].crafting.find(product) == pl[p].crafting.end()) {//key not found
            pl[p].crafting.insert({ product,{4} });//4 is the number of ticks/frames crafting image/animation lasts
        }
        if (pl[p].crafting[product].progress_func()) {//animation/time delay    currently, progress is saved if interrupted which might not make sense in some contexts
            for (int i = 0; i < num_of_ingredients; i++) {//later implement tool degradation here as well
                if (it2.presets[it2.presets[product].ingredients[i]].consumable_ingredient) {
                    int consume_index = inventory_has(it2.presets[product].ingredients[i])[0];
                    delete_item(pl[p].item_inventory[consume_index], { -1,-1 }, consume_index);
                }
            }
            create_item(product, { -1,-1 });
            pl[p].crafting.erase(product);
            pl[p].clean_image = true; //when this function ends, return to default image on next update
           // pl[p].immobile = false;
            return true;//done
        }
        return false;//in progress
    }
    else {
        if (acquire(missing_ingredients[0])) {
            return false;//successfully acquired ingredient, try crafting again next update
        }
        else {
            return false;//acquisition of ingredient in progress, continue to next tick
        }
    }
}

void People::general_search_walk(string target) {
    //walk to search
    if (pl[p].general_search_dest.x == -1 || move_to(pl[p].general_search_dest, "general searching")) {//initialize function object or reinitialize if reached destination
        pl[p].general_search_dest = walk_search_random_dest();
    }//the move_to function triggers in the conditional
}

bool People::answer_item_request() {
    if (pl[p].found_messages.empty()) {
        return false;
    }
    //if request for an item found, and have item, and willing to give item (don't give food if hungry or only to kids if kids are hungry, etc)
    vector<Message> request_messages;
    vector<string> items_requested;
    for (int m_id : pl[p].found_messages) {
        if (message_list[message_by_id(m_id)].messsage.find("requesting ") != -1) {//checks if messages contains substring "requesting". -1 == not found
            request_messages.push_back(message_list[message_by_id(m_id)]);
            string item = message_list[message_by_id(m_id)].messsage;
            string sub_s = "requesting ";
            item.erase(item.find(sub_s), sub_s.length());//erases substring from message, leaving only the item name
            items_requested.push_back(item);
        }
    }
    if (items_requested.empty()) {//none of the messages were item requests
        return false;
    }
    //choose message to answer  //answering item requests. if answered and whether to answer. If like person or they have authority or I submit to them, then chance of if they request something I don't have, I acquire item in order to give it to them
    bool acquire_to_give = false;
    int service=-1;
    for (int i = request_messages.size() - 1; i > -1; i--) {
        int sid = request_messages[i].sender_id;
        change_disposition(sid, 0, "check exists");
        if (pl[p].submissive_to.find(sid) == pl[p].submissive_to.end()) { pl[p].submissive_to.insert({ sid,{0,false} }); }//should have a first contact function to populate these lists instead of checking if exists every time the list is used. Fix this
        if (pl[p].dispositions[sid] < DISLIKE_THRESHOLD && !pl[p].submissive_to[sid].submissive_to) {//don't answer requests from disliked people unless you are submissive to them
            request_messages.erase(request_messages.begin() + i);
            items_requested.erase(items_requested.begin() + i);
        }
        if (pl[p].dispositions[sid] > LOVED_THRESHOLD || pl[p].submissive_to[sid].submissive_to) {
            acquire_to_give = true;
            service = i;
            break;//force answer this request
        }
    }
    if (items_requested.empty()) {//requests were from disliked and non dominant people
        return false;
    }
    if (!acquire_to_give) {
        for (int i = items_requested.size() - 1; i > -1; i--) {
            if (inventory_has(items_requested[i]).empty()) {//if don't have item requested, remove from list of items requested
                items_requested.erase(items_requested.begin() + i);
                request_messages.erase(request_messages.begin() + i);
            }
        }
        if (items_requested.empty()) {//don't have any of the items requested
            return false;
        }
    }

    //conditional checks such as don't give food if self is hungry or give to kids before others if kids are hungry. Not implemented, fix this
    /*
    * bool hungry = pl[p].hunger_level > 50;//condition on willingness to answer request
      bool start_give_food = !hungry && !pl[p].child_is_hungry;
    */
    


    //send answer
    int m_ind = 0;
    if (acquire_to_give) {
        m_ind = service;
        if (acquire(items_requested[service])) {
            //continue
        }
        else {
            return true;//in process
        }
    }
    string target = items_requested[m_ind];//currently simply selects the first item request in list to answer. Fix this, no condition on when or when not to answer has been implemented
    int receiver_id = request_messages[m_ind].sender_id;//id of person who requested the item
    add_func_record("answering request for " + target);
    pl[p].current_image = "pics/human_giving_food.png";
    speak("answering request for " + target, receiver_id);
    //move to requester's position, adjacent
    Person& p2 = pl[p_by_id(receiver_id)];
    if (Position::distance(pl[p].pos, p2.pos) == 1 || move_to(p2.pos, "answering request")) {
        //once reached, place requested item in their inventory
        int index = inventory_has(target)[0];
        p2.item_inventory.push_back(pl[p].item_inventory[index]); //give item id from inventory
        pl[p].item_inventory.erase(pl[p].item_inventory.begin() + index);//remove from own inventory
        pl[p].clean_image = true;
        int op = p;//temp reassign main p
        p = p_by_id(receiver_id);
        change_disposition(pl[op].id, 10, "recieved item requested");
        p = op;
        return true;//done
    }
    return true;//in process
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
vector<People::Position> People::filter_search_results(string target) {     
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

bool People::acquire(string target) {
    if (!inventory_has(target).empty()) {
        //cout << ItemSys::item_list[ItemSys::item_by_id(inventory_has(target)[0])].item_name << "\n";
        return true;
    }
    
    //add_func_record("acquiring " + target);
    //check if target is an item name or item tag
    if (it2.presets.find(target) != it2.presets.end()) {
        //target is an item name
        if (target == "wood") {
            if (cut_down_tree()) {//do/fix this: might make sense to make tree a consumable station, an activity that must be performed at a specific spot (next to a tree) but the spot is consumed at the end (tree turned into wood)
                return true;//done
            }
            return false;//in progress
        }
    }
    else if (anim1.species_names.find(target) != anim1.species_names.end()) {
        //target is an animal name
        if (hunting(target)) {//call hunting function
            return true;//done
        }
        return false;//in progress
    }
    else if (it2.tags.find(target) != it2.tags.end()) {
        //target is a tag name
        for (string item_name : it2.tags[target]) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
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

    //if item is craftable, craft it but if in the process of crafting, the item is found, abort crafting the item
    if (!it2.presets[target].ingredients.empty()) {//if has ingredients, then is craftable
        if (craft(target)) {
            return true;//crafting item was successful
        }
    }
    //if crafting item not yet successful or item is not craftable, look around self for item
    if (pl[p].search_results.find(target) != pl[p].search_results.end()) {//key found, if key exists then at least 1 was found
        Position pos = pl[p].search_results[target][0];
        int item_id = Environment::Map[pos.y][pos.x].item_id;
        if (move_to(pos,"found item"+target)) {//if item is found, move to it and pick it up
            pick_up_item(item_id, pos);
            return true;//item picked up
        }
        return false;//if still moving towards item, continue to next tick
    }
    //if item not found, and people nearby, request item
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {

        bool request_answered = false;
        int answerer_id = -1;
        if (!pl[p].found_messages.empty()) {
            for (int m_id : pl[p].found_messages) {
                Message& m = message_list[message_by_id(m_id)];
                if (m.reciever_id == pl[p].id && m.messsage == "answering request for " + target) {
                    request_answered = true;
                    answerer_id = m.sender_id;
                    break;
                }
            }
        }
        //if request answered, stop requesting and move toward answerer
        if (request_answered) {//Due to sequence ordering issues of NPC updates, need to remember message for a bit to avoid missing messages from NPCs that update after self.
            Position pos = pl[p_by_id(answerer_id)].pos;
            if (Position::distance(pos, pl[p].pos) == 1 || move_to(pos,"recieving item")) {//move to adjacent to answerer
                return false;//wait for answerer to place item in one's inventory (acquire() won't be called next update if was given requested item)
            }
            else {
                return false;//moving toward answerer, in progress
            }
        }
        else {//broadcast request for item
            speak("requesting " + target, -1);
        }
    }
    //if all fails, move in search pattern. Search pattern is shared, to reduce erratic movement from various instances of search patterns
    pl[p].general_search_called = true;
    return false;//searching
}

void People::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases
    
    /*
    Every goal has a method to satisfy alone or cooperatively or worrying about another person having their goals satisfied such as children.

safety - need to add need and function, related to fight function
social - other than seeking social status and mood hits from loneliness, this might be unnecessary given other functions, related to chat and idle.socialize and reproduce and child_birth and need to feed infants
social status - need to add need and function, do this last as it's more complex. Isn't this also a bit redundant with social need?
health - improve this, need triggers for illness and injury
    */
    
    if (pl[p].animal_id_ambush != -1) {
        Animal::animal& a = Animal::al[anim1.a_by_id(pl[p].animal_id_ambush)];
        if (Position::distance({a.pos.x, a.pos.y},pl[p].pos) > pl[p].sightline_radius) {//if lost sight of prey being ambushed, return to regular speed and no stealth
            pl[p].animal_id_ambush = -1;
            pl[p].speed = WALK_SPEED;
            pl[p].in_stealth = false;
        }
        else {
            hunting(a.species);//continue to hunt this type of animal. Need to change to specifically this target, also need to allow interruption by high priority functions such as being attacked/injured/etc
        }
    }
    
    
    
    chat();//chance to chat every update
    if (pl[p].mov) {
        move_to(pl[p].dest, "continue");
        return;
    }
    if(need_light()){}
    else if(child_birth()){}//pregnancy advancement should be moved to update function, not child_birth()
    else if (fight()) { }
    else if(health()){}
    else if(exposure()){  }
    else if (sleeping()) {  }//need to move collapsing from sleep to update function instead of sleeping()
    else if(drinking()){  }
    else if (eating()) { }//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    else if (search_for_new_campsite()) {  }
    else if (reproduce()) {  } //avoid execution of this function to focus on other features without worrying about population size
    else if(answer_item_request()){  }
    else if(hygiene()){  }
    else if(recreation()){  }
    else if(beauty()){  }
    else {idle(); }
    //DO THIS: (this (authority pursuit AI) might be too complex for this version, maybe organic leaders is better and add behavior that makes it likelier for some npcs to get to victory condition?) need to add authority as a need/goal to be pursued. Which means starting and winning fights with new people to increase number of submissives, and gaining favor with more people and increasing favor with existing friends/allies
}

//continue improving this
bool People::health() {
    if (!pl[p].am_injured && !pl[p].am_sick) {//sickness and injury need triggers
        return false;
    }

    //if sick, get medicine and rest/sleep
    if (pl[p].sick_time > 100) {//very sick, go to bed and be bedridden
        pl[p].tired_level = SLEEP_TRIGGER;
        sleeping();
        return true;//bedridden or going to bed
    }
    else if (pl[p].sick_time > 20) {//slightly sick, get medicine
        if (!acquire("medicine")) {
            return true;//getting medicine
        }
        else {
            //if have medicine, heal self and consume medicine
            delete_item(inventory_has("medicine")[0], { -1,-1 }, 0);
            pl[p].am_sick = false;
            pl[p].sick_time = 0;
        }
    }

    //if injured, get bandage and medicine and rest/sleep
    if (pl[p].injured_time > 100) {//very injured, go to bed and be bedridden
        pl[p].tired_level = SLEEP_TRIGGER;
        sleeping();
        return true;//bedridden or going to bed
    }
    else if (pl[p].injured_time > 20) {//slightly injured, get bandage
        if (!acquire("bandage")) {
            return true;//getting bandage
        }
        else {
            //if have bandage, heal self and consume bandage
            delete_item(inventory_has("bandage")[0], { -1,-1 }, 0);
            pl[p].am_injured = false;
            pl[p].injured_time = 0;
        }
    }
    //if can't help self, request help

    //need triggers for illness and injury. Both contagious and not. Each symptom or illness should have a different medicine.
}

//DO THIS later
bool People::need_light() {
    return false;
    //if too dark, either move to light or create or get light source such as candle/torch/campfire
}

bool People::exposure() {//protection from heat/sun and cold/wind/rain. Only temperature protection for now.
    if (pl[p].my_temperature == pl[p].my_preffered_temperature) {//fix this, needs to be a distance from ideal temp, not exactly ideal temp
        return false;//temperature is fine
    }
    bool better_temp_found = false;
    if (pl[p].my_temperature > pl[p].my_preffered_temperature) {//need to add impact on thirst by heat in update function
        //seek cold, am too hot
        for (auto const& i : pl[p].search_results) {
            if (stoi(i.first) < pl[p].my_temperature) {//the only keys that are numbers will be tile temperatures, so find the first tile that has a lower temperature and go there
                move_to(i.second[0], "towards cold");
                better_temp_found = true;
                return true;
            }
        }
        if (!better_temp_found) {
            //take off clothes //what about sunburn? also add natural tanning mechanic
            for (auto& i : pl[p].equipped.equipment) {
                int item_id = pl[p].equipped.unequip(i.first);//remove one item of clothing
                if (item_id != -1) {
                    continue;
                }
                else {
                    pl[p].item_inventory.push_back(item_id);//place unequipped item in inventory
                    return true;
                }
            }
            //implement these 3 later
            //seek darkness/shade
            //go in water
            //move less, be lazy
        }
        //if no method to improve temp, search
        pl[p].general_search_called = true;
    }
    else if (pl[p].my_temperature < pl[p].my_preffered_temperature) {
        //seek heat, am too cold
        for (auto const& i : pl[p].search_results) {
            if (stoi(i.first) > pl[p].my_temperature) {//inverse of "towards cold"
                move_to(i.second[0], "towards heat");
                better_temp_found = true;
            }
        }
        if (!better_temp_found) {
            //put on clothes and blankets
            if (!inventory_has("clothes").empty()) {
                //add one item of clothing. 
                for (auto const& i : pl[p].equipped.equipment) {
                    if (i.second == -1) {
                        if (!inventory_has(i.first).empty()) {//if inventory has item with a tag saying it can be worn in this slot
                            pl[p].equipped.equip(inventory_has(i.first)[0]);
                            return true;
                        }
                    }
                }
            }
            //start or find fire
            if (pl[p].search_results.find("campfire") == pl[p].search_results.end()) {//if there is a campfire, it will warm the area and so is handled by "move to heat"
                //if there is no campfire nearby
                if (!acquire("campfire")) {
                    pl[p].general_search_called= true;//searching for fire
                }
                else {
                    drop_item(inventory_has("campfire")[0]);//place campfire
                }
            }

            //implement these 2 later
            //seek light
            //move more, be active
        }
        //if no method to improve temp, search
        pl[p].general_search_called = true;
    }
    //if am wet or in rain or in strong wind, seek shelter. Need to implement these. Fix this
    return false;//?
}

bool People::hygiene() {
    if (pl[p].dirtiness < HYGIENE_TRIGGER) {
        return false;
    }
    if (pl[p].search_results.find("water") != pl[p].search_results.end()) {//go to water source
        if (move_to(pl[p].search_results["water"][0], "to water")) {//run bathing animation
            pl[p].current_image = "bathing";
            if (pl[p].bathing.progress_func()) {
                pl[p].dirtiness = 0;
                pl[p].clean_image = true;
                return true;//done
            }
        }
    }
    else {
        pl[p].general_search_called = true;
    }
    return true;//in progress
}

bool People::beauty() {
    if (pl[p].beauty_need_satisfied) {
        return false;
    }
    //decorate self and surroundings

    //FIX THIS: need to implement decorative items and bring them to camp or jewelry and craft it

    //temporary implementation, like recreation. Both of these functions are more about polish and content rather than mechanics so do last
    if (acquire("necklace")) {
        pl[p].beauty_need_satisfied = true;
    }
}

//games/dance/stories/song/instruments/sports/competitions/socializing/exploring/swimming/painting rock walls
bool People::recreation() {
    if (!(pl[p].recreation_level > RECREATION_TRIGGER)) {//unsure if this needs a trigger or simply replace idle? and have negative mood if not fulfilled?
        return false;
    }
    //need to add method of choosing what to do to recreate, should desire variety as well
    string option = "play trumpet";
    if (option == "play trumpet") {//need to first get instrument, fix this. Also need to create instrument item
        if (acquire("trumpet")) {
            pl[p].current_image = "playing trumpet";
            pl[p].recreation_level -= 50;//each recreation type should have its own reduction value?
            pl[p].clean_image = true;
        }
    }
}

void People::pickup_infants() {
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

//need to simplify this function.
bool People::search_for_new_campsite(){ //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
    if (!pl[p].sex && pl[p].spouse_id!=-1 && pl[p].campsite_pos == pl[p_by_id(pl[p].spouse_id)].campsite_pos) {
        return false;//prevent searching for a new campsite if married, only for females
    }
    
    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= DAYS_HUNGRY_MOVE_CAMP;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > NEW_CAMP_PROBATION_TIME || pl[p].campsite_age==-1;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age
    bool start = (cond2 && cond3) || pl[p].adopt_spouse_campsite;
    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start){
        return false;
    }


    if (pl[p].campsite_pos.x != -1) { //if have campsite, remove. //Later add an option to just abandon a campsite without removing the house. Should only decontruct if going to carry it to new location such as a tent/sleeping bag/lean to/etc.
        if (move_to(pl[p].campsite_pos, "to campsite")) {//walk to campsite to remove
            int item_id = Environment::Map[pl[p].campsite_pos.y][pl[p].campsite_pos.x].item_id;
            delete_item(item_id, pl[p].campsite_pos, -1);
            pl[p].campsite_pos = { -1,-1 };
            pl[p].campsite_age = -1;
            pl[p].friend_camp_check = true;
            if (pl[p].adopt_spouse_campsite) {
                pl[p].campsite_pos = pl[p_by_id(pl[p].spouse_id)].campsite_pos;
                pl[p].adopt_spouse_campsite = false;//reset for if spouse dies later
                return true;//in progress
            }
        }
        else {
            return true;//in progress
        }
    }
    //if have infants, carry them
    pickup_infants();

    vector<Position> food_pos_list = filter_search_results("food"); //gets results, assigns 1 result to food_pos
    Position food_pos = { -1,-1 };
    bool found_food = false;
    if (!food_pos_list.empty()) {
        food_pos = food_pos_list[0];
        found_food = true;
    }

    if (pl[p].friend_camp_check) {//encourages campsite congregation between people who like each other (forgot to check if other person likes self, fix this)
        for (auto const& i : pl[p].dispositions) {
            if (i.second > LOVED_THRESHOLD) {
                if (pl[p_by_id(i.first)].campsite_pos.x != -1) {//if have a friend with a campsite and am finding a new campsite, move to within 10 tiles of friend to search for campsite location there
                    if (Position::distance(pl[p].pos, pl[p_by_id(i.first)].campsite_pos) < NEW_CAMP_CLOSE_TO_FRIEND || move_to(pl[p_by_id(i.first)].campsite_pos, "to friend's campsite")) {
                        pl[p].friend_camp_check = false;
                    }
                    else {
                        return true;//in progress
                    }
                }
            }
        }
    }

    if (food_pos_list.size() >= 4) {//if there are 4 food items within sight, select area for campsite, else keep searching
        add_func_record("set up camp");
        pl[p].campsite_age = 0; //resets campsite age
        //place tent
        vector<Position> pos_list = pl[p].search_results["no item"];
        if (!pos_list.empty()) {
            create_item("tent", pos_list[0]);//create and place tent
            pl[p].campsite_pos = pos_list[0]; //store campsite location
            
            //if have infants, drop them
            if (!drop_infants()) {
                return true;//in progress
            }
            
            return true; //done. need to add an actual animation/progress to building the tent rather than immediate placement
        }
        else {
            pl[p].general_search_called = true;
            return true;//in progress
        }
        return true;//done
    }
    else if (!food_pos_list.empty()) {
        //need to add a method of investigating if any food found might have more food just out of current sightline, but this probably will require more complex modifiable pathfinding, as in have it be a detour from the current destination rather than a change in destination.
    }

    pl[p].general_search_called = true;
    return true;//in progress
}

bool People::sleeping(){
    bool tired = pl[p].tired_level > SLEEP_TRIGGER;
    bool start_moving_to_bed = pl[p].awake && tired && pl[p].campsite_pos.x != -1 && pl[p].hunger_level<HUNGRY_LEVEL && pl[p].thirst_level<THIRSTY_LEVEL;
    if (start_moving_to_bed) {
        add_func_record("moving to bed");
        if(move_to(pl[p].campsite_pos,"to bed")) { //go to campsite.
            //go to sleep, continue
        }
        else {
            return true;//done and in progress
        }
    }
    bool very_tired = pl[p].tired_level > FORCE_SLEEP_LEVEL;//might need to cap sleep such that a person can't ever have a tired_level over x_level as well as under y_level
    bool cond1 = tired && (pl[p].pos == pl[p].campsite_pos || pl[p].campsite_pos.x == -1);//if tired AND either at campsite or have no campsite
    if (!(!pl[p].awake || cond1 || very_tired)) {//function trigger
        return false;
    }
    add_func_record("sleeping");
    pl[p].current_image = "pics/human_sleeping.png";
    pl[p].awake = false;
    pl[p].tired_level-=SLEEP_REST_RATE; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function
    if (pl[p].tired_level <= 0) {//fix this, need to cap at 0, also need cap for upper limit?
        pl[p].current_image = "pics/human.png";
        pl[p].awake = true;
        return true;//done
    }
    return true;//in progress
}

bool People::eating(){
    if (pl[p].hungry_time > DAYS_HUNGRY_MOVE_CAMP && pl[p].campsite_pos.x != 1 && pl[p].campsite_age>NEW_CAMP_PROBATION_TIME) {//allows search for campsite to trigger but not before new camp is at least a few ticks old
        return false;
    }

    bool hungry = pl[p].hunger_level > HUNGRY_LEVEL;
    bool has_food = false;
    vector<int> food_indexes1 = inventory_has("ready food");//should these return sets instead? would remove the need for converting to sets when set operations are needed. Duplicate indexes are never relevant.
    if (!food_indexes1.empty()) {
        has_food = true;
        pl[p].eating_food_index = food_indexes1[0];
    }

    if ((hungry && !has_food) /* || food_indexes1.size() < MIN_EXTRA_FOOD_IN_INVENTORY*/) {//ensures that person has 2 food items in inventory for self or to share
        if (acquire("ready food")) {
            add_func_record("acquired ready food");
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
    add_func_record("eating");
    if (pl[p].eating_progress.progress == 0) {
        pl[p].current_image = "pics/human_eating.png";
    }
    if (pl[p].age < MAX_INFANT_AGE) {
        pl[p].current_image = "human_infant";
    }
    if (pl[p].eating_progress.progress_func()) {//makes eating take more than 1 frame
        int index = pl[p].eating_food_index;
        int food_id = pl[p].item_inventory[index];
        delete_item(food_id, {-1,-1}, index);//delete food from game
        pl[p].hunger_level -= HUNGER_REDUCTION_RATE; //reduce hungry level by 10, therefore need 2 meals a day to stay at 0 hunger_level average
        pl[p].clean_image = true; //when this function ends, return to default image on next update
        return true;//done eating
    }
     return true;//in progress
}


//continue fixing/improving this
bool People::hunting(string species) {//need to add a percentage success/fail chance to all hunting
    /*DO THIS NOW
        if animal is large then must either get help to carry home, camp next to animal to eat for days, or butcher it and bring home piece by piece
        pack/group hunting surrounding prey
    */
    
    string hunting_method;
    if (species == "deer") {
        hunting_method = "persistence_and_pick_up";
    }
    else if (species == "rabbit") {//need to determine when to use fishing net and when to angle. Maybe according to fish type?
        hunting_method = "trap";

        //need to also add another type of trap, fishing net, and keep track separately
        //fix this, need to add age and lifetime to traps so that they are removed after a certain amount of time if they haven't caught anything. If trap contains prey, then reset trap if reusable.
        if (!pl[p].traps_set.empty()) {//if have traps, check traps
            for (trap_check& t : pl[p].traps_set) {
                if ((pday_count - t.last_day_checked) >= 3) {//check each trap once every other day since it was set
                    if (move_to(t.pos, "to trap")) {
                        t.last_day_checked = pday_count;
                    }
                    break;
                }
            }
        }//checks traps and then continues so as to react if prey is found in trap

    }
    else if (species == "fish") {
        hunting_method = "angling";
    }

    //tracks currently only include footprints, not feces, urine, grazing/hunt remains, sounds, etc.
    //need to first find animal to hunt, if no animal seen then search for tracks, if tracks found then follow tracks
    //need to convert this if conditional into a for loop to check all possible animal names in search results
    if (pl[p].search_results.find(species) == pl[p].search_results.end()) {
        //if no animal found, check for tracks
        if (pl[p].search_results.find("animal tracks")!=pl[p].search_results.end()) {

            //if tracks belong to trappable animals, set traps
            Position track_pos = pl[p].search_results["animal tracks"][0];
            if (Environment::Map[track_pos.y][track_pos.x].track.creature=="rabbit") {//need to make these species checks tag checks on the species instead
                if (pl[p].traps_set.size() < 4) {
                    if (pl[p].search_results.find("no item") != pl[p].search_results.end()) {
                        if (!acquire("trap")) {
                            return false; //in progress
                        }
                        int trap_ind = inventory_has("trap")[0];
                        delete_item(pl[p].item_inventory[trap_ind], { -1,-1 }, trap_ind);
                        create_item("active trap", { -1,-1 });
                        trap_ind = inventory_has("active trap")[0];
                        if (!drop_item(trap_ind)) {
                            pl[p].general_search_called = true;
                            return false;//in progress
                        }
                        pl[p].traps_set.push_back({ pl[p].dropsite, pday_count });
                        pl[p].dropsite = { -1,-1 };
                        return false;//in progress
                    }
                }
            }

            //tracks should hold direction info so hunter knows what direction to follow
            Position dir = { pl[p].search_results["animal tracks"][0].x,pl[p].search_results["animal tracks"][0].y};
            if (move_to({pl[p].pos.x+dir.x,pl[p].pos.y + dir.y }, "following tracks")) {
                //do nothing else, simply follow tracks until prey is found
            }
            return false;//tracking in progress
        }
        else if (hunting_method=="angling") {
            //continue to angling
        }
        else {
            pl[p].general_search_called = true;
            return false;//no animals or tracks found, search elsewhere
        }
    }

    //FIX THIS. search_results should already be sorted, therefore move sort function to the end of find_all to sort all results
    vector<Position> pos_list = pl[p].search_results[species];
    if (!pos_list.empty()) {//second check due to angling not requiring having found the target animal
        sort(pos_list.begin(), pos_list.end());//need to sort vector before using or else will get stuck. unsure if sort by current position or by 0,0
        pl[p].search_results[species] = pos_list;
    }
    Animal::animal& a = (hunting_method != "angling") ? anim1.al[anim1.a_by_id(Environment::Map[pl[p].search_results[species][0].y][pl[p].search_results[species][0].x].animal_id)] : Animal::al[0];//the assignment of the first animal in animal list is simply because &a can't be NULL


    //when ranged attack is added, include throwing a net to catch animal as well as regular ranged attack
    //FIX THIS: need to add relevant changes to Animal to make this work
   if (hunting_method == "ambush") {//ambush hunted animals such as chickens (includes spearfishing) (stalk animal, when close enough and still unnoticed, rush to attack and catch or kill)
       if (Position::distance({ a.pos.x, a.pos.y }, pl[p].pos) == STEALTH_DISTANCE || move_to({a.pos.x, a.pos.y}, "to prey - stealth")) {
           pl[p].animal_id_ambush = a.id;
           pl[p].speed = STEALTH_SPEED;
           pl[p].in_stealth = true;
           if (Position::distance({ a.pos.x, a.pos.y }, pl[p].pos) == AMBUSH_DISTANCE || move_to({ a.pos.x, a.pos.y }, "to prey - ambush")) {
               if (pl[p].sprint_stamina == 0) {//0 means sprint cooldown is done
                   pl[p].speed = SPRINT_SPEED;//sprint once close enough to prey
                   pl[p].in_stealth = false;
                   if (move_to({ a.pos.x, a.pos.y }, "to prey - ambush kill")) {
                       //kill prey
                       if (a.is_alive) {
                           a.is_alive = false;//kill prey
                           return false;//in progress
                       }
                       if (!acquire("knife")) {//should be separated in a process corpse function with the option to process in the field or carry home first, fix this
                           
                           return false;//in progress
                       }
                       Environment::Map[a.pos.y][a.pos.x].animal_id = -1;//remove dead animal from map
                       create_item(a.meat_type, { a.pos.x, a.pos.y });//add meat in its place   <- fix this, Might make more sense to have animals contain a list of items they turn into when butchered and go through the list
                       anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a.id));//erase animal from global animal list
                       return true;//done
                   }
               }
           }
       }
       

   }

   //this one is cooperative, so need to implement cooperation first
   // if (hunting_method == "cliff_run") {//chasing prey off cliffs or into pits to kill or catch

   // }

    //include clams for this method to pick up
    //when hunting, should use the means available, if a better method is known but not currently available, then need to set a flag to prepare the better method for the next hunt
    //also need to add fleeing and tiring to deer
    //FIX THIS: need to add "if have ranged weapon, use at distance" as well as the need to get the ranged weapon if knowledge of its crafting is held, unless materials to make ranged weapon are not found in x amount of time or hunger is too great to wait
    if (hunting_method == "persistence_and_pick_up") {//this serves as both persistance hunting and simple gathering as it is simply "move to animal and kill animal". 
        if (pl[p].search_results.find(species) != pl[p].search_results.end()) {
            Animal::animal& a = anim1.al[anim1.a_by_id(Environment::Map[pl[p].search_results[species][0].y][pl[p].search_results[species][0].x].animal_id)];
            if (!acquire("rock")) {
                return false;//in progress
            }
            bool reached = move_to({ a.pos.x,a.pos.y }, "to prey");
            if (reached) {
                if (a.is_alive) {
                    a.is_alive = false;//kill prey
                    return false;//in progress
                }
                if (!acquire("knife")) {
                   return false;//in progress
                }
                Environment::Map[a.pos.y][a.pos.x].animal_id = -1;//remove dead animal from map. Might make more sense to have animal contain body parts and components such as feathers and bones and when all have been removed from corpse, then the corpse is removed.
                create_item(a.meat_type, {a.pos.x, a.pos.y});//add meat in its place   <- fix this, Might make more sense to have animals contain a list of items they turn into when butchered and go through the list
                anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a.id));//erase animal from global animal list
                return true;//done
            }
        }
        else {
            pl[p].general_search_called = true;
            return false;//search for target
        }
    }
 
    //this serves as the trapping method, need to add a bit of variety for if prey is fish or not. FIX THIS
    //need to add fish
    //(includes fishing with nets) (go to area known to have animal, set trap and either wait for trap to spring or come back later to check traps)
    else if (hunting_method == "trap") {//need to handle when to remove traps (could make them one use too) or place them in new spots, especially when either not having a campsite or when moving campsites, fix this
            if (!a.is_alive) {
                if (!acquire("knife")) {//fix this, this should be already handled in acquire(), as in simply call acquire(item) and if it returns true then it's becuase already have item, else it's in process and therefore if false return false in hunting() because an action is in process
                    
                    return false;//in progress
                }
                if (move_to({a.pos.x,a.pos.y}, "to trapped small game")) {
                    Environment::Map[a.pos.y][a.pos.x].animal_id = -1;//remove dead animal from map
                    delete_item(Environment::Map[a.pos.y][a.pos.x].item_id, { a.pos.x,a.pos.y }, -1);//delete active trap
                    create_item(a.meat_type, { a.pos.x,a.pos.y });//add meat in its place
                    anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a.id));//erase animal from global animal list
                    return true;//done
                }
            }
        
     }

    else if (hunting_method == "angling") {
        if (pl[p].active_fish_hook_pos.x != -1) {
            //get fishing rod and bait
            if (!acquire("fishing rod")) {
                return false;//don't have fishing rod, getting it
            }
            if (!acquire("fish bait")) {
                return false;//don't have fish bait, getting it
            }
            //go to water
            if (pl[p].search_results.find("water") != pl[p].search_results.end()) {
                if (Position::distance(pl[p].search_results["water"][0], pl[p].pos) == 1 || move_to(pl[p].search_results["water"][0], "to water")) {
                    //reached water edge, continue angling function
                }
                return false;//moving towards water
            }
            else {
                pl[p].general_search_called = true;
                return false;//search for water
            }
            //throw fishing rod hook into water
            create_item("active fish hook", { -1,-1 });//FIX THIS: need to create fish, fish hook, fish bait, fishing rod, and active fish hook and related fish behavior
            int trap_ind = inventory_has("active fish hook")[0];
            if (!drop_item(trap_ind)) {//if no valid tile to throw hook in, if valid tile then drop hook
                pl[p].general_search_called = true;
                pl[p].active_fish_hook_pos = pl[p].dropsite;
                return false;//in progress
            }
        }
        
        int fish_id = Environment::Map[pl[p].active_fish_hook_pos.y][pl[p].active_fish_hook_pos.x].animal_id;
        if (fish_id == -1) {
            pl[p].time_waited++;//might make more sense to use progress_state instead, fix this
            //pl[p].move_already = true;//don't move this tick, might not work as might move before reaching this line of code. fix this
            if (pl[p].time_waited >= ANGLING_WAIT_TIME) {
                //remove active fish hook
                delete_item(Environment::Map[pl[p].active_fish_hook_pos.y][pl[p].active_fish_hook_pos.x].item_id, pl[p].active_fish_hook_pos, -1);
                pl[p].time_waited = 0;
                pl[p].active_fish_hook_pos = { -1,-1 };
                return false;//did not catch fish
            }
            return false;//waiting for fish to bite
        }
        else {
            Animal::animal& a2 = anim1.al[anim1.a_by_id(fish_id)];
            if (!a2.is_alive) {//something is dead on the hook, caught something. Distance doesn't matter because not moving to animal because it's "reeled in"
                //remove active hook ("reel it in")
                delete_item(Environment::Map[pl[p].active_fish_hook_pos.y][pl[p].active_fish_hook_pos.x].item_id, pl[p].active_fish_hook_pos, -1);
                pl[p].time_waited = 0;
                pl[p].active_fish_hook_pos = { -1,-1 };
                Environment::Map[a2.pos.y][a2.pos.x].animal_id = -1;//remove dead animal from map
                create_item(a2.meat_type, { a2.pos.x,a2.pos.y });//add meat in its place
                anim1.al.erase(anim1.al.begin() + anim1.a_by_id(a2.id));//erase animal from global animal list
                return true;//done
            }
        }
    }
}


//NOTE: for implementing cooperation, conduct through speak() requests and answers. Person 1 proposes joint action, Person 2 decides whether to agree or not. If a 3rd person or more are involved, then need to set a meeting location and time to conduct the proposition -> up/down vote and an option to continue action with those who said yes only. Later add option to be able to coerce those who said no into complying.


bool People::drinking() {
    if (pl[p].thirst_level < THIRSTY_LEVEL && pl[p].drinking.progress==0) {
        return false;
    }
    if (pl[p].search_results.find("water") != pl[p].search_results.end()) {
        if (Position::distance(pl[p].pos, pl[p].search_results["water"][0])==1 || move_to(pl[p].search_results["water"][0], "to water")) {
            pl[p].thirst_level -= THIRST_REDUCTION_RATE;
            pl[p].drinking.progress_done = pl[p].thirst_level / THIRST_REDUCTION_RATE;//drink until practically no longer any thirst
            if (pl[p].drinking.progress_func()) {//drinking delay. add animation
                return true;//done
            }
            return true;//in progress
        }
    }
    else {
        pl[p].general_search_called = true;
    }
    return true;//in progress
}

//integrate this into craft() as a generic that also serves mining stone, obtaining water and milking cows, picking berries off bushes instead of consuming whole bush
bool People::cut_down_tree() {
    if (!inventory_has("wood").empty()) {
        return true;
    }
    if (pl[p].search_results.find("tree") != pl[p].search_results.end()) {
        if (Position::distance(pl[p].pos, pl[p].search_results["tree"][0])==1 || move_to(pl[p].search_results["tree"][0], "to tree")) {
            Position pos = pl[p].search_results["tree"][0];
            int tree_id = Environment::Map[pos.y][pos.x].item_id;
            delete_item(tree_id, pos, -1);
            create_item("wood", pos);
            int wood_id = Environment::Map[pos.y][pos.x].item_id;
            pick_up_item(wood_id, pos);
            return true;//done
        }
        return false;//in progress
    }
    else {
        pl[p].general_search_called = true;
        return false;//searching
    }
}

//this might count as magic numbers but it might make sense to leave them here as it's a simple -100 to 100 range cap
void People::change_disposition(int p_id, int amount, string reason) {
    if (p_id == 0) {
        throw std::invalid_argument("pid is 0");
    }
    if (pl[p].id == p_id) {
        return;
    }
    if (pl[p].dispositions.find(p_id) != pl[p].dispositions.end()) {
        pl[p].dispositions[p_id] = pl[p].dispositions[p_id] + amount;
        if (pl[p].dispositions[p_id] < -100) {//caps dispostion at -100 and 100
            pl[p].dispositions[p_id] = -100;
        }
        else if (pl[p].dispositions[p_id] > 100) {
            pl[p].dispositions[p_id] = 100;
        }
    }
    else {
        pl[p].dispositions.insert({ p_id,amount });
    }
}

//same magic number issue of percent chances, fix this
void People::chat() {//if make this a speak() action, can affect dispositions of more than 1 person with 1 comment, fix this?
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
    int topic = rand() % 100;
    int p2_ind = p_by_id(Environment::Map[pl[p].search_results["people"][0].y][pl[p].search_results["people"][0].x].person_id);
    int p2_id = pl[p2_ind].id;
    if (topic < 50) {//compliment or insult
            int comment = (rand() % 30) - 15;
            if (pl[p].dispositions.find(p2_id) == pl[p].dispositions.end()) {//fix this every instance, these are unnecessary as it is more readable to just call change_disposition with an amount of 0 to do the same (insert key if key not found)
                pl[p].dispositions.insert({ p2_id,0 });
            }
            if (pl[p].dispositions[p2_id] >= 0) {//if like person, make compliment bigger, if like person, make insult smaller. Inverse for dislike person
                if (comment >= 0) {
                    comment = comment * ((pl[p].dispositions[p2_id] / 100) * 2 + 1);
                }
                else {
                    comment = comment * ((pl[p].dispositions[p2_id] / 100) * 0.5 + 1);
                }
            }
            else {
                if (comment >= 0) {
                    comment = comment * ((pl[p].dispositions[p2_id] / 100) * 0.5 + 1);
                }
                else {
                    comment = comment * ((pl[p].dispositions[p2_id] / 100) * 2 + 1);
                }
            }

            int op = p;
            p = p2_ind;
            string valence;//valence should be the int and comment should be the string, switch, fix this, just for clarity

            (comment < 0) ? valence = "insult" : valence = "compliment";
            change_disposition(pl[op].id, comment, valence);//positive is compliment, negative is insult
            p = op;

        }
        else {
            share_disposition(p2_ind);
        }
    }
}

//has a magic number but unsure if it needs to be pulled out as it's only relevant here and might not need to be balanced against other numbers.
void People::authority_calc() {
    int num_people_liked_by = 0; 
    int amount_liked = 0;
    int num_submissives = 0;
    bool am_sovereign = true;
    for (int i = 0; i < pl.size(); i++) {
        if (pl[i].dispositions.find(pl[p].id) != pl[i].dispositions.end()) {
            if (pl[i].dispositions[pl[p].id] > 0) {
                num_people_liked_by++;
                amount_liked += pl[i].dispositions[pl[p].id];
            }
        }
        if (pl[i].submissive_to.find(pl[p].id) != pl[i].submissive_to.end()) {
            if (pl[i].submissive_to[pl[p].id].submissive_to) {
                num_submissives++;
            }
        }
    }
    for (auto const& i : pl[p].submissive_to) {//if one is submissive to anyone, one is not sovereign. Fix this. DO THIS: Need to implement a rebellion function so that when one's authority increases a certain amount or relative to a dominator, can cast off submissive status and become sovereign
        if (i.second.submissive_to) {
            am_sovereign = false;
        }
    }
    
    bool corrected1 = false;
    bool corrected2 = false;
    bool corrected3 = false;
    bool corrected4 = false;
    if (pl[p].num_fights_won == 0) { pl[p].num_fights_won = 1; corrected1 = true; }
    if (num_people_liked_by == 0) { num_people_liked_by=1; corrected2 = true;}//to avoid multiplying by 0
    if (amount_liked == 0) { amount_liked = 1; corrected3 = true;}
    if (num_submissives == 0) { num_submissives = 1; corrected4 = true;}

    pl[p].authority = num_people_liked_by * amount_liked * num_submissives * pl[p].num_fights_won;
    
    bool c = corrected1 || corrected2 || corrected3 || corrected4;
    if (c && pl[p].authority == 1) { pl[p].authority = 0; }//to correct for factors being initialized as 1, not sure if this is the best approach

    if (corrected1) { pl[p].num_fights_won = 0; }//to correct for these being initialized as 1
    if (corrected2) { num_people_liked_by = 0; }
    if (corrected3) { amount_liked = 0; }
    if (corrected4) { num_submissives = 0; }
    pl[p].num_people_liked_by = num_people_liked_by;
    pl[p].amount_liked = amount_liked;
    pl[p].num_submissives = num_submissives;
    
    //10000 = 10*10*10*10 = 1*100*1*1  <-- this may be a problem, rethink formula
    if (pl[p].authority > 10000 && am_sovereign) {
        pl[p].monument_unlocked = true;
        game_won = true;
    }
    else {
        pl[p].monument_unlocked = false;//access to monument building can be lost if authority is lost
    }
}

bool People::hostile_detection() {
    bool found_hostiles = false;
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
        for (Position pos : pl[p].search_results["people"]) {
            Person& p2 = pl[p_by_id(Environment::Map[pos.y][pos.x].person_id)];
            for (int i : p2.active_hostile_towards) {
                if (i == pl[p].id) {//if a person nearby is actively hostile to self
                    pl[p].active_hostile_towards.push_back(p2.id);
                    pl[p].hostile_towards.push_back(p2.id);
                    found_hostiles = true;
                }
            }
        }
    }
    if (found_hostiles) {
        return true;
    }
    else {
        return false;
    }
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

//note: maybe a mechanic to desire to be liked, such that if someone dislikes one, one tries to improve the other's opinion of onself, would help prevent excessive social isolation on a population level
//but this should be either random or personality determined so that some seek out more or less to be liked. 

//has percent magic numbers
//need to implement being downed if lost a personal fight and how to recover, as well as in what cases might be killed or can kill (maybe a percent chance?)
//need to add weapons and armor that affect dice rolls, and triggers for acquiring them
bool People::fight() {//Order of execution for people here might necessitate having an extra tick to resolve results of combat dice rolls, otherwise people who update first will always attack and the latter always defend?
    hostile_detection();//gets active enemies
    pl[p].active_hostile_towards = remove_dup(pl[p].active_hostile_towards);
    pl[p].hostile_towards = remove_dup(pl[p].hostile_towards);
    int fight_choice = -1;
    if (pl[p].hostile_towards.empty()) {//if initiating fight
        //go through every person have disposition towards. If hate someone, chance to trigger fight. //not yet implemented: If fight triggered, chance to invite friends and family to join fight. 
        vector<int> hated;
        for (auto const& i : pl[p].dispositions) {
            if (i.second < HATED_THRESHOLD) {
                hated.push_back(i.first);//get id's
            }
        }
        if (!hated.empty()) {
            fight_choice = rand() % hated.size();//get index
            fight_choice = hated[fight_choice];//get id of enemy
        }
        int chance_to_fight = rand() % 100;

        if (!(chance_to_fight < 5 && fight_choice != -1)) {//function trigger
            return false;//no fight
        }

        //if initiating fight, become hostile towards target
        pl[p].active_hostile_towards.push_back(fight_choice);
        pl[p].hostile_towards.push_back(fight_choice);
    }
    else {
        if (!pl[p].active_hostile_towards.empty()) {
            fight_choice = pl[p].active_hostile_towards[0];//if didn't initiate fight, focus on fighting first active hostile found
        }
    }

    if (!pl[p].active_hostile_towards.empty()) {//active hostiles means fight is active
        pl[p].current_image = "fighting";

        bool won = false;//for individual fights and personal glory
        //DO THIS, later: check combat allies, if a third have been downed or killed, surrender and submit. Currently fights are only 1v1, no combat allies. Multiple enemies or combat allies might appear simply due to hate triggering 1v1 fights but is unintended.
        for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {//for every enemy combatant in the fight
            int p2 = p_by_id(fight_choice);
            bool tie = true;
            
            //combat
            while (tie && (Position::distance(pl[p].pos, pl[p2].pos) == 1 || move_to(pl[p2].pos, "to enemy combatant"))) {//go to target
                if (pl[p].dice == -1) {
                    pl[p].dice = rand() % 20;//when next to target, progress delay and dice roll, currently no progress delay implemented
                    return true;//in progress
                }
                if (p == 0) {
                    cout << "\player fought\n";
                }
                if (pl[p].dice > pl[p2].dice) {//if my dice roll is larger than theirs, I win, else I lose. Tie means keep fighting
                    won = true;
                    tie = false;
                    for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {//win or lose, need to move myself or other from active_hostile to hostile list to no longer be targeted in fight
                        if (pl[p].active_hostile_towards[i] == pl[p2].id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].id) {
                            pl[p2].active_hostile_towards.erase(pl[p2].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                }
                else if (pl[p].dice < pl[p2].dice) {
                    pl[p].downed = true;
                    won = false;
                    tie = false;
                    for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {
                        if (pl[p].active_hostile_towards[i] == pl[p2].id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].id) {
                            pl[p2].active_hostile_towards.erase(pl[p2].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                }
                else {
                    pl[p].dice = -1;
                }
            }
            //can only fight against 1 person at a time meaning being outnumbered personally at the moment is immediate loss of fight
            if (won) {
                pl[p].num_fights_won++;

            }
            else {
                //pl[p].num_fights_won--;   //Fights lost getting subtracted might be causing game to be unwinnable given that the wins and losses average out to 0, preventing win state. Keep them commented out for now.

            }
        }
    }
    
    if (pl[p].active_hostile_towards.empty() && !pl[p].hostile_towards.empty()) {//unsure if this check is redundant. Post fight/battle handling.
        pl[p].clean_image = true;
        bool battle_won = false;
        if (!pl[p].combat_allies.empty()) {
            //check if battle won or lost   not yet implemented
            //do something
            //if in group and group wins fight, get a bit of reflected glory (increase fights won by 1 even if lost personal fights)
        }
        if (pl[p].downed = false) {//for now, battle is won if not downed when no active hostiles left
            battle_won = true;
        }
        else {
            battle_won = false;
        }
        if (battle_won) {
            pl[p].num_fights_won++;
            for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//enemies submit. If I lose then when enemy updates they will modify my own list to submit to them
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost++;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost >= 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].submissive_to = true;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].id,{false,1} });
                }
            }
        }
        else {
            //pl[p].num_fights_won--;
            for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//reduces enemy submission level to me
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    //pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost--;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].fights_lost < 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].id].submissive_to = false;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].id,{false,-1} });
                }
            }
        }

        for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//for every person I'm hostile to, remove me from their hostile list. Clean up this code to make more readable
            for (int j = 0; j < pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.size(); j++) {
                if (pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards[j] == pl[p].id) {
                    pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.erase(pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.begin() + j);
                    break;
                }
            }
        }
        pl[p].hostile_towards.clear();
        pl[p].downed = false;//when fight ends, no longer downed.
        return true;//fight over
    }
    return false;//no fight. is this check redundant?
}


//fix this, has magic numbers?
void People::share_disposition(int p2_ind) {
    if (pl[p].dispositions.empty()) {
        return;
    }
    //random person for now, but should adjust to move all dispositions closer (or further) between the 2 people chatting
    int subjectp_ind = rand() % pl[p].dispositions.size();
    vector<int> pids;
    for (auto const& i : pl[p].dispositions) {//convert to vector because can't access random element from map
        pids.push_back(i.first);
    }
    int subjectp_id = pids[subjectp_ind];
    int disp = pl[p].dispositions[subjectp_id];

    int their_opinion_of_me;
    if (pl[p2_ind].dispositions.find(pl[p].id) != pl[p2_ind].dispositions.end()) {
        their_opinion_of_me = pl[p2_ind].dispositions[pl[p].id];
    }
    else{
        pl[p2_ind].dispositions.insert({pl[p].id,0});
        their_opinion_of_me = 0;
    }

    if (pl[p2_ind].dispositions.find(subjectp_id) == pl[p2_ind].dispositions.end()) {
        pl[p2_ind].dispositions.insert({ subjectp_id,0 });
    }

    if (their_opinion_of_me >= 0) {
        int op = p;
        p = p2_ind;
        disp = (their_opinion_of_me/10)*(disp / 10);//if like the person, disposition towards 3rd party goes up/down by a fraction, greater change proportional to how much one likes the sharer
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0)||(pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].id, -1, "different friend/enemy");
        }
        p = op;
    }
    else {
        int op = p;
        p = p2_ind;
        disp = -1*(disp / 10)* (their_opinion_of_me / 10);//if dislike the person, same as above but inverted
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0) || (pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].id, -1, "different friend/enemy");
        }
        p = op;
    }
}

//DO THIS
bool People::give_tribute() {
    vector<int> superiors;
    int sum = 0;
    for (auto const& i : pl[p].submissive_to) {
        if (i.second.submissive_to) {
            superiors.push_back(i.first);
            sum += pl[p_by_id(i.first)].authority;
        }
    }
    int give_to = rand() % sum;
    sum = 0;
    int tribute_to = -1;
    for (int i : superiors) {
        sum += pl[p_by_id(i)].authority;
        if (sum > give_to) {
            tribute_to = i;
        }
    }
    //choose gift to give
        //food, tool, clothing, art, etc

    //go to give it
    if (move_to(pl[p_by_id(tribute_to)].pos,"to give tribute")) {
        //give tribute
        //dispositions affected
        return true;//done????
    }
    return false;//in progress???? how to signal don't run this function?
}

//DO THIS
bool People::rebel() {
    vector<int> rebel_targets;
    for (auto const& i : pl[p].submissive_to) {
        if (i.second.submissive_to) {
            if (pl[p_by_id(i.first)].authority <= pl[p].authority || pl[p_by_id(i.first)].num_fights_won<=pl[p].num_fights_won) {
                rebel_targets.push_back(i.first);
            }
        }
    }
    //DO THIS
    //trigger fight against one of the targets, preferrably the one with lowest authority and/or fights won
    return false;
}

//DO THIS
void People::build_monument() {
    if (!pl[p].monument_unlocked) {
        return;
    }
    //not sure yet how to implement this. If built, win game. If choose to continue playing, then grants huge authority boost
    //temp implementation
    create_item("monument", { -1,-1 });
    drop_item(inventory_has("monument")[0]);
    //cout << pl[p].id << " HAS WON THE GAME\n";
}

void People::create_tracks(Position pos) {
    if (pl[p].last_position.x != -1) {
        Environment::Tracks t = { "human", 0, pos.x, pos.y};
        Environment::Map[pl[p].last_position.y][pl[p].last_position.x].track = t;
    }
}


//need to implement a chance of no longer submitting to someone if their authority level falls too low or below one's own
//need to either figure out a way to handle order execution priority between getting food and removing campsite or create a function called remove_campsite to encapsulate its code and call before attempting to get food

//note: need to make parent check on kid periodically, and give priority to giving food to kids rather than others. fix this //feed hungriest first? Unsure if this should be implemented
//note: spoken messages might need their own priority tree separate from the movement related actions

//note: DO THIS add a small limit to inventory size to encourage the use of bags, carts/travois, multiple trips, etc. Makes more interesting logistics and behavior and the dropping of items or use of storage containers creates a nicer aesthetic

//disposition should be affected by or affect:
//item requests. don't request from disliked people. If I have authority or they submit to me, chance of requesting items even if I don't need them. Need to add requests for other actions such as to marry, to marry someone's daughter or to marry my kids to theirs, to start a fight with an enemy, to move campsite somewhere else (near mine or somewhere specific), etc
//chatting. likelihood of chatting with someone
//marriage. if married and whether to marry
//family. like children, parents and siblings more?
//movement. keep larger distance from enemies, closer to liked people
//fight. if fought on the same side as someone or opposite sides, and if made to submit to someone
//need more social activities, maybe song, dance, ritual. Cooperative actions such as group hunts, group gathers, building, crafting, etc. Smoking, etc.
//need more sources of drama such as infidelity, jealousy (of wealth measured in average hunger level, items, kids, being liked, etc), emotions such as sadness/grief if someone dies, theft, specific revenge, asking for forgiveness, asking for 3rd party help in resolving a problem, etc.

/* TO DO:
* fix bugs
* add disposition effects - later
* finish rebel - later
* handle picking up traps or moving them - later
* handle better for carrying and dropping infants - later
* 
* then implement player and do a test run to see if can reach victory condition
* then polish graphics and animations
* then add more variety both visual, item, misc small features/mechanics, etc
* then add sounds/music, menu, tutorial, etc
*/

//if average family is kept at 2 parents, 2 grandparents of male, and 2 kids/infants, then family is size 6, then soft cap total pop size at around 60 people or 10 families

//need to conduct unit tests on all functions to ensure they work properly and for all edge cases

//need to write out in english list of all conditionals, scenarios and behaviors and then check that the code actually does these. 
//write out exactly what each function does in english to see if it is actually a desirable behavior

//Compare current version of my game to rimworld and list all differences so as to reach action/item parity where it makes sense. 
//Where an item or action does not make sense to add to my game, then add something else such that there is numerical parity of features at least. 
//Do the same with other similar games to ensure my quality is worth an equal pricing of game.