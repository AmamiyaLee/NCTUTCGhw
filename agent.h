#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"
#include <vector>
#include <limits>
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
public:
    std::array<int, 3> bag;
    int hint;
    int total_amt;
    int bonus_amt;
    int todo;
    int newhint;
    const  std::array<std::array<int, 6>, 32> tupleindexs{{
    {{0, 4, 8, 9, 12, 13}},
    {{1, 5, 9, 10, 13, 14}},

    
    
    {{3, 2, 1, 5, 0, 4}},
    {{7, 6, 5, 9, 4, 8}},

    
    
    {{15, 11, 7, 6, 3, 2}},
    {{14, 10, 6, 5, 2, 1}},

  
    
    {{12, 13, 14, 10, 15, 11}},
    {{8, 9, 10, 6, 11, 7}},

        {{3, 7, 11, 10, 15, 14}},
        {{2, 6, 10, 9, 14, 13}},
        {{0, 1, 2, 6, 3, 7}},
        {{4, 5, 6, 10, 7, 11}},
        {{12, 8, 4, 5, 0, 1}},
        {{13, 9, 5, 6, 1, 2}},
        {{15, 14, 13, 9, 12, 8}},
        {{11, 10, 9, 5, 8, 4}},
/////////////////////
 
    {{2, 1, 6, 5, 10, 9}},
    {{1, 0, 5, 4, 9, 8}},
    

    {{4, 8, 5, 9, 6, 10}},
    {{8, 12, 9, 13, 10, 14}},
        

        {{13, 14, 9, 10, 5, 6}},
        {{14, 15, 10, 11, 6, 7}},
        

        {{11, 7, 10, 6, 9, 5}},
        {{7, 3, 6, 2, 5, 1}},
    
        {{1, 2, 5, 6, 9, 10}},
        {{2, 3, 6, 7, 10, 11}},
        {{7, 11, 6, 10, 5, 9}},
        {{11, 15, 10, 14, 9, 13}},
        {{14, 13, 10, 9, 6, 5}},
        {{13, 12, 9, 8, 5, 4}},
        {{8, 4, 9, 5, 10, 6}},
        {{4, 0, 5, 1, 6, 2}}
}};

;
    void reset(){
            total_amt=0;
            bonus_amt=0;
            bag[0]=4;
            bag[1]=4;
            bag[2]=4;
            hint=0;
            newhint=0;
            todo=0;
        }
    
    long get_feature(const board& b, const std::array<int, 6> index){
        long result = 0;
        int tile;
        for(int i : index){
            //std::cout<<i<<"。  "<<result<<std::endl;
            
            result *= 15;
            tile = b.operator()(i);
            result += tile;
            
        }
        return result;
    }
    
};



/**
 * random environment
 * add a new random tile to an empty cell
 * tile bag>> {1,2,3} and only refill when its empty({0,0,0} since its an array and cant delete element)
 * pick a random one from tile bag
 */
class weight_agent : public agent {
public:
    weight_agent(const std::string& args = "") : agent(args) {
       
            init_weights(meta["init"]);
        if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
            load_weights(meta["load"]);
    }
    virtual ~weight_agent() {
        if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
            save_weights(meta["save"]);
    }
    float val(const board& boards){
        float result = 0;
        for(int i = 0; i < 32; i++){
            board b=boards;
            result += net[i/16][get_feature(b, tupleindexs[i])];
            
        }
        return result;
    }
protected:
    virtual void init_weights(const std::string& info) {
        //std::cout<<"boom on initweight"<<std::endl;
        
        for(int i = 0; i < 2; i++){
            net.emplace_back(15*15*15*15*15*15); // create an empty weight table
            
        }
        //std::cout<<"boom after initweight"<<std::endl;
    }
    virtual void load_weights(const std::string& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));
        net.resize(size);
        for (weight& w : net) in >> w;
        in.close();
    }
    virtual void save_weights(const std::string& path) {
        std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        uint32_t size = net.size();
        out.write(reinterpret_cast<char*>(&size), sizeof(size));
        for (weight& w : net) out << w;
        out.close();
    }
    
protected:
    std::vector<weight> net;
};

class random_agent : public weight_agent {
public:
    random_agent(const std::string& args = "") : weight_agent(args) {
        if (meta.find("seed") != meta.end())
            engine.seed(int(meta["seed"]));
    }
    virtual ~random_agent() {}

protected:
    std::default_random_engine engine;
};
/**
 * base agent for agents with a learning rate
 */
class learning_agent : public weight_agent {
public:
    learning_agent(const std::string& args = "") : weight_agent(args), alpha(0.01f) {
        if (meta.find("alpha") != meta.end())
            alpha = float(meta["alpha"]);
    }
    void train_weight(const board& before,const board& after,const int reward){
        float diff = alpha * (reward + val(after) - val(before))/32;
        //std::cout<<"train"<<std::endl;
        for(int i = 0; i < 32; i++){
            long feature = get_feature(before, tupleindexs[i]);
            net[i/16][feature] +=  diff ;
            
        }
    }
    void train_weight(const board& before){
        
        float diff = alpha*(0.0 - val(before))/32;
       //std::cout<<"train"<<std::endl;
        for(int i = 0; i < 32; i++){
            long feature = get_feature(before, tupleindexs[i]);
            net[i/16][feature] +=  diff ;
            
        }
    }
    
    virtual ~learning_agent() {}
    
protected:
    float alpha;
};
class evilenv : public random_agent{
    public:
    evilenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
    down({ 0, 1, 2, 3}),up({12,13,14,15}),left({3,7,11,15}),right({0,4,8,12}) {
            bag[0]=4;
            bag[1]=4;
            bag[2]=4;
        }

        float search(board now,int L,char type,float alpha,float beta){
        // std::cout<<"playerhint: "<<now.hint<<std::endl;
        // std::cout<<"playerbag: "<<now.bag[0]<<" "<<now.bag[1]<<" "<<now.bag[2]<<std::endl;
        // std::cout<<"platertotal: "<<now.total_amt<<std::endl;
        // std::cout<<"playerbonus: "<<now.bonus_amt<<std::endl<<std::endl;
        int layer=L-1;
        board after;
        std::array<int,4> space;
        if(type == 'b'){  //before state
            float score = -std::numeric_limits<float>::max();
            int r, v = 0;
            for(int i = 0; i < 4; ++i){                
                after = now;
                r = after.slide(i);
                if(r != -1){
                    v = 1;
                    float child = search(after, layer,'a',alpha,beta);
                    child += r;

                    if(score < child) score = child;
                    alpha = std::max(alpha, score);
                    if(beta <= alpha) break;
                }
            }
            //existing child-node
            if(v == 1) return score;
            //non-existing child-node
            return -2;
        }
        else if(type=='a'){ //after state
            if(now.bag[0]==0&&now.bag[1]==0&&now.bag[2]==0){ //resupply tilebag
                now.bag[0]=4;
                now.bag[1]=4;
                now.bag[2]=4;
            }
            float score=0;
            if(L == 0){
                score=val(now);
                return score;
            }
            score=std::numeric_limits<float>::max();;
            if(now.last_dir==0){
            //left
                space=left;
            }
            else if(now.last_dir==1){
            //right
                space=right;
            }
            else if(now.last_dir==2){
            //up
                space=up;
            }
            else if(now.last_dir==3){
            //down
                space=down;
            }

            
            for(int pos:space){
                if(now(pos)!=0)continue;
                after=now;
                after.place(pos,now.hint);
                for(int i=0;i<3;i++){
                    if(now.bag[i]>0){
                        after.bag=now.bag;
                        after.hint=i+1;
                        after.bag[i]--;
                        after.total_amt++;

                        float c = search(after,layer,'b',alpha,beta);
                        if(c==-1)return -1;
                        else{
                            if(score>c) score=c;
                            beta = std::min(beta, score);
                            if(beta <= alpha) break;

                    }
                    }
                }
                int max=0;
                for(int i=0;i<16;i++){//check if theres a tile>48 1,1 2,2 3,3 4,6 5,12 6,24
                    if(now(i)>=7){
                        if(now(i)>=max){
                         max=now(i);
                        }
                    }
                }
                
                if(max>6&& (now.bonus_amt+1)/(now.total_amt+1) <= 1/21){
                    after.bag=now.bag;
                            for(int i = 4; i <= (max-3); ++i){
                            after.hint = 4;
                            after.bonus_amt++;
                            after.total_amt++;
                            float c = search(after, layer,'b',alpha,beta);
                            if(c == -1) return -1;
                            else{
                            if(score>c) score=c;
                            beta = std::min(beta, score);
                            if(beta <= alpha) break;

                            }
                        }
                }


            }

            return score;  
        }
        return 0;
    }

    virtual action take_action(const board& before) {
        todo=newhint;
        //std::cout<<newhint<<"   "<<todo<<std::endl<<std::endl;
        if(todo>3){
            bonus_amt++;
            // std::cout<<"bonus tile count: "<<bonus_amt<<std::endl;
        }
        //std::cout<<todo<<std::endl;
        float score = std::numeric_limits<float>::max();
        int at = 100;
        int depth=5;
        board after;

        total_amt++;
        bool bonus=false;
        int max=0;
        for(int i=0;i<16;i++){//check if theres a tile>48 1,1 2,2 3,3 4,6 5,12 6,24
                    if(after(i)>=7){
                        if(after(i)>=max){
                         max=after(i);
                        }
                    }
                }
        std::array<int,4> space;
        if(bag[0]==0&&bag[1]==0&&bag[2]==0){
             bag[0]=4;
            bag[1]=4;
            bag[2]=4;
        }


        if(before.last_dir==5){
            //initialize
        total_amt=0;
            bonus_amt=0;
            bag[0]=4;
            bag[1]=4;
            bag[2]=4;
            hint=0;
            newhint=0;
            todo=0;

            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (before(pos) != 0) continue;
                std::uniform_int_distribution<int> dis(1,3);
                newhint=dis(engine);
                while(bag[newhint-1]==0){
                    newhint=dis(engine);
                }
                todo=dis(engine);
                while(bag[todo-1]==0){
                    todo=dis(engine);
                }
                bag[todo-1]--;
                board::cell tile = todo;
                bag[newhint-1]--;
                //total_amt++;
                //std::cout<<newhint<<"   "<<todo<<std::endl;
                hint=newhint;
                return action::place(pos, tile);
            }
            }
        
        if(before.last_dir==0){
            space=left;
        }
        else if(before.last_dir==1){
            space=right;
        }
        else if(before.last_dir==2){
            space=up;
        }
        else if(before.last_dir==3){
            space=down;
        }
        else if(before.last_dir==4){
            if(bag[0]==0&&bag[1]==0&&bag[2]==0){
                bag[0]=4;
                bag[1]=4;
                bag[2]=4;
            }
            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (before(pos) != 0) continue;
                std::uniform_int_distribution<int> dis(1,3);
                newhint=dis(engine);
                while(bag[newhint-1]==0){
                    newhint=dis(engine);
                }
                

                board::cell tile = todo;
                bag[newhint-1]--;
                //total_amt++;
                //std::cout<<todo<<"  "<<newhint<<std::endl;
                hint=newhint;
                return action::place(pos, tile);
            }
        }
        else{
            return action();
        }

        
        for(int pos : space){
        if(before(pos) != 0) continue;
                    after = before;
                    after.place(pos,todo);
                    for(int i = 0; i < 3; ++i){
                        if(bag[i] > 0){//bag contains i
                            after.bag[0] = bag[0];
                            after.bag[1] = bag[1];
                            after.bag[2] = bag[2];
                            after.total_amt=total_amt;
                            after.bonus_amt=bonus_amt;  
                            //generate new hint
                            after.hint = i+1;
                            --after.bag[i];
                            float t = search(after, 3,'b',-std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
                            if(t == -1){
                                newhint = i+1;
                                return action::place(pos, todo);
                            }
                            else if(score > t){
                                score = t;
                                newhint = i+1;
                                at = pos;
                            }
                        }
                    }
                    //max >= 7
                    
                    if(max > 6 && (bonus_amt+1)/(total_amt+1) <= 1/21){
                        after.bag = bag;
                        //generate new hint
                        for(int i = 4; i <= (max-3); ++i){
                            after.total_amt=total_amt;
                            after.bonus_amt=bonus_amt;
                            after.bag[0] = bag[0];
                            after.bag[1] = bag[1];
                            after.bag[2] = bag[2];
                            after.hint = i;
                            float t = search(after, 3,'b',-std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
                            if(t == -1){
                                newhint = i;
                                return action::place(pos, todo);
                            }
                            else if(score > t){
                                score = t;
                                newhint = i;
                                at = pos;
                            }
                        }
                    }
                }
                hint=newhint;
                --bag[newhint-1];
                return action::place(at, todo);
        
    }

    private:
    std::array<int, 4> left,right,up,down;
    
    
    
};
class rndenv : public random_agent {
public:
    
     
	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args),
    down({ 0, 1, 2, 3}),up({12,13,14,15}),left({3,7,11,15}),right({0,4,8,12}) {
            bag[0]=4;
            bag[1]=4;
            bag[2]=4;
        }
        
	virtual action take_action(const board& after) {
       // std::cout<<"env take action\n";
        //std::cout<<newhint<<"   "<<todo<<std::endl;
        todo=newhint;
        //std::cout<<newhint<<"   "<<todo<<std::endl<<std::endl;
        if(todo>3){
            bonus_amt++;
            // std::cout<<"bonus tile count: "<<bonus_amt<<std::endl;
        }
        //std::cout<<todo<<std::endl;
        total_amt++;
        bool bonus=false;
        int max=0;
        for(int i=0;i<16;i++){//check if theres a tile>48 1,1 2,2 3,3 4,6 5,12 6,24
                    if(after(i)>=7){
                        if(after(i)>=max){
                         max=after(i);
                        }
                    }
                }
        std::array<int,4> space;
        if(bag[0]==0&&bag[1]==0&&bag[2]==0){
             bag[0]=4;
            bag[1]=4;
            bag[2]=4;
        }
        



        if(after.last_dir==5){ //dir5=initial a new episode
            total_amt=0;
            bonus_amt=0;
            bag[0]=4;
            bag[1]=4;
            bag[2]=4;
            hint=0;
            newhint=0;
            todo=0;

            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (after(pos) != 0) continue;
                std::uniform_int_distribution<int> dis(1,3);
                newhint=dis(engine);
                while(bag[newhint-1]==0){
                    newhint=dis(engine);
                }
                todo=dis(engine);
                while(bag[todo-1]==0){
                    todo=dis(engine);
                }
                bag[todo-1]--;
                board::cell tile = todo;
                bag[newhint-1]--;
                //total_amt++;
                //std::cout<<newhint<<"   "<<todo<<std::endl;
                hint=newhint;
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
            if(bag[0]==0&&bag[1]==0&&bag[2]==0){
                bag[0]=4;
                bag[1]=4;
                bag[2]=4;
            }
            std::array<int,16> spacearr={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            std::shuffle(spacearr.begin(), spacearr.end(), engine);
            for (int pos : spacearr) {
                if (after(pos) != 0) continue;
                std::uniform_int_distribution<int> dis(1,3);
                newhint=dis(engine);
                while(bag[newhint-1]==0){
                    newhint=dis(engine);
                }
                

                board::cell tile = todo;
                bag[newhint-1]--;
                //total_amt++;
                //std::cout<<todo<<"  "<<newhint<<std::endl;
                hint=newhint;
                return action::place(pos, tile);
            }
        }
        std::shuffle(space.begin(), space.end(), engine);
		for (int pos : space) {
            
			if (after(pos) != 0) continue;
            if(!bonus){
                
                std::uniform_int_distribution<int> dis(1,3);
                newhint=dis(engine);
                while(bag[newhint-1]==0){
                    newhint=dis(engine);
                }
                

                board::cell tile = todo;
                bag[newhint-1]--;
                //total_amt++;
                hint=newhint;
                return action::place(pos, tile);
            }
            else{
                //random bonus tile 1/21
                std::array<int,21> ran={0};
                
                ran[0]=1;
                std::shuffle(ran.begin(),ran.end(),engine);
                
                if(ran[0]==1){
                //use bonus tile and random from 6 to maxtile/8
                    float ratio=(bonus_amt+1)/(total_amt+1);
                    
                    if(ratio<=(1/21)){
                        std::vector<int> bonustiles;
                        for(int b=4;b<max-2;b++){
                            bonustiles.push_back(b);
                        
                        }
                        
                        std::shuffle(bonustiles.begin(),bonustiles.end(),engine);
                        //std::cout<<bonustiles[0]<<"。"<<max<<std::endl;
                        newhint=bonustiles[0];
                        //total_amt++;
                        
                        hint=newhint;
                        
                        return action::place(pos,todo);
                    
                    }
                    else{
                        //use tilebag
                                    
                            std::uniform_int_distribution<int> dis(1,3);
                            newhint=dis(engine);
                            while(bag[newhint-1]==0){
                                newhint=dis(engine);
                            }
                            

                            board::cell tile = todo;
                            bag[newhint-1]--;
                            //total_amt++;
                            hint=newhint;
                            return action::place(pos, tile);
                    }
  
                }
                else{
                 //use tilebag
                    
                       std::uniform_int_distribution<int> dis(1,3);
                        newhint=dis(engine);
                        while(bag[newhint-1]==0){
                            newhint=dis(engine);
                        }
                        

                        board::cell tile = todo;
                        bag[newhint-1]--;
                       //total_amt++;
                        hint=newhint;
                        return action::place(pos, tile);
                }
            }
		}
        //std::cout<<"fucked up"<<std::endl;
		return action();
	}

private:
	std::array<int, 4> left,right,up,down;
    
        
	
    
    
};
/**
 * td0 player
 * select a legal action randomly
 */
class tdplayer : public learning_agent {
public:
    tdplayer(const std::string& args = "") : learning_agent("name=learning role=player " + args),
    opcode({ 0, 1, 2, 3 }),
    down({ 0, 1, 2, 3}),up({12,13,14,15}),left({3,7,11,15}),right({0,4,8,12}) {
       time=0;
    }
    //virtual void close_episode
//    virtual void close_episode(const std::string& flag = "") {
//        //std::cout<<"training"<<std::endl;
//        // train the n-tuple network by TD(0)
//
//    }
    float search(board now,int L,char type,float alpha,float beta){
        // std::cout<<"playerhint: "<<now.hint<<std::endl;
        // std::cout<<"playerbag: "<<now.bag[0]<<" "<<now.bag[1]<<" "<<now.bag[2]<<std::endl;
        // std::cout<<"platertotal: "<<now.total_amt<<std::endl;
        // std::cout<<"playerbonus: "<<now.bonus_amt<<std::endl<<std::endl;
        int layer=L-1;
        board after;
        std::array<int,4> space;
        if(type == 'b'){  //before state
            float score = -std::numeric_limits<float>::max();
            int r, v = 0;
            for(int i = 0; i < 4; ++i){                
                after = now;
                r = after.slide(i);
                if(r != -1){
                    v = 1;
                    float child = search(after, layer,'a',alpha,beta);
                    child += r;

                    if(score < child) score = child;
                    alpha = std::max(alpha, score);
                    if(beta <= alpha) break;
                }
            }
            //existing child-node
            if(v == 1) return score;
            //non-existing child-node
            return -2;
        }
        else if(type=='a'){ //after state
            if(now.bag[0]==0&&now.bag[1]==0&&now.bag[2]==0){ //resupply tilebag
                now.bag[0]=4;
                now.bag[1]=4;
                now.bag[2]=4;
            }
            float score=0;
            if(L == 0){
                score=val(now);
                return score;
            }
            score=std::numeric_limits<float>::max();;
            if(now.last_dir==0){
            //left
                space=left;
            }
            else if(now.last_dir==1){
            //right
                space=right;
            }
            else if(now.last_dir==2){
            //up
                space=up;
            }
            else if(now.last_dir==3){
            //down
                space=down;
            }

            
            for(int pos:space){
                if(now(pos)!=0)continue;
                after=now;
                after.place(pos,now.hint);
                for(int i=0;i<3;i++){
                    if(now.bag[i]>0){
                        after.bag=now.bag;
                        after.hint=i+1;
                        after.bag[i]--;
                        after.total_amt++;

                        float c = search(after,layer,'b',alpha,beta);
                        if(c==-1)return -1;
                        else{
                            if(score>c) score=c;
                            beta = std::min(beta, score);
                            if(beta <= alpha) break;

                    }
                    }
                }
                int max=0;
                for(int i=0;i<16;i++){//check if theres a tile>48 1,1 2,2 3,3 4,6 5,12 6,24
                    if(now(i)>=7){
                        if(now(i)>=max){
                         max=now(i);
                        }
                    }
                }
                
                if(max>6&& (now.bonus_amt+1)/(now.total_amt+1) <= 1/21){
                    after.bag=now.bag;
                            for(int i = 4; i <= (max-3); ++i){
                            after.hint = 4;
                            after.bonus_amt++;
                            after.total_amt++;
                            float c = search(after, layer,'b',alpha,beta);
                            if(c == -1) return -1;
                            else{
                            if(score>c) score=c;
                            beta = std::min(beta, score);
                            if(beta <= alpha) break;

                            }
                        }
                }


            }

            return score;  
        }
        return 0;
    }
    

    
virtual action take_action(const board &before){
        int op, imdt_r;
        int valid = 0;
        float score = -std::numeric_limits<float>::max();
        float current[4] = {0};      
        board temp = before;
         if(bag[0]==0&&bag[1]==0&&bag[2]==0){
             bag[0]=4;
            bag[1]=4;
            bag[2]=4;
        }


/*===============================================*/
        temp.hint =hint ;//get hint from env
        temp.total_amt=total_amt;
        temp.bonus_amt=bonus_amt;
        temp.bag[0] = bag[0];
        temp.bag[1] = bag[1];
        temp.bag[2] = bag[2];
/*===============================================*/
//                 if(time==5){
//         std::cout<<before<<std::endl;
//         std::cout<<"hint  total   bonus   bag0   bag1   bag2  \n";
//         std::cout<<temp.hint<<" "<<temp.total_amt<<" "<<temp.bonus_amt<<" "<<temp.bag[0]<<" "<<temp.bag[1]<<" "<<temp.bag[2]<<std::endl;
// }
        // std::cout<<"hint  total   bonus   bag0   bag1   bag2  \n";
        // std::cout<<temp.hint<<" "<<temp.total_amt<<" "<<temp.bonus_amt<<" "<<temp.bag[0]<<" "<<temp.bag[1]<<" "<<temp.bag[2]<<std::endl;
        

        for(int i = 0; i < 4; ++i){
            board as = temp;
            int reward = as.slide(i);
            current[i] += reward;
            if(reward != -1){
                valid=1;
                int max=0;
                for(int i=0;i<16;i++){//check if theres a tile>48 1,1 2,2 3,3 4,6 5,12 6,24
                    if(before(i)>=7){
                        if(before(i)>=max){
                         max=before(i);
                        }
                    }
                }
                
                current[i] += search(as, 2,'a',-std::numeric_limits<float>::max(),std::numeric_limits<float>::max());//searching i layers
            
            }
        }
        for(int i = 0; i < 4; ++i){
            if(current[i] != -1 && score < current[i]){
                score = current[i];
                op = i;
            }
        }

        //action found
        if(valid == 1){
        //     if(time==5){
        //         std::cout<<"   "<<current[0]<<" "<<current[1]<<" "<<current[2]<<" "<<current[3]<<std::endl;
        //     std::cout<<op<<std::endl;
        // }
            imdt_r = temp.slide(op);
            struct state step ={temp,imdt_r};
            episode.push_back(step);
            return action::slide(op);
        }
        else{

            // std::cout<<time<<"   "<<current[0]<<" "<<current[1]<<" "<<current[2]<<" "<<current[3]<<std::endl;
          
          /*  if(best_val!=std::numeric_limits<float>::min()){
                std::cout<<"wrong"<<std::endl;
            }*/
            //std::cout<<sizeof(episode[0])<<std::endl;
            //std::cout<<before;
            train_weight(episode[episode.size() - 1].after);
            for(int i = episode.size() - 2; i >= 0; i--){
                state step_next = episode[i + 1];

                train_weight(episode[i].after, step_next.after, step_next.reward);
            }
            episode.clear();
        //train_weight(before);
            //struct state step = {before, best_r};
            //episode.push_back(step);
            return action();
        }
        
    }
    // virtual action take_action(const board& before) {
        
    //     int bestop=0;
    //     float best_val=-std::numeric_limits<float>::max();
    //     int best_r;
    //     board after;
    //     bool avai=false;
    //     for (int op : opcode) {
    //         board b=before;
    //         board::reward reward = b.slide(op);
            
    //         if (reward != -1){
    //             avai=true;
                
    //             //std::cout<<"take action0\n";
    //             float val_n=val(b)+reward;
    //            // std::cout<<"take action1\n";
    //             if(val_n>best_val){
    //                 bestop = op;
    //                 best_val = val_n;
    //                 after=b;
    //                 best_r=reward;
    //             }
    //             //savestate(after,reward);
                
                
    //         }
    //     }
    //     if(avai){
    //         //std::cout<<best_val<<std::endl;
    //     //train_weight(before,after,best_r);
    //         struct state step = {after, best_r};
    //         episode.push_back(step);
    //     return action::slide(bestop);
    //     }
    //     else{
    //       /*  if(best_val!=std::numeric_limits<float>::min()){
    //             std::cout<<"wrong"<<std::endl;
    //         }*/
    //         //std::cout<<sizeof(episode[0])<<std::endl;
    //         train_weight(episode[episode.size() - 1].after);
    //         for(int i = episode.size() - 2; i >= 0; i--){
    //             state step_next = episode[i + 1];

    //             train_weight(episode[i].after, step_next.after, step_next.reward);
    //         }
    //         episode.clear();
    //     //train_weight(before);
    //         //struct state step = {before, best_r};
    //         //episode.push_back(step);
    //     return action();
    //     }
    // }
    /*
    float eval(const board& before,int op){
        board after(before);
        board::reward r=after.slide(op);
        
        return r+val(after);
    }*/
public:
    int time;
    
private:
    struct state {
        // select the necessary components of a state
        board after;
        int reward;
    };
    std::array<int, 4> left,right,up,down;
    std::vector<state> episode;

    
private:
    std::array<int, 4> opcode;
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
/*
const std::array<std::array<int, 6>, 32> tupleindexs = {{
    {{0, 4, 8, 9, 12, 13}}, 0
    {{1, 5, 9, 10, 13, 14}},
    {{1, 2, 5, 6, 9, 10}},
    {{2, 3, 6, 7, 10, 11}},
    
    {{3, 2, 1, 5, 0, 4}}, 1
    {{7, 6, 5, 9, 4, 8}},
    {{7, 11, 6, 10, 5, 9}},
    {{11, 15, 10, 14, 9, 13}},
    
    {{15, 11, 7, 6, 3, 2}}, 2
    {{14, 10, 6, 5, 2, 1}},
    {{14, 13, 10, 9, 6, 5}},
    {{13, 12, 9, 8, 5, 4}},
    
    {{12, 13, 14, 10, 15, 11}}, 3
    {{8, 9, 10, 6, 11, 7}},
    {{8, 4, 9, 5, 10, 6}},
    {{4, 0, 5, 1, 6, 2}},
    
    {{3, 7, 11, 10, 15, 14}}, 6
    {{2, 6, 10, 9, 14, 13}},
    {{2, 1, 6, 5, 10, 9}},
    {{1, 0, 5, 4, 9, 8}},
    
    {{0, 1, 2, 6, 3, 7}}, 7
    {{4, 5, 6, 10, 7, 11}},
    {{4, 8, 5, 9, 6, 10}},
    {{8, 12, 9, 13, 10, 14}},
    
    {{12, 8, 4, 5, 0, 1}}, 4
    {{13, 9, 5, 6, 1, 2}},
    {{13, 14, 9, 10, 5, 6}},
    {{14, 15, 10, 11, 6, 7}},
    
    {{15, 14, 13, 9, 12, 8}}, 5
    {{11, 10, 9, 5, 8, 4}},
    {{11, 7, 10, 6, 9, 5}},
    {{7, 3, 6, 2, 5, 1}}
}};
*/
