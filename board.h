#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <math.h>
/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  (3)
 *  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11)
 * (12) (13) (14) (15)
 *
 */
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 4> row;
	typedef std::array<row, 4> grid;
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0) {}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 4][i % 4]; }
	const cell& operator ()(unsigned i) const { return tile[i / 4][i % 4]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
    int last_dir=5;
    int hint;
    std::array<int,3> bag;
    int total_amt;
    int bonus_amt;
    
public:

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
    enum Dir{LEFT,RIGHT,UP,DOWN}; //left=0 right=1 up=2 down=3
	reward place(unsigned pos, cell tile) {
        last_dir=4;
        if (pos >= 16) {
            std::cout<<"wrong pos"<<std::endl;
            return -1;
        }
		//if (tile != 1 && tile != 2&&tile != 3) return -1;
        
		
//        std::cout<<"+-------------------------+"<<std::endl;
//        std::cout<<operator()(0)<<" "<<operator()(1)<<" "<<operator()(2)<<" "<<operator()(3)<<" "<<std::endl;
//        std::cout<<operator()(4)<<" "<<operator()(5)<<" "<<operator()(6)<<" "<<operator()(7)<<" "<<std::endl;
//        std::cout<<operator()(8)<<" "<<operator()(9)<<" "<<operator()(10)<<" "<<operator()(11)<<" "<<std::endl;
//        std::cout<<operator()(12)<<" "<<operator()(13)<<" "<<operator()(14)<<" "<<operator()(5)<<" "<<std::endl;
        operator()(pos) = tile;
//        std::cout<<"Placed "<<tile << " on position ["<<pos<<"]"<<std::endl;
//        std::cout<<operator()(0)<<" "<<operator()(1)<<" "<<operator()(2)<<" "<<operator()(3)<<" "<<std::endl;
//        std::cout<<operator()(4)<<" "<<operator()(5)<<" "<<operator()(6)<<" "<<operator()(7)<<" "<<std::endl;
//        std::cout<<operator()(8)<<" "<<operator()(9)<<" "<<operator()(10)<<" "<<operator()(11)<<" "<<std::endl;
//        std::cout<<operator()(12)<<" "<<operator()(13)<<" "<<operator()(14)<<" "<<operator()(5)<<" "<<std::endl;
//        std::cout<<"+-------------------------+"<<std::endl;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
        //std::cout<<"this move is :"<<opcode<<std::endl;
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left() {

		
        
		board prev = *this;
		reward score = 0;
        int before_s=0;
        for(int i=0;i<16;i++){
            if(operator()(i)<3){
               
            }
            else{
                before_s+=pow(3,(operator()(i)-2));
                             }
            }
		for (int r = 0; r < 4; r++) {
			auto& row = tile[r];
			int  hold = 0;
			for (int c = 0; c < 4; c++) {
				int tile = row[c];
                if (tile == 0) {
                    if(c!=0){
                    row[c-1] = hold;
                    hold = tile;
                    }
                    continue;
                }
				row[c] = 0;
				if (hold) {
                    if(tile==1||tile==2){
                        if ((tile != hold)&&(hold==1||hold==2)) {
                            row[c-1] = 3;
                            //score += (1 << tile);
                            hold = 0;
                        } else {
                            row[c-1] = hold;
                            hold = tile;
                        }
                    }
                    else{
  
					if (tile == hold) {
						row[c-1] = ++tile;
						score += (1 << tile);
						hold = 0;
					} else {
						row[c-1] = hold;
						hold = tile;
					}
                    }
				} else {
                    if(c!=0){
                        row[c-1]=tile;
                        hold=0;
                    }
                    else{
					hold = tile;
                    }
				}
			}
            if (hold){
                if(tile[r][2]==0)
                tile[r][2] = hold;
                else{
                    tile[r][3]=hold;
                }
            }
		}
        
        last_dir=LEFT;
        int after_s=0;
        for(int i=0;i<16;i++){
            if(operator()(i)<3){
            
            }
            else{
                after_s+=pow(3,(operator()(i)-2));
            }
        }
        score=after_s-before_s;
		return (*this != prev) ? score : -1;
	}
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
        last_dir=RIGHT;
		return score;
	}
	reward slide_up() {
		rotate_right();
		reward score = slide_right();
		rotate_left();
        last_dir=UP;
		return score;
	}
	reward slide_down() {
		rotate_right();
		reward score = slide_left();
		rotate_left();
        last_dir=DOWN;
		return score;
	}

	void transpose() {
		for (int r = 0; r < 4; r++) {
			for (int c = r + 1; c < 4; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 4; r++) {
			std::swap(tile[r][0], tile[r][3]);
			std::swap(tile[r][1], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 4; c++) {
			std::swap(tile[0][c], tile[3][c]);
			std::swap(tile[1][c], tile[2][c]);
		}
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b){
		out << "+------------------------+" << std::endl;
		for(auto& row : b.tile){
			out << "|" << std::dec;
			for(auto t : row){
                    int k = (t > 3) ? (1 << (t-3) & -2u)*3 : t;
                    out << std::setw(6) << k;
			}
			out << "|" << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

private:
	grid tile;
	data attr;
};
