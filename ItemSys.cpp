#include "ItemSys.hpp"
using namespace std;

vector<ItemSys::Item> ItemSys::item_list;
int ItemSys::item_id_iterator = 0; 

ItemSys::ItemSys() {
    presets.insert({ "tent",            { -1, "tent", "pics/house.png", {/*tags*/}, {/*ingredients*/},false}});
    presets.insert({ "berrybush",       { -1, "berrybush", "pics/berrybush.png", {"food", "ready food"}, {/*ingredients*/},false}});
    presets.insert({ "grain",            { -1, "grain", "grain" , {"food", "needs processing"}, {/*ingredients*/},true} });
    presets.insert({ "bread",           { -1, "bread", "bread" , {"ready food"}, {"grain","mortar_pestle"},false}});
    presets.insert({ "rock",            { -1, "rock", "rock",{},{} ,true} });
    presets.insert({ "mortar_pestle",   { -1, "mortar_pestle", "mortar and pestle" , {"tool"}, {"rock"},false}});

    presets.insert({ "knife",   { -1, "knife", "pics/debug.png" , {"tool"}, {"rock"},false} });
    presets.insert({ "trap",   { -1, "trap", "trap" , {}, {"rock","wood"},false}});
    presets.insert({ "active trap",   { -1, "active trap", "trap" , {}, {},false} });//a version of trap that avoids getting picked up, must create a special pick up function for it
    presets.insert({ "rabbit_meat",   { -1, "rabbit_meat", "rabbit_meat" , {"ready food"}, {"rabbit"},false}});
    presets.insert({ "deer_meat",   { -1, "deer_meat", "deer_meat" , {"ready food"}, {"deer"},false}});

    presets.insert({ "tree",   { -1, "tree", "tree" , {"wood source"}, {},false} });
    presets.insert({ "wood",   { -1, "wood", "wood" , {}, {},true} });

    presets.insert({ "monument",   { -1, "monument", "monument" , {}, {},false} });

    presets.insert({ "medicine",   { -1, "medicine","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "bandage",   { -1,  "bandage","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "campfire",   { -1,  "campfire","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "necklace",   { -1,  "necklace","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "trumpet",   { -1, "trumpet","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "fishing rod",   { -1,  "fishing rod","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "fishing bait",   { -1, "fishing bait","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "poncho",   { -1, "poncho","pics/debug.png" , {"clothes","torso_wearing_eq"}, {"wood"},false}});
    presets["poncho"].insulation_from_cold = 10;
    presets.insert({ "active fish hook",   { -1, "active fish hook","pics/debug.png" , {}, {"wood"},false} });

    fill_tag_lookup();
    /*
    cout << "\n\n\n\n\n";
    for (auto const& i : tags) {
        cout << i.first << ": ";
        for (string s : i.second) {
            cout << s << ", ";
        }
        cout << "\n";
    }
    */
}

void ItemSys::fill_tag_lookup() {
    for(auto const& i : presets){
        for (string t : i.second.tags) {
            if (tags.find(t) != tags.end()) {//if tag is in list
                tags[t].push_back(i.first);//add item name to tag list
            }
            else {
                tags.insert({ t,{i.first} });//else add tag to list with item name as its first element
            }
        }
    }
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
