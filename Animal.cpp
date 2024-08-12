#include "Animal.hpp"
#include "People.hpp"
using namespace std;

/*
if (al[a].fleeing) {
    int chance = rand() % 2;//50/50 chance that animal sprints away rather than simply moving away
    if (chance == 0) {
        al[a].speed = 2;
    }
}
else {
    al[a].speed = 1;
}
if (Position::distance(al[a].pos, dest) == 1) {//prevents overshooting destination if fleeing
    al[a].speed = 1;
}
*/

//^^^ need to isolate any unique parts of Animal both in the hpp and cpp, then import the latest version of People and strip it to create an updated Animal and add back in the unique parts saved prior.

vector<Animal::animal> Animal::al; //pl, using the name pl because of the frequency of use, used to store all Person instances
vector<Animal::Message> Animal::message_list;
vector<int> Animal::Message_Map[Environment::map_y_max][Environment::map_x_max];
int Animal::a = -1;//index for accessing current person. Using index to access instead of a pointer because list may change such as when a new person is born or dies which invalidates pointers to pl (people_list)
ItemSys Animal::it2;//used to access member functions and variables of ItemSys
int Animal::pday_count;
int Animal::phour_count;
int Animal::animal_id_iterator = 0;
int Animal::TILE_PIXEL_SIZE = 32;//can be modified by Game() class. Was originally 16, not sure if changing it to 32 made a difference. FIX THIS
vector<int> Animal::animals_in_stealth;
map<string, Animal::Species> Animal::species;
//Need to unit test every function and also test the frequency that each function executes, average value for all variables, and different scenarios (resource/people density, size, environment, etc)

//note: readability might be increased by creating null constants for things like int (-1) and Position (-1,-1), etc.
//could also maybe just overload and use the operator! as a return true or false if null function

//need to go through all parts of code that affect current_image to ensure there are no missing or wrong calls

Animal::Animal() {}

Animal::Animal(int init) {


}

void Animal::fill_species_presets() {
    vector<vector<string>> data = get_data("animal species csv");
    for (int i = 0; i < data.size(); i++) {//csv still needs to be transposed before download, unsure if it starts on first or third row?
        Species spec;
        int r = -1;
        spec.species=data[i][++r];
        spec.diet = data[i][++r];
        spec.meat_type = data[i][++r];//is this necessary?
        spec.HUNGER_REDUCTION_RATE = stoi(data[i][++r]);//per tick for now
        spec.HUNGRY_LEVEL = stoi(data[i][++r]);//assume only need 1 meal per day
        spec.DAYS_HUNGRY_MOVE_CAMP = stoi(data[i][++r]);
        spec.STARVATION_LEVEL = stoi(data[i][++r]);//record is 61 days no food, general max is 30 days no food, average is 8 to 21 days no food. For now set to 8
        spec.THIRST_REDUCTION_RATE = stoi(data[i][++r]);//per tick for now
        spec.THIRSTY_LEVEL = stoi(data[i][++r]);//assume only need one drink per day
        spec.DEHYDRATION_LEVEL = stoi(data[i][++r]);//rule of thumb is survive without water for 3 days, not sure about average or max record, set to 3 days for now
        spec.MAX_INFANT_AGE = stoi(data[i][++r]);
        spec.MIN_ADULT_AGE = stoi(data[i][++r]);
        spec.MAX_AGE = stoi(data[i][++r]);
        spec.REPRODUCTION_TRIGGER = stoi(data[i][++r]);//once every 7 days
        spec.spouse_distance_limit = stoi(data[i][++r]);//something like this could serve to keep packs/herds close together?
        spec.NEW_CAMP_PROBATION_TIME = stoi(data[i][++r]);//a third of a day
        spec.SLEEP_REST_RATE = stoi(data[i][++r]);
        spec.SLEEP_TRIGGER = stoi(data[i][++r]);//sleep every 2/3rds of a day
        spec.FORCE_SLEEP_LEVEL = stoi(data[i][++r]);//record is 11 days of no sleep, but extreme symptoms start at 36 hours (1.5 days). For now just set at 2x sleep trigger
        spec.campsite_distance_search = stoi(data[i][++r]);//should be half a day's walk from camp, fix this
        spec.HEAT_DEATH_TEMPERATURE = stoi(data[i][++r]);//humidity lowers heat tolerance such that at 100% humidity, humans die at 95F of heat in 6 hours. Need to implement humidity. //max heat that can be survived is between 104 and 122 F. For now set at 110F;//need to implement a time component to surviving temperatures.
        spec.COLD_DEATH_TEMPERATURE = stoi(data[i][++r]);//40 to 50 degrees can cause death in one to three hours. 32 to 40 degrees can cause death in 30 to 90 minutes. 32 degrees or less can cause death in as little as 15 to 45 minutes.//measured in F
        spec.SPRINT_LIMIT = stoi(data[i][++r]);
        spec.WALK_SPEED = stoi(data[i][++r]);
        spec.STEALTH_SPEED = stoi(data[i][++r]);
        spec.SPRINT_SPEED = stoi(data[i][++r]);
        spec.AMBUSH_DISTANCE = stoi(data[i][++r]);
        spec.STEALTH_DISTANCE = stoi(data[i][++r]);//distance from prey at which person enters stealth
        spec.SICK_TIME_DEATH = stoi(data[i][++r]); //later replace by specific illness/injury and body part mechanics such as bleeding out and organ failure
        spec.INJURED_TIME_DEATH = stoi(data[i][++r]);
        /*
        fight / flight
        social type
        tameable ?
        domesticated ?
        pack animal ? carry capacity
        other roles
        breathes water / air ?
        can burrow ?
        can swim ?
        can fly ?
        migration / territoriality
        actions
        hunting methods
        calories when killed
        spec.components
        */
        species.insert({ spec.species,spec });
    }
}

void Animal::update_all(int day_count, int hour_count, int hours_in_day) {
    a_p_flip = true;
    pday_count = day_count;
    phour_count = hour_count;

    //updates these constants if zoom level changes
    //WALK_SPEED = TILE_PIXEL_SIZE / 4;
    //STEALTH_SPEED = TILE_PIXEL_SIZE / 8;
    //SPRINT_SPEED = TILE_PIXEL_SIZE / 2;

    animals_in_stealth.clear();//clears every tick for simpler implementation. This needs to be rewritten in a better method instead of using Environment to communicate with Animal

    for (int i = 1; i < al.size(); i++) {//i starts at 1 because for now, the first Person in the list is reserved for player control
        a = i;
        ox = al[a].pos.x;
        oy = al[a].pos.y;
        update(day_count, hour_count, hours_in_day);
        if (al[a].in_stealth) {
            animals_in_stealth.push_back(al[a].a_id);
        }
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
    message_clear_flag = !message_clear_flag;//this is handled in Animal only, not People
}

bool Animal::check_death() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    
    bool starvation = c.hunger_level > sp.STARVATION_LEVEL;
    bool dehydration = c.thirst_level > sp.DEHYDRATION_LEVEL;
    bool old_age = c.age > sp.MAX_AGE;
    bool freeze_death = c.my_temperature <= sp.COLD_DEATH_TEMPERATURE;
    bool heat_death = c.my_temperature >= sp.HEAT_DEATH_TEMPERATURE;
    bool fatal_injury = c.injured_time == sp.INJURED_TIME_DEATH;
    bool fatal_sickness = c.sick_time == sp.SICK_TIME_DEATH;

    bool death = !c.is_alive || starvation || dehydration || old_age || freeze_death || heat_death || fatal_injury || fatal_sickness;
    if (death) {
        c.is_alive = false;
        c.current_image = c.species+"_dead.png";
        if (c.age < sp.MAX_INFANT_AGE) {
            c.current_image = c.species+"_infant_dead";
        }
        //if have spouse, free spouse to remarry, need a more realistic way to handle this rather than instant long distance unlinking
        if (c.spouse_id != -1) {
            if (c.species == "human") {
                People::pl[People::p_by_id(People::pl[People::p].spouse_id)].spouse_id = -1;//unlink from spouse
            }
            else {
                al[a_by_id(al[a].spouse_id)].spouse_id = -1;//unlink from spouse
            }
        }
        //set these to -1 to prevent others referencing them
        c.hunger_level = -1;
        c.thirst_level = -1;
        c.hungry_time = -1;
        c.reproduction_cooldown = -1;
        return true;
    }
    return false;
}

void Animal::update(int day_count, int hour_count, int hours_in_day) {
    animal& an = al[a];
    Species& sp = species[an.species];
    if (check_death()) {
        return;
    }

    if (an.speed_type == "walking") {//adjusts in case tile size changed
        an.speed = sp.WALK_SPEED;
    }
    else if (al[a].speed_type == "sprinting") {
        an.speed = sp.SPRINT_SPEED;
    }
    else if (an.speed_type == "stealth") {
        an.speed = sp.STEALTH_SPEED;
    }

    if (hour_count == 0) { //once a day check
        if (an.hunger_level > sp.HUNGRY_LEVEL) { //tracks for continuous hungry days at the start of every day
            an.hungry_time++;
        }
        else {
            an.hungry_time = 0;
        }
        an.age++;
    }
    if (an.age < sp.MAX_INFANT_AGE) {//is infant. Currently that means it doesn't do anything except get hungry and needs to be fed
        an.current_image = an.species+"_infant";
        an.hunger_level++;
        if (an.hunger_level > sp.HUNGRY_LEVEL) {
            speak("requesting food", -1);
        }
        if (an.being_carried) {//if being carried, then position is the position of the carrier offset by 1
            an.pos = al[a_by_id(al[a].carried_by_id)].pos;
            an.pos.x += 1;
        }
        an.immobile = true;
        eating();
        return;
    }
    else {
        an.immobile = false;
        an.clean_image = true;
    }

    an.hunger_level++; //hunger increases by 1 per hour, meaning it must decrease by 20 per day to stay at 0 daily average
    an.tired_level++; //same for tired level
    an.campsite_age++;
    an.reproduction_cooldown++; //for when to find mate and create new person
    an.thirst_level++;

    if (an.am_injured) {
        an.injured_time++;
    }
    if (an.am_sick) {
        an.sick_time++;
    }
    if (Environment::Map[an.pos.y][an.pos.x].temperature > an.my_temperature) {//like other needs, having this update every tick is not ideal and should be changed.
        an.my_temperature++;//fix this, need to make increase/decrease a percent of the difference between my_temp and tile_temp. Such that a large difference causes a large change in my_temp but a small one does not. Do the same for the clothing insulation below.

    }
    else if (Environment::Map[an.pos.y][an.pos.x].temperature < an.my_temperature) {
        an.my_temperature--;
    }

    if (an.clean_image) {
        an.current_image = an.species;//default image
        an.clean_image = false;
    }

    find_all();//gets all items, people, etc from within sight/earshot to then react to or inform next action

    utility_function();
    if (!an.move_already && an.general_search_called) {
        general_search_walk("");//ensures this function only executes once per update and also only after all other move_to's have been considered. This prioritizes intentional movement (moving to a target) rather than random movement
    }

    an.found_messages.clear();
    an.search_results.clear();

    //check to ensure that spouses share campsite
    if (!an.sex && !an.adopt_spouse_campsite && an.spouse_id != -1) {
        an.campsite_pos = al[a_by_id(al[a].spouse_id)].campsite_pos;
    }

    an.move_already = false;
    an.general_search_called = false;

}

void Animal::utility_function() {//is currently actually just a behavior tree not a utility function. Selects what action to take this tick.
    //this implementation allows functions to be interrupted by higher priority ones on every update, however this means that a function may not properly reset or preserve as needed for when it gets called again later, need to fix
    //if(func()==false) go to next func(), if(func()==true) executed this func() for both in progress and done cases
    animal& an = al[a];
    Species& sp = species[an.species];
    if (an.mov) {
        move_to(an.dest, "continue");
        return;
    }

    //initializing function locks so that if a function starts execution, it is allowed to run for at least a few ticks before getting preempted

    /* Not yet implemented for Animal
    if (an.animal_id_ambush != -1) {
        Animal::animal& an2 = Animal::al[a_by_id(an.animal_id_ambush)];//need to fix, as this needs to also allow ambushing humans, not just animals?
        if (Position::distance(an2.pos, an.pos) > an.sightline_radius) {//if lost sight of prey being ambushed, return to regular speed and no stealth
            an.animal_id_ambush = -1;
            an.speed = sp.WALK_SPEED;
            an.speed_type = "walking";
            an.in_stealth = false;
        }
        else {
            hunting(an2.species);//continue to hunt this type of animal. Need to change to specifically this target, also need to allow interruption by high priority functions such as being attacked/injured/etc
            return;
        }
    }
    */

    if (an.progress_states.find("utility preemption protector") == an.progress_states.end()) {
        an.progress_states.insert({ "utility preemption protector",{15} });//protects a function for at most 5 ticks
    }

    if (an.protected_func != -1) {
        if (an.progress_states["utility preemption protector"].progress_func()) {//not sure if it actually helped
            an.protected_func = -1;//release
        }
    }
    if (need_light() || an.protected_func == 0) { an.protected_func = 0; }
    else if (child_birth() || an.protected_func == 1) { an.protected_func = 1; }//pregnancy advancement should be moved to update function, not child_birth()
    //else if (health() || an.protected_func == 3) { an.protected_func = 3; }
    else if (exposure() || an.protected_func == 4) { an.protected_func == 4; }
    else if (sleeping() || an.protected_func == 5) { an.protected_func = 5; }//need to move collapsing from sleep to update function instead of sleeping()
    else if (drinking() || an.protected_func == 6) { an.protected_func = 6; }
    else if (eating() || an.protected_func == 7) { an.protected_func = 7; }//if don't have food, searches for food. Therefore the structure of utility_function is focused on which needs to satsify first (sleep, hunger, campsite, reproduction, etc)
    //for nests, dens, beehives, etc. Need to implement
    //else if (search_for_new_campsite() || an.protected_func == 8) { an.protected_func = 8; }
    else if (reproduce() || an.protected_func == 9) { an.protected_func =9; } //avoid execution of this function to focus on other features without worrying about population size
    //else if (answer_item_request() || an.protected_func == 10) { an.protected_func = 10; }
    else { idle(); }
}

bool Animal::child_birth() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    if (c.sex) {
        return false;//am male
    }
    if (c.pregnancy.progress == 0) {
        return false;//not pregnant
    }
    if (c.pregnancy.progress_func()) {//advance pregnancy until done, if done create child
        int sex = rand() % 2;
        Position child_pos;
        string no_type = (c.species == "human") ? "no people" : "no animal";
        if (c.search_results.find(no_type) != c.search_results.end()) {
            if (Position::distance(c.search_results[no_type][0], c.pos) == 1) {//if empty adjacent tile
                child_pos = c.search_results[no_type][0];
            }
        }
        if (child_pos.x == -1) {//if no empty adjacent tile found
            c.general_search_called = true;
            return true;//in progress
        }
        if (c.species == "human") {
            People::Person child;
            child.p_id = People::new_person_id();
            child.pos = child_pos;
            child.sex = sex;
            Environment::Map[child.pos.y][child.pos.x].person_id = child.p_id;
            People::pl.push_back(child);
            c.children_id.push_back(child.p_id);
            People::pl[People::p_by_id(c.spouse_id)].children_id.push_back(child.p_id);
        }
        else {
            animal child;
            child.a_id = new_animal_id();
            child.pos = child_pos;
            child.sex = sex;
            Environment::Map[child.pos.y][child.pos.x].animal_id = child.a_id;
            al.push_back(child);
            c.children_id.push_back(child.a_id);
            al[a_by_id(c.spouse_id)].children_id.push_back(child.a_id);
        }
        return true;//done
    }
}

bool Animal::reproduce() {
    animal& an = al[a];
    Species& sp = species[an.species];
    if (!(an.reproduction_cooldown > sp.REPRODUCTION_TRIGGER)) {//function trigger
        return false;
    }
    vector<Position>& pos_list1 = an.search_results[an.species];//note: using reference (&) reduces copying
    int a2 = -1;
    for (int i = 0; i < pos_list1.size(); i++) {//filter out valid mates from found list
        int anim_id = Environment::Map[pos_list1[i].y][pos_list1[i].x].animal_id;
        if (anim_id == -2) {
            cout << "error: anim_id==-2\n";//don't know why this is happening, already checked find_all() but it's the only place it could be inserted
            return true;//try again
        }
        int aid = a_by_id(anim_id);
        if (al[aid].sex != an.sex && al[aid].age > sp.MIN_ADULT_AGE && al[aid].is_alive) {
            bool is_my_child = false;
            for (int i = 0; i < an.children_id.size(); i++) {
                if (an.children_id[i] == al[aid].a_id) {
                    is_my_child = true;
                    break;
                }
            }
            if (!is_my_child && ((al[aid].spouse_id == -1 && an.spouse_id == -1) || (al[aid].spouse_id == an.a_id && an.spouse_id == al[aid].a_id))) {//if not my child AND both unmarried or if married to each other
                a2 = a_by_id(anim_id);//mate found
                break;
            }
        }
    }
    bool mate_willing = false;
    if (a2 != -1) {
        if (al[a2].reproduction_cooldown > sp.REPRODUCTION_TRIGGER && al[a2].sex != an.sex) {
            mate_willing = true;
        }
        if (mate_willing && (Position::distance(an.pos, al[a2].pos) == 1 || move_to(al[a2].pos, "going to mate"))) {//go to tile adjacent to a2
            //create a new human, add pregnancy later, only female creates child
            if (!an.sex) {//if female
                an.pregnancy.progress = 1;//am now pregnant
                an.reproduction_cooldown = 0;//reset
                al[a2].reproduction_cooldown = 0;//unsure if this is the best way to handle interaction between 2 people, speaking or some other function might be better to avoid 2 people not being in sync
                if (an.spouse_id == -1 && al[a2].spouse_id == -1) {//if don't have spouse, set as spouse
                    an.spouse_id = al[a2].a_id;
                    al[a2].spouse_id = an.a_id;
                }
                //remove campsite and adopt male's campsite as own.
                an.adopt_spouse_campsite = true;
                return true;//male will simply no longer call reproduce() given the cooldown==0, so only female needs to return true
            }
            else {
                //if male, simply wait for female to update one's reproduction_cooldown
            }
        }
    }
    else {//if no mate found, walk to search
        an.general_search_called = true;
    }
    return true;//in progress
}

//can this function also be folded into the find_all function somehow to further reduce for loops searching on the map?
//might also need to add some restriction so that each person doesn't blast out too many messages at once? Or maybe that's ok?
void Animal::speak(string message_text, int receiver_id) {//if receiver_id == -1, then the message is for everyone
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    //current valid messages include: need to list valid messages here
    //the outward ring method might make more sense in this function to allow certain objects such as walls to block sound, might implement later but not currently
    Message m = { new_message_id(), (c.species=="human")?People::pl[People::p].p_id:c.a_id, c.species, receiver_id, message_text, c.pos};//creates message
    for (int y = c.pos.y - c.audioline_radius; y < c.pos.y + c.audioline_radius; y++) {//creates copies of message for each map position it reaches then adds to global message list
        for (int x = c.pos.x - c.audioline_radius; x < c.pos.x + c.audioline_radius; x++) {
            if (Position::valid_position({ x,y })) {
                Message_Map[y][x].push_back(m.message_id);
                message_list.push_back(m);
            }
        }
    }
}

//fix this, this function has magic numbers but they are random percents, need to decide how to handle them
bool Animal::idle() {
    animal& an = al[a];
    Species& sp = species[an.species];
    //flip between idle and default image
    if (an.current_image == an.species) {
        an.current_image = an.species+"_idle.png"; //need to make image, just have human with raised hands
    }
    else if (an.current_image == an.species+"_idle.png") {
        an.current_image = an.species;
    }
    return true;
}

//Need to implement
//bool Animal::answer_item_request() {}

//FIX THIS
bool Animal::acquire(string target) {//target_type: animal/plant/pickup/adjaceny/craftable/person/information/permission
    //First, determine the type of target to be acquired
    string target_type;
    //if item can be picked up: target_type = "pickup"
    //else if item can be carried = "carry"
    //else if item is a source = "source" / "adjacency"     (terrain/plant/animal)
    //else if item is a station = "station" / "adjacency"   (campfire)
    //else if item is a building = "building"

    //check if target is an item name or item tag
    ItemSys::Item it = ItemSys::as_item_preset_by_name(target);
    if (it.item_name != "") {//target is an item name
       
            target_type = "pickup";
        
    }
    else if (Plants::species_presets.find(target) != Plants::species_presets.end()) {//target is a plant name
        target_type = "source";
    }
    /*
    else if (it2.tags.find(target) != it2.tags.end()) {//target is tag name
        for (string item_name : it2.tags[target]) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            if (acquire(item_name)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    */
  
    
    else if (target == "plant") {
        for (auto w : Plants::species_presets) {//for every item with this tag, attempt to acquire item, if one is acquired then tag is acquired therefore return true
            if (acquire(w.first)) {
                return true;//done
            }
        }
        return false;//in progress
    }
    else if (Environment::terrains.find(target) != Environment::terrains.end()) {//target is a terrain type
        target_type = "source";
    }
    else {
        throw invalid_argument{"target is invalid"};
        return false;
    }

    //Second, determine if already have the target or if it is nearby

    

    //if item has prereqs for acquiring, such as if item is a source (tree) and requires an axe to obtain, then fulfill prereq first (acquire(axe)) before continuing. Same for animals/plants, acquire spear/sickle

    //look around self for item
    //if found, move to item (or adjacent according to target_type) (or hunting action if living animal)

    if (al[a].search_results.find(target) != al[a].search_results.end()) {//key found, if key exists then at least 1 was found
        Position pos = al[a].search_results[target][0];
        int item_id = Environment::Map[pos.y][pos.x].item_id;
        if (Position::distance(al[a].pos, pos) == 1 || move_to(pos, "to found item " + target)) {//if item is found, move to it and pick it up
            if (target_type == "pickup" && move_to(pos, "to found item " + target)) {
                //likely is food, eat it right off the map
                return true;//target acquired
            }
            else {
                return false;//still moving to dest
            }

            if (target_type == "source") {
                //likely is water, drink right off the map
                //adjacency_acquire_handler(target);//extract/deconstruct
                return true;//target acquired
            }
        }
        return false;//if still moving towards item, continue to next tick
    }

    //if not found in immediate vicinity, then according to target_type:

    //hunting and well/tuber digging have special actions if the target isn't found (tracking,setting traps,digging) so need to execute those if target not found



    //if item is buildable, build it

    //else if item has a source (wood comes from trees, rock comes from stone terrain, water is from water terrain, bones from animals, etc
    //then acquire source
    if (it.item_type == "material") {
        ItemSys::Material m = ItemSys::material_presets[it.item_name];
        acquire(m.source);
    }

    //if all fails, move in search pattern. Search pattern is shared, to reduce erratic movement from various instances of search patterns
    al[a].general_search_called = true;
    return false;//searching
}

//continue improving this   Need to implement
/*
bool Animal::health() {
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
*/
//DO THIS later
bool Animal::need_light() {
    return false;
    //if too dark, either move to light or create or get light source such as candle/torch/campfire
}

bool Animal::exposure() {//protection from heat/sun and cold/wind/rain. Only temperature protection for now.
    animal& an = al[a];
    Species& sp = species[an.species];
    if (an.my_temperature == an.my_preffered_temperature) {//fix this, needs to be a distance from ideal temp, not exactly ideal temp
        return false;//temperature is fine
    }
    bool better_temp_found = false;
    if (an.my_temperature > an.my_preffered_temperature) {//need to add impact on thirst by heat in update function
        //seek cold, am too hot
        for (auto const& i : an.search_results) {
            if (stoi(i.first) < an.my_temperature) {//the only keys that are numbers will be tile temperatures, so find the first tile that has a lower temperature and go there
                move_to(i.second[0], "towards cold");
                better_temp_found = true;
                return true;
            }
        }
        if (!better_temp_found) {
            //implement these 3 later
            //seek darkness/shade
            //go in water
            //move less, be lazy
        }
        //if no method to improve temp, search
        an.general_search_called = true;
    }
    else if (an.my_temperature < an.my_preffered_temperature) {
        //seek heat, am too cold
        for (auto const& i : an.search_results) {
            if (stoi(i.first) > an.my_temperature) {//inverse of "towards cold"
                move_to(i.second[0], "towards heat");
                better_temp_found = true;
            }
        }
        if (!better_temp_found) {
            //implement later
            //move more, be active
        }
        //if no method to improve temp, search
        an.general_search_called = true;
    }
    //if am wet or in rain or in strong wind, seek shelter. Need to implement these. Fix this
    return false;//?
}

//need to simplify this function.
/*
bool Animal::search_for_new_campsite() { //need to bias search direction in the direction of wherever there is more food rather than waiting to randomly stumble on a site with enough food for campsite. Also need to add a system of not searching the same tile within too short a time frame.
    if (!pl[p].sex && pl[p].spouse_id != -1 && pl[p].campsite_pos == pl[p_by_id(pl[p].spouse_id)].campsite_pos) {
        return false;//prevent searching for a new campsite if married, only for females
    }

    bool cond2 = pl[p].campsite_pos.x == -1 || pl[p].hungry_time >= DAYS_HUNGRY_MOVE_CAMP;//AND: have no campsite OR have been hungry too long
    bool cond3 = pl[p].campsite_age > NEW_CAMP_PROBATION_TIME || pl[p].campsite_age == -1;//AND: campsite is old enough to move again. Unsure if this might have an issue if the null campsite has an age
    bool start = (cond2 && cond3) || pl[p].adopt_spouse_campsite;
    //currently only creates a campsite after having been hungry 3 days. Still need to figure out when and when not to create a campsite, such as for trips away from home or extreme high mobility nomad
    //function trigger
    if (!start) {
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
*/

bool Animal::sleeping() {
    animal& c = (a_p_flip) ? al[a] : People::pl[People::p];//controls if accessing an animal from animal list or a person from people list (derived from Animal)
    Species& sp = species[c.species];
    bool tired = c.tired_level > sp.SLEEP_TRIGGER;
    bool start_moving_to_bed = c.awake && tired && c.campsite_pos.x != -1 && c.hunger_level < sp.HUNGRY_LEVEL && c.thirst_level < sp.THIRSTY_LEVEL;
    if (start_moving_to_bed) {
        if (move_to(c.campsite_pos, "to bed")) { //go to campsite.
            //go to sleep, continue
        }
        else {
            return true;//done and in progress
        }
    }
    bool very_tired = c.tired_level > sp.FORCE_SLEEP_LEVEL;//might need to cap sleep such that a person can't ever have a tired_level over x_level as well as under y_level
    bool cond1 = tired && (c.pos == c.campsite_pos || c.campsite_pos.x == -1);//if tired AND either at campsite or have no campsite
    if (!(!c.awake || cond1 || very_tired)) {//function trigger
        return false;
    }
    c.current_image = c.species+"_sleeping.png";
    c.awake = false;
    c.tired_level -= sp.SLEEP_REST_RATE; //every call to this function reduces tired by 11, this means need 5 hours/updates to stop sleeping and sleep every 50 hours/updates. Is -11 so as to do -10 per hour and also -1 to negate the +1 tired in the regular update function
    if (c.tired_level <= 0) {//fix this, need to cap at 0, also need cap for upper limit?
        c.current_image = c.species;
        c.awake = true;
        return true;//done
    }
    return true;//in progress
}

//NOTE: for implementing cooperation, conduct through speak() requests and answers. Person 1 proposes joint action, Person 2 decides whether to agree or not. If a 3rd person or more are involved, then need to set a meeting location and time to conduct the proposition -> up/down vote and an option to continue action with those who said yes only. Later add option to be able to coerce those who said no into complying.

//new funcs

bool Animal::carry() {//should also have a push/pull action for larger/heavier items such as a large stone block for a monument
    //receive whether it is an item or person
    //recieve its id and store it
    //set current image to "carrying" and the current image and state of target person if a person to "carried", tie the position of the carried item/person to the tile in front of the carrier but also offset into the tile of the carrier so the images overlap
    return false;
}

bool Animal::drop() {
    return false;
    //if carrying something/someone, remove the pixel offset of the carried item/person and reset images of carrier and carried, remove id of carried from carrier and reset the state of carried to false
}

bool Animal::adjacency_acquire_handler() {//for cutting down trees, mining rock, digging out dirt, collecting water, etc
    //accept target from acquire()
    //acquire should only call this func if person is next to target source
    return false;
    //if tree, if have axe, chop tree animation

    //if stone, if have pickaxe, mining animation

    //if it's a station passed as an ingredient/requisite to craft something, then "acquiring" the station counts as either being next to it or building it first
    //ex: campfire for cooking

    //Construction is a variation on crafting but done by emplacing something on a tile with the resources adjacent to that tile or self rather than in one's inventory.
}


