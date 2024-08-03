#include "Plants.hpp"

vector<Plants::Plant> Plants::species_presets;
vector<Plants::Plant> Plants::plants;
int plant_index;//current plant being updated

Plants::Plants(int a) {

}

void Plants::update_all() {
	for (int i = 0; i < plants.size(); i++) {
		plant_index = i;
		update();
	}
}

void Plants::update() {
	if (!plants[plant_index].is_alive) {
		return;
	}
	check_death();
	string species;
	string soil_requirements;
	int light_min;//minimum light needed to grow and live
	int temp_min;
	int temp_max;//temperature limits for growth, death and leaf loss
	int water_min;
	int water_max;
	string nutrient_requirements;
	int max_height;
	int current_height;//height and radius shade out shorter plants to outcompete them (deprive them of sunlight and water)
	int max_radius;
	int current_radius;//how far the tree shades out shorter plants or trees of the same height, or bushes doing the same to shorter plants
	vector<string> components;//Items: fruit, log, wood, branch, leaves, roots, sap, bark, fibers, etc
	int reproduction_rate;//in days
	int reproduction_distance;//in tiles, distance plant spreads
	//plant effects should be a property of their components? (leaves, fruit, etc) Same for calories?
	int lifespan;
	int age;
	int growth_rate;
	bool domesticated;
}

void Plants::reproduce() {

}

void Plants::check_death() {
	bool dehydration;
	bool drowned;
	bool light_deprivation;
	bool cold_death = plants[plant_index].current_temp <= plants[plant_index].temp_min;
	bool heat_death= plants[plant_index].current_temp >= plants[plant_index].temp_max;
	bool old_age_death = plants[plant_index].age>= plants[plant_index].lifespan;
	bool death = dehydration || drowned || light_deprivation || cold_death || heat_death || old_age_death;
	if (death) {
		plants[plant_index].is_alive = false;
		//change image to dead plant
	}
}
void Plants::fill_presets() {
	//fill from plants csv
}


