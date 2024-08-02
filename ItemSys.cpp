#include "ItemSys.hpp"
using namespace std;

vector<ItemSys::Item> ItemSys::item_list;
int ItemSys::item_id_iterator = 0; 

ItemSys::ItemSys() {
    tent = { -1, "tent", "pics/house.png" };
    berrybush = { -1, "berrybush", "pics/berrybush.png", {"food"}};
    grain = { -1, "grain", "grain" , {"food", "needs processing"}};
    bread = { -1, "bread", "bread" , {"food"} };
    rock = { -1, "rock", "rock" };
    mortar_pestle = { -1, "mortar_pestle", "mortar and pestle" , {"tool"} };
}

int ItemSys::item_by_id(int id) {//uses binary search to find and return index to item in item list
    int low = 0;
    int high = item_list.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (item_list[mid].item_id == id) {
            return mid;
        }
        (item_list[mid].item_id < id) ? low = mid + 1 : high = mid - 1;
    }
    return -1;//not found
}

int ItemSys::new_item_id() {//unsure if this function is redundant with how int++ works or if there's a better method
    item_id_iterator++;
    return item_id_iterator;
}
