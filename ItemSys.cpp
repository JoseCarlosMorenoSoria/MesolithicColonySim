#include "ItemSys.hpp"
using namespace std;

//TO DO TODAY: Need to add a method to read from a csv file and use it to hold and read data for Items and for Images
//also need to add exception-throws for if an image/item that doesn't exist is called, to catch what the name asked for is and find the source

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
    presets.insert({ "active trap",   { -1, "active trap", "trap" , {}, {},false,false} });//a version of trap that avoids getting picked up, must create a special pick up function for it
    presets.insert({ "rabbit_meat",   { -1, "rabbit_meat", "rabbit_meat" , {"ready food"}, {"rabbit"},false}});
    presets.insert({ "deer_meat",   { -1, "deer_meat", "deer_meat" , {"ready food"}, {"deer"},false}});

    presets.insert({ "tree",   { -1, "tree", "tree" , {"wood source"}, {},false,false} });
    presets.insert({ "wood",   { -1, "wood", "wood" , {}, {},true} });

    presets.insert({ "monument",   { -1, "monument", "monument" , {}, {},false,false} });

    presets.insert({ "medicine",   { -1, "medicine","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "bandage",   { -1,  "bandage","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "campfire",   { -1,  "campfire","pics/debug.png" , {}, {"wood"},false,false} });
    presets.insert({ "necklace",   { -1,  "necklace","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "trumpet",   { -1, "trumpet","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "fishing rod",   { -1,  "fishing rod","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "fishing bait",   { -1, "fishing bait","pics/debug.png" , {}, {"wood"},false} });
    presets.insert({ "poncho",   { -1, "poncho","pics/debug.png" , {"clothes","torso_wearing_eq"}, {"rock"},false}});
    presets["poncho"].insulation_from_cold = 10;
    presets.insert({ "active fish hook",   { -1, "active fish hook","pics/debug.png" , {}, {"wood"},false,false} });

    fill_tag_lookup();
    fill_ingredients_lookup();
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

    //test_read();
}

void ItemSys::test_read() {
//read from csv into a vector of structs
    fstream fin;// File pointer 
    fin.open("test_shiet - Sheet1.csv", ios::in);// Open an existing file 
    vector<string> row;// Read the Data from the file as String Vector 
    string line, word;
    bool firstrowdone = false;
    bool start_count = false;//for counting the max number of ingredient columns
    int count = 0;
    while (getline(fin, line)) {// read an entire row and store it in a string variable 'line' 
        cout << "\n";
        row.clear();
        stringstream s(line);// used for breaking words 
        while (getline(s, word, ',')) {// read every column data of a row and store it in a string variable, 'word' 
            row.push_back(word);// add all the column data of a row to a vector 
        }
        if (firstrowdone) {
            test_struct ts;
            ts.item_name = row[0];
            ts.item_id = stoi(row[1]);
            ts.item_stat1 = stoi(row[2]);
            int ingr_end = 3 + count;
            for (int i = 3; i < ingr_end; i++) {
                    if (row[i] == "") {
                        break;
                    }
                    ts.ingredients.push_back(row[i]);
            }
            ts.bool1 = (row[ingr_end] == "TRUE") ? true : false;
            ts.image = row[ingr_end +1];
            testlist.push_back(ts);
        }
        if (!firstrowdone) {
            for (string i : row) {
                if (i == "ingredients") { start_count = true; }
                else if (i == "bool1") { start_count = false; }
                if (start_count) { count++; }
            }
            firstrowdone = true;
        }
    }
    for (auto i : testlist) {//print vector of structs to verify it got inserted correctly
        cout << "\n";
        cout<<i.item_name<<"; ";
        cout << i.item_id << "; ";
        cout << i.item_stat1 << "; ";
        for (auto j : i.ingredients) {
            cout << j << "; ";
        }
        cout << i.bool1 << "; ";
        cout << i.image << "; ";
    }
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

void ItemSys::fill_ingredients_lookup() {
    for (auto const& i : presets) {
        for (string t : i.second.ingredients) {
            if (ingredients.find(t) != ingredients.end()) {//if ingredient is in list
                ingredients[t].push_back(i.first);//add item name to ingredients list
            }
            else {
                ingredients.insert({ t,{i.first} });//else add ingredient to list with item name as its first element
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
