#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include <vector>
class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * random environment
 * add a new random tile to an empty cell
 * tile bag>> {1,2,3} and only refill when its empty({0,0,0} since its an array and cant delete element)
 * pick a random one from tile bag
 */
class rndenv : public random_agent {
public:
    std::vector<int> tilebag;
     const int tilebagarr[3]={1,2,3};
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
    down({ 0, 1, 2, 3}),up({12,13,14,15}),left({3,7,11,15}),right({0,4,8,12}) {
            tilebag.assign(tilebagarr,tilebagarr+3);
        }

	virtual action take_action(const board& after) {
        std::array<int,4> space;
        if(tilebag.empty()){
            tilebag.assign(tilebagarr,tilebagarr+3);
        }
        if(after.last_dir==5){
            tilebag.clear();
            tilebag.assign(tilebagarr,tilebagarr+3);
            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (after(pos) != 0) continue;
                std::shuffle(tilebag.begin(),tilebag.end(),engine);
                board::cell tile = tilebag[0];
                tilebag.erase(tilebag.begin());
                return action::place(pos, tile);
            }
        }
		
        if(after.last_dir==0){
            space=left;
        }
        else if(after.last_dir==1){
            space=right;
        }
        else if(after.last_dir==2){
            space=up;
        }
        else if(after.last_dir==3){
            space=down;
        }
        else if(after.last_dir==4){
            if(tilebag.empty()){
                tilebag.assign(tilebagarr,tilebagarr+3);
            }
            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (after(pos) != 0) continue;
                std::shuffle(tilebag.begin(),tilebag.end(),engine);
                board::cell tile = tilebag[0];
                tilebag.erase(tilebag.begin());
                return action::place(pos, tile);
            }
        }
        std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
            
			if (after(pos) != 0) continue;
            std::shuffle(tilebag.begin(),tilebag.end(),engine);
			board::cell tile = tilebag[0]; //randomly put 2tile or 4tile, need to modify to tile bag for threes!
            tilebag.erase(tilebag.begin());
            return action::place(pos, tile);
		}
 
		return action();
	}

private:
	std::array<int, 4> left,right,up,down;
	
    
    
};

/**
 * dummy player
 * select a legal action randomly
 */
class player : public random_agent {
public:
	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),
		opcode({ 0, 1, 2, 3 }) {}

	virtual action take_action(const board& before) {
		std::shuffle(opcode.begin(), opcode.end(), engine);
		for (int op : opcode) {
			board::reward reward = board(before).slide(op);
			if (reward != -1) return action::slide(op);
		}
		return action();
	}

private:
	std::array<int, 4> opcode;
};
