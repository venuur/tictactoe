#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <iterator>

using std::ostream;
using std::vector;
using std::array;
using std::cout;
using std::endl;
using std::default_random_engine;
using std::uniform_int_distribution;
using std::size;
using std::random_device;

static const int TIE = 0;
static const int PLAYING = -1;
static const int EMPTY = 0;
static random_device global_rng;


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
	
	bool is_playing() const {
		return status == PLAYING;
	}
	
	// Valid return only if Board::is_won returns true.
	int winning_player() const {
		return status;
	}
	
	void apply_move(Move m) {
		board[m.position] = m.player;
		update_status();
	}
	
	vector<Move> valid_moves(int player) const {
		vector<Move> moves;
		
		for (int i = 0; i < 9; i++) {
			if (board[i] == EMPTY) {
				moves.push_back(Move(i, player));
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
		virtual ~Player() {};	
		virtual Move next_move(const Board&) = 0;
};

class RandomPlayer : public Player {
public:
	int player;
	
	RandomPlayer(int p) : 
			player(p) {
		unsigned int random_seed = global_rng();
		cout << "Using random seed: " << random_seed << endl;
		seed = random_seed;
		generator.seed(seed);
	}
		
	RandomPlayer(int p, unsigned int s) : 
			player(p),
			seed(s),
			generator(seed) {}

	virtual Move next_move(const Board& b) {
		vector<Move> moves = b.valid_moves(player);
		cout << "Valid Moves: ";
		for (Move m : moves) {
			cout << m << " ";
		}
		cout << endl;
		uniform_int_distribution<int> idx_dist(0, size(moves) - 1);
		int random_index = idx_dist(generator);
		return moves[random_index];
	}
	
private:
	unsigned int seed;
	default_random_engine generator;
};

class Tictactoe {
public:
	Tictactoe(Player* p1, Player* p2) : 
			players({p1, p2}) {}
			
	void play();
	friend ostream& operator<<(ostream& os, const Tictactoe& game);
	
private:
	Board board;
	array<Player*, 2> players;
	vector<Move> action_log;
};


void test_board_status();
void test_board_moves();
void test_random_moves();
void test_random_game();


int main(int argc, char** argv) {
	cout << "Tictactoe Engine" << endl;
	
	test_board_status();
	test_board_moves();
	test_random_moves();
	test_random_game();
	
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
		Move m = p1.next_move(b);
		cout << m << endl;
		b.apply_move(m);
		cout << b;
		m = p2.next_move(b);
		cout << m << endl;
		b.apply_move(m);
		cout << b;
	}
}

void test_random_game() {
	RandomPlayer p1(1), p2(2);
	Tictactoe(&p1, &p2).play();
}


void Tictactoe::play() {
	int player_idx = 0;
	do {
		Player* current_player = players[player_idx];
		Move m = current_player->next_move(board);
		board.apply_move(m);
		action_log.push_back(m);
		cout << *this;
		player_idx = (player_idx + 1) % 2;
	} while (board.is_playing());
}


ostream& operator<<(ostream& os, const Tictactoe& game) {
	cout << game.board;
	cout << "Moves: ";
	for (Move m : game.action_log) {
		cout << m << " ";
	}
	cout << endl;
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