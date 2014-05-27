#ifndef CB_BOTWRAP
#define CB_BOTWRAP

#include "../interpreter/pkb.h"

class Robot {
friend class Game;
private:
	int x, y, energy, health, attr;
	std::string displaced, id;
public:
	pkb_instance pkb;
	int get_x() {return x;}
	int get_y() {return y;}
	int get_energy() {return energy;}
	int get_health() {return health;}
	std::string get_id() {return id;}
	bool alive() {return (health > 0);}

	void init(int y, int x, int health, int energy, std::string id) {
		this->y = y;
		this->x = x;
		this->health = health;
		this->energy = energy;
		this->id = id;
	}
	
	void tick(Map &map) {
		pkb.ireg[23] = x;
		pkb.ireg[24] = y;
		pkb.ireg[7] = health;
		pkb.ireg[4] = energy;
		pkb.tick(map, y, x);
	}

	void placeMarker(Map &map) {
		displaced = map.read(y,x);
		map.write(y,x,"@robot");
	}
	void removeMarker(Map &map) {
		if (map.read(y,x) == "@hit") {
			health -= 10;
		}
		map.write(y,x,displaced);
	}
	int getattr() {return attr;}
	void setattr(int attr) {this->attr = attr;}
	std::string get_exec() { // get the currently executed line
		if (!alive()) {
			return "dead";
		}
		if (pkb.finished) {
			return "finished";
		} else {
			return pkb.get_exec();
		}
	}
};

#endif
