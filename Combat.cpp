#include "People.hpp"
#include "Animal.hpp"
using namespace std;


//Any main functions that regard combat for People


bool People::hostile_detection() {
    bool found_hostiles = false;
    if (pl[p].search_results.find("people") != pl[p].search_results.end()) {
        for (Position pos : pl[p].search_results["people"]) {
            Person& p2 = pl[p_by_id(Environment::Map[pos.y][pos.x].person_id)];
            for (int i : p2.active_hostile_towards) {
                if (i == pl[p].p_id) {//if a person nearby is actively hostile to self
                    pl[p].active_hostile_towards.push_back(p2.p_id);
                    pl[p].hostile_towards.push_back(p2.p_id);
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

        bool won = false;//for individual fights and personal glory
        //DO THIS, later: check combat allies, if a third have been downed or killed, surrender and submit. Currently fights are only 1v1, no combat allies. Multiple enemies or combat allies might appear simply due to hate triggering 1v1 fights but is unintended.
        for (int i = 0; i < pl[p].active_hostile_towards.size(); i++) {//for every enemy combatant in the fight
            int p2 = p_by_id(fight_choice);
            bool tie = true;

            //combat
            while (tie && (Position::distance(pl[p].pos, pl[p2].pos) == 1 || move_to(pl[p2].pos, "to enemy combatant"))) {//go to target

                if (pl[p].progress_states.find("fighting") != pl[p].progress_states.end()) {
                    if (!pl[p].progress_states["fighting"].progress_func()) {
                        pl[p].current_image = "fighting";
                        return true;//in progress
                    }
                    else {
                        pl[p].clean_image = true;
                    }
                }
                else {
                    pl[p].progress_states.insert({ "fighting",{10} });
                }

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
                        if (pl[p].active_hostile_towards[i] == pl[p2].p_id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].p_id) {
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
                        if (pl[p].active_hostile_towards[i] == pl[p2].p_id) {
                            pl[p].active_hostile_towards.erase(pl[p].active_hostile_towards.begin() + i);
                            break;
                        }
                    }
                    for (int i = 0; i < pl[p2].active_hostile_towards.size(); i++) {
                        if (pl[p2].active_hostile_towards[i] == pl[p].p_id) {
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
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].p_id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].fights_lost++;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].fights_lost >= 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].submissive_to = true;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].p_id,{false,1} });
                }
            }
        }
        else {
            //pl[p].num_fights_won--;
            for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//reduces enemy submission level to me
                if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.find(pl[p].p_id) != pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.end()) {
                    //pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].fights_lost--;
                    if (pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].fights_lost < 3) {
                        pl[p_by_id(pl[p].hostile_towards[i])].submissive_to[pl[p].p_id].submissive_to = false;
                    }
                }
                else {
                    pl[p_by_id(pl[p].hostile_towards[i])].submissive_to.insert({ pl[p].p_id,{false,-1} });
                }
            }
        }

        for (int i = 0; i < pl[p].hostile_towards.size(); i++) {//for every person I'm hostile to, remove me from their hostile list. Clean up this code to make more readable
            for (int j = 0; j < pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards.size(); j++) {
                if (pl[p_by_id(pl[p].hostile_towards[i])].hostile_towards[j] == pl[p].p_id) {
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

