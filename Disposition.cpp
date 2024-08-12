#include "People.hpp"
using namespace std;

//Need to figure out a method, similar to handling the separation of animation and game logic, for keeping disposition changes (when, why, how much, for who, etc) separate from game logic. 



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
    if (pl[p2_ind].dispositions.find(pl[p].p_id) != pl[p2_ind].dispositions.end()) {
        their_opinion_of_me = pl[p2_ind].dispositions[pl[p].p_id];
    }
    else {
        pl[p2_ind].dispositions.insert({ pl[p].p_id,0 });
        their_opinion_of_me = 0;
    }

    if (pl[p2_ind].dispositions.find(subjectp_id) == pl[p2_ind].dispositions.end()) {
        pl[p2_ind].dispositions.insert({ subjectp_id,0 });
    }

    if (their_opinion_of_me >= 0) {
        int op = p;
        p = p2_ind;
        disp = (their_opinion_of_me / 10) * (disp / 10);//if like the person, disposition towards 3rd party goes up/down by a fraction, greater change proportional to how much one likes the sharer
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0) || (pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].p_id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].p_id, -1, "different friend/enemy");
        }
        p = op;
    }
    else {
        int op = p;
        p = p2_ind;
        disp = -1 * (disp / 10) * (their_opinion_of_me / 10);//if dislike the person, same as above but inverted
        change_disposition(subjectp_id, disp, "shared disposition");
        if ((pl[p2_ind].dispositions[subjectp_id] >= 0 && pl[op].dispositions[subjectp_id] >= 0) || (pl[p2_ind].dispositions[subjectp_id] <= 0 && pl[op].dispositions[subjectp_id] <= 0)) {
            change_disposition(pl[op].p_id, 1, "shared friend/enemy");
        }
        else {
            change_disposition(pl[op].p_id, -1, "different friend/enemy");
        }
        p = op;
    }
}


//this might count as magic numbers but it might make sense to leave them here as it's a simple -100 to 100 range cap
void People::change_disposition(int p_id, int amount, string reason) {
    if (p_id == 0) {
        throw std::invalid_argument("pid is 0");
    }
    if (pl[p].p_id == p_id) {
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





