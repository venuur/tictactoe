#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <iterator>
#include <string>
#include <chrono>
#include <sstream>
#include <memory>
#include <unordered_set>
#include <utility>
#include <iomanip>
#include <unordered_map>

using std::ostream;
using std::vector;
using std::array;
using std::cout;
using std::cerr;
using std::endl;
using std::default_random_engine;
using std::uniform_int_distribution;
using std::size;
using std::random_device;
using std::string;
using std::chrono::system_clock;
using std::stringstream;
using std::unique_ptr;
using std::unordered_set;
using std::unordered_map;

static const int TIE = 0;
static const int PLAYING = -1;
static const int EMPTY = 0;
static random_device global_rng;


// Manipulation for player values of 1 and 2.
int other_player(int p) {
	return (p % 2) + 1;
}

// Manipulation for indexes 0 and 1 corresponding to player 1 and 2.
int other_player_index(int p) {
	return (p + 1) % 2;
}


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
			status(PLAYING),
			m_next_player(1) {}
		
	Board(const vector<int>& b) : 
			status(PLAYING),
			m_next_player(1){
		board = b;
		update_status();
		for (int pos : b) {
			if (pos > 0) {
				m_next_player = other_player(m_next_player);
			}
		}
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
		m_next_player = other_player(m_next_player);
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
	
	int next_player() {
		return m_next_player;
	}
	
	int next_player_idx() {
		return m_next_player - 1;
	}

private:
	vector<int> board;
	int status;
	int m_next_player;
	
	void update_status();
};


class Player {
	public:
		virtual ~Player() {};
		virtual Move next_move(const Board&) = 0;
};


class Tictactoe {
public:
	vector<Move> action_log;
	Board board;
	
	Tictactoe(Player* p1, Player* p2) :
			players({p1, p2}),
			next_player_idx(0) {}
			
	Tictactoe(Player* p1, Player* p2, Board initial) :
			board(initial), 
			players({p1, p2}),
			next_player_idx(board.next_player_idx()) {}
			
	void play();
	friend ostream& operator<<(ostream& os, const Tictactoe& game);
	
private:
	array<Player*, 2> players;
	int next_player_idx;
};


class RandomPlayer : public Player {
public:
	RandomPlayer(int p) : 
			player(p) {
		// Use random device to create random sequence for seeds.
		// Use system clock to get different sequence each time.
		unsigned int random_seed = 
			global_rng() + system_clock::now().time_since_epoch().count();
		seed = random_seed;
		generator.seed(seed);
	}
		
	RandomPlayer(int p, unsigned int s) : 
			player(p),
			seed(s),
			generator(seed) {}

	virtual Move next_move(const Board& b) {
		vector<Move> moves = b.valid_moves(player);
		uniform_int_distribution<int> idx_dist(0, size(moves) - 1);
		int random_index = idx_dist(generator);
		return moves[random_index];
	}
	
private:
	int player;
	unsigned int seed;
	default_random_engine generator;
};


class OneStepAheadPlayer : public Player {
public:
	OneStepAheadPlayer(int p) :
			player(p),
			random_alternative(player) {}
			
	virtual Move next_move(const Board& b) {
		vector<Move> moves = b.valid_moves(player);
		
		// Look for winning moves.
		for (Move m : moves) {
			Board next_board = b;
			next_board.apply_move(m);
			if (next_board.is_won()) {
				return m;
			}
		}
		
		// Look for blocking moves.
		int other = other_player(player);
		for (Move m : moves) {
			Board next_board = b;
			next_board.apply_move(Move(m.position, other));
			if (next_board.is_won()) {
				return m;
			}
		}
		
		// Default to a random move.
		return random_alternative.next_move(b);
	}
 
private:
	int player;
	RandomPlayer random_alternative;
};


class OneStepAheadMCSTPlayer : public Player {
public:
	OneStepAheadMCSTPlayer(
			int p, 
			int n = 100, 
			double win  = 1.0, 
			double tie  = 0.5,
			double loss = 0.0) : 
			player(p),
			n_samples(n),
			win_score(win),
			tie_score(tie),
			loss_score(loss),
			self(player),
			opponent(player) {}
			
	virtual Move next_move(const Board& b) {
		// Set initial move scores to zero.
		vector<Move> moves = b.valid_moves(player);
		unordered_map<int, double> move_scores;
		for (Move m : moves) {
			move_scores.insert({m.position, 0.0});
		}
		
		// Simulate games forward and accumulate scores.
		for (int i = 0; i < n_samples; i++) {
			Move next_move = self.next_move(b);
			Board next_board = b;
			next_board.apply_move(next_move);
			
			Tictactoe continuation(&self, &opponent, next_board);
			continuation.play();
			
			if (continuation.board.is_won()) {
				if (continuation.board.winning_player() == player) {
					move_scores[next_move.position] += win_score;
				} else {
					move_scores[next_move.position] += loss_score;
				} 
			} else {
				move_scores[next_move.position] += tie_score;
			}
		}
		
		// Pick highest scoring move.
		int max_position = moves[0].position;
		double max_score = move_scores[max_position];
		for (auto move_score : move_scores) {
			cout << "Move " << move_score.first
			     << " Score " << move_score.second
				 << endl;
			if (max_score < move_score.second) {
				max_position = move_score.first;
				max_score = move_score.second;
			}
		}
		
		Move selected_move = Move(max_position, player);
		cout << "Selected " << selected_move << endl;
		
		return selected_move;
	}
	
private:
	int player;
	int n_samples;
	double win_score;
	double tie_score;
	double loss_score;
	OneStepAheadPlayer self;
	OneStepAheadPlayer opponent;
};



void test_board_status();
void test_board_moves();
void test_random_moves();
void test_random_game();
void test();
void score_players( 
		string player_one_name, 
		string player_two_name,
		int n_games);
Player* find_player_by_name(string player_name, int player);
ostream& operator<<(ostream& os, const vector<Move>& moves);


class CLIHandler {
public:
	CLIHandler(int argc, char** argv) :
			n_args(argc),
			valid_player_names({
				"random", 
				"one_step_ahead",
				"one_step_ahead_mcst"}) {
		for (int i = 1; i < n_args; i++) {
			args.push_back(argv[i]);
		}
	}
	
	void run_command() {
		if (n_args < 2) {
			print_usage();
		} else if (args[0] == "test") {
			test();
		} else if (args[0] == "random") {
			test_random_game();
		} else if (args[0] == "score") {
			run_score();
		} else {
			print_usage();
		}
	}
	
	void run_score() {
		if (n_args < 5) {
			print_usage_score();
		} else {
			stringstream ss;
			int n_games;
			string player_one_name, player_two_name;
			ss << args[1];
			ss >> n_games;
			ss.str("");
			ss.clear();
			
			ss << args[2];
			ss >> player_one_name;
			if (valid_player_names.count(player_one_name) == 0) {
				cerr << "Player one name, "
				     << player_one_name
					 << ", not found." << endl;
				print_usage_score();
				return;
			}
			ss.str("");
			ss.clear();
			
			ss << args[3];
			ss >> player_two_name;
			if (valid_player_names.count(player_two_name) == 0) {
				cerr << "Player two name, "
				     << player_two_name
					 << ", not found." << endl;
				print_usage_score();
				return;
			}
			ss.str("");
			ss.clear();
			
			score_players(player_one_name, player_two_name, n_games);
		}
	}
	
	void print_usage();
	void print_usage_score();
	
private:
	int n_args;
	vector<string> args;
	unordered_set<string> valid_player_names;
};


int main(int argc, char** argv) {
	cout << "Tictactoe Engine" << endl;
	
	CLIHandler cli(argc, argv);
	
	cli.run_command();
	
	return 0;
}


void test() {
	test_board_status();
	test_board_moves();
	test_random_moves();
	test_random_game();
}

void score_players( 
		string player_one_name, 
		string player_two_name,
		int n_games=100) {
	// Metrics from player_one's perspective.
	int wins = 0;
	int losses = 0;
	int ties = 0;
	
	vector<vector<Move>> game_logs(n_games);
	
	for (int i = 0; i < n_games; i++) {
		unique_ptr<Player> player_one(find_player_by_name(
				player_one_name, 1));
		unique_ptr<Player> player_two(find_player_by_name(
				player_two_name, 2));
				
		Tictactoe game(player_one.get(), player_two.get());
		game.play();
		
		game_logs[i] = game.action_log;
		cout << game_logs[i] << " ";
		
		if (game.board.winning_player() == 1) {
			wins++;
			cout << "W";
		} else if (game.board.winning_player() == 2) {
			losses++;
			cout << "L";
		} else if (game.board.is_tie()) {
			ties++;
			cout << "T";
		}
		
		cout << endl;
	}
	
	double win_percent = static_cast<double>(wins) / n_games * 100;
	double loss_percent = static_cast<double>(losses) / n_games * 100;
	double tie_percent = static_cast<double>(ties) / n_games * 100;
	double mean_moves = 0;
	for (auto log : game_logs) {
		mean_moves += size(log);
	}
	mean_moves /= n_games;
	
	
	cout << "Wins (%) "
	     << "Losses (%) "
		 << "Ties (%) "
		 << "Mean Moves\n"
		 << std::setw(8) << win_percent  << " "
		 << std::setw(10) << loss_percent << " "
		 << std::setw(8) << tie_percent  << " "
		 << std::setw(10) << mean_moves
		 << endl;
}


Player* find_player_by_name(string player_name, int player) {
	if (player_name == "random") {
		return new RandomPlayer(player);
	} else if (player_name == "one_step_ahead") {
		return new OneStepAheadPlayer(player);
	} else if (player_name == "one_step_ahead_mcst") {
		return new OneStepAheadMCSTPlayer(player, 10000);
	} else {
		return 0; // If valid player not found.
	}
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


void CLIHandler::print_usage() {
	cout << "\nUsage: ./tictactoe.exe COMMAND COMMAND_ARGS\n\n"
	     << "COMMAND       One of the following:\n"
		 << "  test        Runs a series of tests of game engine features.\n"
		 << "  random      Plays game between to players randomly choosing moves.\n"
		 << "  score       Plays a game n times between two players and returns score by wins, losses, and ties by player one.\n\n"
		 << "COMMAND_ARGS  Arguments to each command.\n"
		 << "  test        None.\n"
		 << "  random      None.\n"
		 << "  score       n_games, player_one_name, player_two_name.\n\n"
		 << endl;
}

void CLIHandler::print_usage_score() {
	cout << "\nUsage: ./tictactoe.exe score n_games player_one_name player_two_name\n\n"
	     << "  n_games          Number of games to play.\n"
		 << "  player_one_name  Name of player one, one of random, one_step_ahead.  This determines the players move choices.\n"
		 << "  player_two_name  Name of player two, see player_one_name.\n\n"
		 << endl;
}


void Tictactoe::play() {
	// If game has already been played. Do nothing.
	if (!board.is_playing()) {
		return;
	}
	
	for (; 
			board.is_playing(); 
			next_player_idx = other_player_index(next_player_idx)) {
		Move m = players[next_player_idx]->next_move(board);
		board.apply_move(m);
		action_log.push_back(m);
		//cout << *this << endl;
	}
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

ostream& operator<<(ostream& os, const vector<Move>& moves) {
	for (int i = 0; i < size(moves)-1; i++) {
		os << moves[i] << " ";
	}
	os << moves[size(moves)-1];
	return os;
}