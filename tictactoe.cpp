#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <iterator>

using std::ostream;
using std::vector;
using std::cout;
using std::endl;
using std::default_random_engine;
using std::uniform_int_distribution;
using std::size;

static const int TIE = 0;
static const int PLAYING = -1;
static const int EMPTY = 0;


class Move {
public:
	int position;
	int player;
	
	Move(int pos, int p) :
			position(pos),
			player(p) {}
			
	friend ostream& operator<<(ostream& os, const Move& m);
};


class Board {
public:	
	Board() : 
			board(9, 0),
			status(PLAYING) {}
		
	Board(const vector<int>& b) : 
			status(PLAYING) {
		board = b;
		update_status();
	}
		
	friend ostream& operator<<(ostream& os, const Board& b);
	
	bool is_won() const {
		return status > 0;
	}
	
	bool is_tie() const {
		return status == TIE;
	}
	
	// Valid return only if Board::is_won returns true.
	int winning_player() const {
		return status;
	}
	
	void apply_move(Move m) {
		board[m.position] = m.player;
		update_status();
	}
	
	vector<Move> valid_moves(int player) {
		vector<Move> moves;
		
		for (auto b : board) {
			if (b == EMPTY) {
				moves.push_back(Move(b, player));
			} 
		}
		
		return moves;
	}

private:
	vector<int> board;
	int status;
	
	void update_status();
};


class Player {
	public:
		int player;
	
		virtual ~Player() {};	
		virtual Move next_move(Board) = 0;
}

class RandomPlayer : Player {
public:
	RandomPlayer(int p) : player(p) {}

	virtual Move next_move(Board b) {
		vector<Move> moves = b.valid_moves(player);
		uniform_int_distribution<int> idx_dist(0, size(moves));
		int random_index = idx_dist(generator);
		return moves[random_index];
	}
	
private:
	default_random_engine generator;
}


void test_board_status();
void test_board_moves();
void test_random_moves();


int main(int argc, char** argv) {
	cout << "Tictactoe Engine" << endl;
	
	test_board_status();
	test_board_moves();
	test_random_moves();
	
	return 0;
}


void test_board_status() {
	static const vector<vector<int>> test_boards = {
			{0, 0, 0, 0, 0, 0, 0, 0, 0},
			{1, 1, 1, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 1, 1, 1, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 1, 1, 1},
			{1, 0, 0, 1, 0, 0, 1, 0, 0},
			{0, 1, 0, 0, 1, 0, 0, 1, 0},
			{0, 0, 1, 0, 0, 1, 0, 0, 1},
			{1, 0, 0, 0, 1, 0, 0, 0, 1},
			{0, 0, 1, 0, 1, 0, 1, 0, 0},
			{2, 2, 2, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 2, 2, 2, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 2, 2, 2},
			{2, 0, 0, 2, 0, 0, 2, 0, 0},
			{0, 2, 0, 0, 2, 0, 0, 2, 0},
			{0, 0, 2, 0, 0, 2, 0, 0, 2},
			{2, 0, 0, 0, 2, 0, 0, 0, 2},
			{0, 0, 2, 0, 2, 0, 2, 0, 0},
			{1, 2, 2, 2, 1, 1, 1, 2, 2}
		};
	
	for (auto tb : test_boards) {
		Board b(tb);
		cout << b << endl;
	}
}


void test_board_moves() {
	Board b1, b2;
	static const vector<int> player_seq1 = {1, 2, 1, 2, 1, 2, 1, 2, 1};
	static const vector<int> player_seq2 = {2, 1, 2, 1, 2, 1, 2, 1, 2};
	static const vector<int> position_seq = {1, 0, 2, 4, 3, 5, 7, 6, 8};
	
	cout << b1;
	for (int i = 0; i < 9; i++) {
		Move m(position_seq[i], player_seq1[i]);
		b1.apply_move(m);
		cout << b1;
	}
	
	cout << b2;
	for (int i = 0; i < 9; i++) {
		Move m(position_seq[i], player_seq2[i]);
		b2.apply_move(m);
		cout << b2;
	}
}

void test_random_moves() {
	Board b;
	RandomPlayer p1(1);
	RandomPlayer p2(2);
	
	cout << b;
	for (int i = 0; i < 4; i++) {
		Move m = 
	}
}


ostream& operator<<(ostream& os, const Board& b) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			os << b.board[3*i + j]
			   << (j == 2 ? "\n" : "|");
		}
		if (i < 2) {
			os << std::string(5, '-')
			   << endl;
		}
	}
	
	if (b.is_won()) {
		os << "Player " << b.winning_player() << " wins." << endl;
	} else if (b.is_tie()) {
		os << "Tie." << endl;
	} else {
		os << "Playing." << endl;
	}
	
	return os;
}

void Board::update_status() {
	static const vector<int> players = {1, 2};
	
	bool player_won = false;
	for (auto player : players) {
		// Win by rows
		for (int row = 0; row < 3; row++) {
			player_won = player_won || 
				(board[3*row] == player) &&
				(board[3*row + 1] == player) &&
				(board[3*row + 2] == player);
		}
		
		// Win by columns
		for (int column = 0; column < 3; column++) {
			player_won = player_won ||
				(board[column] == player) &&
				(board[3 + column] == player) &&
				(board[6 + column] == player);
			
		}
		
		// Win by diagonals
		player_won = player_won ||
			((board[0] == player) &&
			 (board[3+1] == player) &&
			 (board[6+2] == player)) ||
			((board[2] == player) &&
			 (board[3+1] == player) &&
			 (board[6] == player));

		if (player_won) {
			status = player;
			return;
		}
	}
	
	bool board_tied = true;
	for (auto b : board) {
		if (b == EMPTY) {
			board_tied = false;
			break;
		}
	}
	if (board_tied) {
		status = TIE;
		return;
	}
	
	status = PLAYING;
}


ostream& operator<<(ostream& os, const Move& m) {
	os << m.player << "@" << m.position;
	return os;
}