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

#define max(a,b) ((a)>(b)?(a):(b))
#define min(a,b) ((a)<(b)?(a):(b))

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
        return 3 - player;
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
    bool is_spot_valid(Point center) const {
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
    std::string encode_player(int state) {
        if (state == BLACK) return "O";
        if (state == WHITE) return "X";
        return "Draw";
    }
    std::string encode_spot(int x, int y) {
        if (is_spot_valid(Point(x, y))) return ".";
        if (board[x][y] == BLACK) return "O";
        if (board[x][y] == WHITE) return "X";
        return " ";
    }
    std::string encode_output(bool fail=false) {
        int i, j;
        std::stringstream ss;
        ss << "Timestep #" << (8*8-4-disc_count[EMPTY]+1) << "\n";
        ss << "O: " << disc_count[BLACK] << "; X: " << disc_count[WHITE] << "\n";
        if (fail) {
            ss << "Winner is " << encode_player(winner) << " (Opponent performed invalid move)\n";
        } else if (next_valid_spots.size() > 0) {
            ss << encode_player(cur_player) << "'s turn\n";
        } else {
            ss << "Winner is " << encode_player(winner) << "\n";
        }
        ss << "+---------------+\n";
        for (i = 0; i < SIZE; i++) {
            ss << "|";
            for (j = 0; j < SIZE-1; j++) {
                ss << encode_spot(i, j) << " ";
            }
            ss << encode_spot(i, j) << "|\n";
        }
        ss << "+---------------+\n";
        ss << next_valid_spots.size() << " valid moves: {";
        if (next_valid_spots.size() > 0) {
            Point p = next_valid_spots[0];
            ss << "(" << p.x << "," << p.y << ")";
        }
        for (size_t i = 1; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << ", (" << p.x << "," << p.y << ")";
        }
        ss << "}\n";
        ss << "=================\n";
        return ss.str();
    }
    std::string encode_state() {
        int i, j;
        std::stringstream ss;
        ss << cur_player << "\n";
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE-1; j++) {
                ss << board[i][j] << " ";
            }
            ss << board[i][j] << "\n";
        }
        ss << next_valid_spots.size() << "\n";
        for (size_t i = 0; i < next_valid_spots.size(); i++) {
            Point p = next_valid_spots[i];
            ss << p.x << " " << p.y << "\n";
        }
        return ss.str();
    }
};

int BoardPoint [8][8] = {
    {10, 2, 4, 4, 4, 4, 2,10},
    { 2, 1, 3, 3, 3, 3, 1, 2},
    { 4, 3, 3, 3, 3, 3, 3, 4},
    { 4, 3, 3, 3, 3, 3, 3, 4},
    { 4, 3, 3, 3, 3, 3, 3, 4},
    { 4, 3, 3, 3, 3, 3, 3, 4},
    { 2, 1, 3, 3, 3, 3, 1, 2},
    {10, 2, 4, 4, 4, 4, 2,10}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

OthelloBoard now;

const int BoardWeight[8][8] = { 
    {100, -10,  11,  6,  6, 11, -10, 100},
    {-10, -20,   1,  2,  2,  1, -20, -10},
    {  1,   2,   5,  4,  4,  5,   1,  11},
    {  6,   2,   4,  2,  2,  4,   2,   6},
    {  6,   2,   4,  2,  2,  4,   2,   6},
    { 11,   2,   5,  4,  4,  5,   1,  11},
    {-10, -20,   1,  2,  2,  2, -20, -10},
    {100, -10,  11,  6,  6, 11, -10, 100}
};

int calculate(OthelloBoard current){
    int points = 0;
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            if(current.board[i][j] == player){
                points += BoardPoint[i][j];
            }
            else if(current.board[i][j] == 3-player){
                points -= BoardPoint[i][j];
            }
        }
    }
    return points;
};

int loop(int x, int y, int plyr, OthelloBoard current){
    int points = 0;
    for(int i=1; i<7; i++){
        if(current.board[i][y]==player) points += 3*plyr;
        if(current.board[x][i]==player) points += 3*plyr;
    }
    return points;
};

int heuristicValue(OthelloBoard current){
    int points = 0;
    if(current.winner == player) points += 100;
    if(current.winner == 3 - player) points += 100;
    if(current.cur_player == player) points += current.next_valid_spots.size();

    points += calculate(current);

    if(current.board[0][0] == player){
        points += loop(0,0,1,current);
        if(current.board[1][1] == player) points += 2;
        if(current.board[0][1] == player) points += 3;
        if(current.board[1][0] == player) points += 3;
    }

    if(current.board[0][0] == 3 - player){
        points += loop(0,0,-1,current);
    }

    if(current.board[7][7] == player){
        points += loop(7,7,1,current);
        if(current.board[6][6] == player) points += 2;
        if(current.board[7][6] == player) points += 3;
        if(current.board[6][7] == player) points += 3;
    }

    if(current.board[7][7] == 3 - player){
        points += loop(7,7,-1,current);
    }
    
    if(current.board[0][7] == player){
        points += loop(0,7,1,current);
        if(current.board[1][6] == player) points += 2;
        if(current.board[0][6] == player) points += 3;
        if(current.board[1][7] == player) points += 3;
    }

    if(current.board[0][7] == 3 - player){
        points += loop(0,7,-1,current);
    }

    if(current.board[7][0] == player){
        points += loop(7,7,1,current);
        if(current.board[6][1] == player) points += 2;
        if(current.board[6][0] == player) points += 3;
        if(current.board[7][1] == player) points += 3;
    }
        
    if(current.board[7][0] == 3 - player){
        points += loop(7,0,-1,current);
    }

    return points;
};

class Node{
    public:
        Point p;
        int score;
        Node(){}
        Node(Point P, int Score) : p(P), score(Score) {}
};

Node MiniMax(const OthelloBoard curState, int depth, int alpha, int beta, bool isMax){
    if(depth == 0 || curState.done){
        return Node(Point(-1,-1),heuristicValue(curState));
    }
    if(curState.cur_player==player){
        Point P_Max = Point(-2,-2);
        int Max = INT_MIN;
        
        if(curState.next_valid_spots.empty()){
            OthelloBoard next = curState;
            next.cur_player = 3 - next.cur_player;
            Node nextMoveMin = MiniMax(next, depth-1, alpha, beta, false);
            Max = max(Max, nextMoveMin.score);
            return Node(P_Max, Max);
        }
        for(Point valid_spot : curState.next_valid_spots){
            OthelloBoard nextState = curState;
            nextState.put_disc(valid_spot);
            Node nextMoveMin = MiniMax(nextState, depth-1, alpha, beta, false);
            if(nextMoveMin.score > Max){
                Max = nextMoveMin.score;
                P_Max = valid_spot;
            }
        }
        return Node(P_Max, Max);
    }
    else{
        Point P_Min = Point(-3,-3);
        int Min = INT_MAX;

        if(curState.next_valid_spots.empty()){
            OthelloBoard nextState = curState;
            nextState.cur_player = 3 - nextState.cur_player;
            Node nextMoveMax = MiniMax(nextState, depth-1, alpha, beta, true);
            Min = min(nextMoveMax.score,Min);
            return Node(P_Min, Min);
        }
        for(Point valid_spot : curState.next_valid_spots){
            OthelloBoard nextState = curState;
            nextState.put_disc(valid_spot);
            Node nextMoveMax = MiniMax(nextState, depth-1, alpha, beta, true);
            if(nextMoveMax.score < Min){
                Min = nextMoveMax.score;
                P_Min = valid_spot;
            }
        }
        return Node(P_Min, Min);
    }  
}

void read_board(std::ifstream& fin) {
    fin >> player;
    now.cur_player = player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
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
    Node bestp = MiniMax(now, 6, INT_MIN, INT_MAX, true);
    // Remember to flush the output to ensure the last action is written to file.
    fout << bestp.p.x << " " << bestp.p.y << std::endl;
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

