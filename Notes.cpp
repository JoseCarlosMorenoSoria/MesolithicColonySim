//TO DO NOW:
// need to fix why the campsite restriction and trigger to move isn't working
// 
//remember close family to avoid mating with parents/etc. Children exclusion done.
//Unsure if need to add a way to remember past locations that had food or if random searching is good enough for now in order to create cyclical seasonal camp movement
//add a way to compare value of foods according to calorie count and work/time cost
//if do see a rabbit now, get rock and move to within x distance of rabbit then throw rock and kill rabbit (for now just use hitscan and animation, no real projectile), rock ends up next to dead rabbit
//add fish and fishing
	// if Person is looking for food and sees water, get wood and craft spear (add wood spawning and spear item), go to water tile, stab water tile for a chance at catching fish, then process fish with knife into food
//add item degradation to ensure continual crafting of new items
// add stations such as campfire so that Person must be next to campfire to turn deer or rabbit meat into ready food, stations also degrade or require refuelling (if campfire is low or off, brin wood to it to keep it on or reignite), tents require maintenance or else they degrade away


//add dispositions between people and associated behavior (social fights, avoidance, escalation, calling for family/friends to join fight, noticing a family member being attacked and going to defend without being asked, sharing dispositions with others, etc)
	//add map<People,Disposition> to Person and activities and triggers for changing dispositions
	// add disposition checks to actions such as give item so as not to give items to people who are disliked or to give items based on a percent chance affected by disposition level, or higher chance of setting up camp near someone liked or the inverse if disliked, same for marriage and item requests, add a socialize need/function to seek out liked people to encourage clique congregation and avoidance
	// add chance of certain events happening such as a social fight between 2 people who hate each other
	// to add social fight, need to add hp to Person and a melee attack that reduces hp, don't reduce to the point of killing the other person
	// share dispositions with others, shared disposition is added to one's own disposition proportionally according to the disposition to the sharer
	// random chances to compliment and insult others, chances affected by disposition (highest disposition == never insult, lowest disposition == never compliment)
//add method to map and label natural cliques formed through the network of dispositions
	//???
//add method to map or create internal hierarchies among cliques, as well as ways to break/shift the hierarchies
	//???	add ambition stat to create probabilty of actively trying to curry favor with members of someone's clique and undermining that person?
	// have some way to ensure that 2 or more people can both be key friends of clique leader but hate each other so that when leader dies a power struggle occurs??
	//have 2 types of disposition, how much you like someone and how much you respect someone, so that someone can be a follower of x leader because they respect them (authority) but hate them and therefore be prone to chance of rebellion
//ensure natural methods for cliques not to become too stable so that groups are always changing, being formed, dissappearing, splitting, merging
//add more clique based behavior such as fighting over land if hungry, blood feuds, defend each other, raid each other, etc
	//?
//add tracking for hunting, so animals leave footprints or blood or feces by chance and over time (and these disappear over time) which trigger Person either to set traps or follow trail to animal to hunt
//add projectiles for when throwing rock/spear/dart, etc
//add temperature to affect clothing use and creation
//add terrain feritility and distribute using a noise function to create areas of varying amount of plants and therefore animals and food
//add trees, more animals, more aesthetic variation, more tools, weapons, armor, clothing, tents/house types, human visual variation, etc
//add cultural activities, actions, rituals, etc. 


//gameplay - sandbox colony sim with rpg mechanics - survival social management game - Nomad King
//survive (keep your npc alive from wolves, other npcs, get enough food, water, avoid dying from cold/heat, etc)
//raise a family (get a mate, have kids, keep them alive by bringing them food, give them commands for tasks to prioritize such as hunting, gathering, crafting, building, etc)
//raise your disposition with other people to gain advantages in resource access and number of people you can delegate tasks too
//achieve enough social influence to be declared tribal chief and beat out competitors for the title both external and internal, as well as settling disputes within your group and defending against outside attacks
//can delegate tasks, roles and partition terrirory but npcs might rebel according to their dispositions to you and each other and internal motives
//ensure your chosen heir successfully takes over after your death, keep your dynasty alive. 