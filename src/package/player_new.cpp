#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <climits>

#define DEPTH 6
#define diffCTR 4
#define cornerCTR 10

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;  //player black = 1, player white = 2
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const { //check trhu 8 directions whether the spot is valid or not
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    OthelloBoard(const OthelloBoard& copy){
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = copy.board[i][j];
            }
        }
        cur_player = copy.cur_player;
        disc_count = copy.disc_count;
        next_valid_spots = copy.next_valid_spots;
        done = copy.done;
        winner = copy.winner;
    }

    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    } 
};
  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int player;
const int SIZE = 8;
std::vector<Point> next_valid_spots;
OthelloBoard global;

const int boardWeight[8][8] = { 
    {25, -5,  11,  6,  6, 11, -5, 25},
    {-5, -10,   1,  1,  1,  1, -10, -5},
    { 11,  1,   4,  2,  2,  4,   1, 11},
    {  6,  1,   2,  1,  1,  2,   1,  6},
    {  6,  1,   2,  1,  1,  2,   1,  6},
    { 11,  1,   4,  2,  2,  4,   1, 11},
    {-5, -10,   1,  1,  1,  1, -10, -5},
    {25, -5,  11,  6,  6, 11, -5, 25 }
}; 

//calculate corner and edges 
int corner(OthelloBoard now){

    int points = 0;
    if (now.winner == player)
        points += 100;
    if (now.winner == 3 - player)
        points -= 100;
    if (now.cur_player == player)
        points += now.next_valid_spots.size();


    int weight = 0;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++) {
            if (now.board[i][j] == player)
                weight += boardWeight[i][j];
            else if (now.board[i][j] == 3 - player)
                weight -= boardWeight[i][j];
        }
    }

    points += weight*0.1;

    int corner = 0;
    int opcor = 0;

    if(now.board[0][0]==player){
        corner+=1;
        points += 50;

        bool row = false;
        bool col = true;
        int count = 0;

        for(int i=1; i<=6; i++){ 
            if(now.board[0][i]!=player) row = true;
            if(now.board[i][0]!=player) col = true;
            if(!row && now.board[0][i]==player){
                count+=1;
            }
            if(!col && now.board[i][0]==player){
                count+=1;
            }
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points+=count;

        if(now.board[0][1] == player) points+=2;
        if(now.board[1][1] == player) points+=3;
        if(now.board[1][0] == player) points+=2;
        
    }
    else if(now.board[0][0]==3-player){
        points -= 50;
        opcor += 1;

        bool row = false;
        bool col = true;
        int count = 0;

        for(int i=1; i<=6; i++){ 
            if(now.board[0][i]!=3-player) row = true;
            if(now.board[i][0]!=3-player) col = true;
            if(!row && now.board[0][i]==3-player){
                count+=1;
            }
            if(!col && now.board[i][0]==3-player){
                count+=1;
            }
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points-=count;

        if(now.board[0][1] == 3-player) points-=2;
        if(now.board[1][1] == 3-player) points-=3;
        if(now.board[1][0] == 3-player) points-=2;
    }

    if(now.board[0][7]==player){
        corner+=1;
        points += 50;

        bool row = false;
        bool col = true;
        int count = 0;
        int i=6;
        int j=1;
        while(i!=0 && j!=7){
            if(now.board[0][i]!=player) row = true;
            if(now.board[j][7]!=player) col = true;
            if(now.board[0][i]==player && !row) count+=1;
            if(now.board[j][7]==player && !col) count +=1;
            i--, j++;
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points+=count;
        
        if(now.board[1][7] == player) points+=2;
        if(now.board[1][6] == player) points+=3;
        if(now.board[0][6] == player) points+=2;
    }
    else if(now.board[0][7]==3-player){
        points -= 50;
        opcor += 1;

        bool row = false;
        bool col = true;
        int count = 0;
        int i=6;
        int j=1;
        while(i!=0 && j!=7){
            if(now.board[0][i]!=3-player) row = true;
            if(now.board[j][7]!=3-player) col = true;
            if(now.board[0][i]==3-player && !row) count+=1;
            if(now.board[j][7]==3-player && !col) count +=1;
            i--, j++;
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points-=count;
        
        if(now.board[1][7] == 3-player) points-=2;
        if(now.board[1][6] == 3-player) points-=3;
        if(now.board[0][6] == 3-player) points-=2;
    }

    if(now.board[7][0]==player){
        corner+=1;
        points += 50;

        bool row = false;
        bool col = true;
        int count = 0;
        int j=6;
        int i=1;
        while(j!=0 && i!=7){
            if(now.board[7][i]!=player) row = true;
            if(now.board[j][0]!=player) col = true;
            if(now.board[7][i]==player && !row) count+=1;
            if(now.board[j][0]==player && !col) count +=1;
            j--, i++;
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points+=count;

        if(now.board[7][1] == player) points+=2;
        if(now.board[6][1] == player) points+=3;
        if(now.board[6][0] == player) points+=2;
    }
    else if(now.board[7][0]==3-player){
        points -= 50;
        opcor += 1;

        bool row = false;
        bool col = true;

        int count = 0;
        int j=6;
        int i=1;
        while(j!=0 && i!=7){
            if(now.board[7][i]!=3-player) row = true;
            if(now.board[j][0]!=3-player) col = true;
            if(now.board[7][i]==3-player && !row) count+=1;
            if(now.board[j][0]==3-player && !col) count +=1;
            j--, i++;
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points-=count;

        if(now.board[7][1] == 3-player) points-=2;
        if(now.board[6][1] == 3-player) points-=3;
        if(now.board[6][0] == 3-player) points-=2;
    }

    if(now.board[7][7]==player){
        corner+=1;
        points += 50;

        bool row = false;
        bool col = true;
        int count = 0;
        for(int i=6; i>=1; i--){ 
            if(now.board[7][i]!=player) row = true;
            if(now.board[i][7]!=player) col = true;
            if(!row && now.board[7][i]==player){
                count+=1;
            }
            if(!col && now.board[i][7]==player){
                count+=1;
            }
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points+=count;

        if(now.board[6][7] == player) points+=2;
        if(now.board[6][6] == player) points+=3;
        if(now.board[7][6] == player) points+=2;
    }
    else if(now.board[7][7]==3-player){
        points -= 50;
        opcor += 1;

        bool row = false;
        bool col = true;
        int count = 0;
        for(int i=6; i>=1; i--){ 
            if(now.board[7][i]!=3-player) row = true;
            if(now.board[i][7]!=3-player) col = true;
            if(!row && now.board[7][i]==3-player){
                count+=1;
            }
            if(!col && now.board[i][7]==3-player){
                count+=1;
            }
        }
        if(!row && !col) count*=4;
        if(!row || !col) count*=2;
        points-=count;
        
        if(now.board[6][7] == 3-player) points-=2;
        if(now.board[6][6] == 3-player) points-=3;
        if(now.board[7][6] == 3-player) points-=2;
    }

    // std::cout<<now.cur_player<<"-crnr:"<<points*1<<' ';

    size_t disc_diff = now.disc_count[player] - now.disc_count[3-player];

    if(corner==0 && now.disc_count[now.EMPTY] > 48){
        return disc_diff + points*20;
    }
    else if(opcor >corner) return disc_diff + points*5;
    else if(now.disc_count[now.EMPTY] > 24){
        return disc_diff + points*25;
    }
    return disc_diff + points *30;
}

int heuristic(OthelloBoard now){
    //std::cout<<"heur: "<<total<<".:. ";
    return corner(now);
}

class Node{
    public:
        Point p;
        int score;
        Node(){}
        Node(Point P, int Score) : p(P), score(Score) {}
};



Node MiniMax(const OthelloBoard curState, int depth, int alpha, int beta){
    // bool print=false;
    // if(alpha==INT_MIN&&beta==INT_MIN)print=true;
    //std::cout<<"in"<<depth<<"\n";
    if(depth == 0 || curState.done){
        return Node(Point(-1,-1),heuristic(curState));
        // return Node(Point(-1,-1),CEK(curState));
    }
    if(curState.cur_player == player){//max
        // std::cout<<"max"<<depth<<' ';
        Point P_Max = Point(-2,-2);
        int Max = INT_MIN;
        
        for(auto valid_spot : curState.next_valid_spots){
            OthelloBoard nextState (curState);
            nextState.put_disc(valid_spot);

            Node nextMoveMin = MiniMax(nextState, depth-1, alpha, beta);
            if(nextMoveMin.score > Max){
                Max = nextMoveMin.score;
                P_Max = valid_spot;
                // std::cout<<depth<<"P_Max , Max: "<<P_Max.x<<P_Max.y<<" "<<Max<<"\n";
            }
            if(Max>alpha){
                alpha = Max;
            }
            if(alpha >= beta) {break;
            }
        }
        //std::cout<<depth<<"P_Max , Max: "<<P_Max.x<<P_Max.y<<" "<<Max<<"\n";
            
        // if(print)std::cout<<" "<<depth<<"Max "<<Max<<"\n";
        return Node(P_Max, Max);
    }
    else{//min
        // std::cout<<"min"<<depth<<' ';
        Point P_Min = Point(-3,-3);
        int Min = INT_MAX;

        for(auto valid_spot : curState.next_valid_spots){
            OthelloBoard nextState (curState);
            nextState.put_disc(valid_spot);
            
            Node nextMoveMax = MiniMax(nextState, depth-1, alpha, beta);
            if(nextMoveMax.score < Min){
                Min = nextMoveMax.score;
                P_Min = valid_spot;
                // std::cout<<depth<<"P_Min , Min: "<<P_Min.x<<P_Min.y<<" "<<Min<<"\n";
            }
            if(Min<beta){
                beta = Min;
            }
            if(beta <= alpha) {break;
            }
        }
        //std::cout<<depth<<"Min: "<<P_Min.x<<P_Min.y<<" "<<Min<<"\n";
        // if(print)std::cout<<" "<<depth<<"Min "<<Min<<"\n";
        return Node(P_Min, Min);
    }  
}



void read_board(std::ifstream& fin) {
    fin >> player;
    global.cur_player = player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> global.board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({static_cast<float>(x), static_cast<float>(y)});
    }
}

void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = next_valid_spots.size();
    if(n_valid_spots == 0) return;

    global.next_valid_spots.clear();
    global.next_valid_spots = next_valid_spots;
    // for(auto it:global.next_valid_spots){
    //     std::cout<<it.x<<it.y<<std::endl;
    // }
    Node maxim = MiniMax(global, DEPTH, INT_MIN, INT_MAX);
    // Remember to flush the output to ensure the last action is written to file.
    fout << maxim.p.x << " " << maxim.p.y << std::endl;
    // std::cout<<"Best Spot: "<<maxim.p.x << " " <<maxim.p.y <<std ::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
