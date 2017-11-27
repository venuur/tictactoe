#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define N_COLS 3
#define N_ROWS N_COLS
#define BOARD_SIZE N_ROWS*N_COLS
#define ACTION_SIZE 3		/* Includes null terminator. */
#define N_ACTIONS BOARD_SIZE + 1 /* All moves and user commands. */
#define LINE_BUFFER_SIZE 80
#define INVALID_ACTION {INVALID, "", 0}

static char WELCOME_MESSAGE[] =
  "Welcome to Tic-Tac-Toe!\n\n"
  "In this game you enter your move as \"<row><col>\"\n"
  "where row and col are the row letter a, b, or c\n"
  "and col number 1, 2, or 3.\n\n";

static const char ACTION_PROMPT[] =
  "Please enter the position for you move (q to quit).\n";


enum board_mark {EMPTY=0, OH, EX};
typedef enum board_mark board_mark;
static char BOARD_SYMBOLS[] = {' ', 'O', 'X'};

typedef enum action_type {INVALID, COMMAND_QUIT, MOVE} action_type;
typedef struct Action {
  action_type type;
  char label[ACTION_SIZE];
  int index;
} Action;

static const Action ACTIONS[] = {
  {COMMAND_QUIT, "q\0", -1},	      /* Indexes don't matter for commands. */
  {MOVE, "a1", 0},
  {MOVE, "a2", 1},
  {MOVE, "a3", 2},
  {MOVE, "b1", 3},
  {MOVE, "b2", 4},
  {MOVE, "b3", 5},
  {MOVE, "c1", 6},
  {MOVE, "c2", 7},
  {MOVE, "c3", 8},
  INVALID_ACTION		/* Guard end of array with invalid action. */
};


typedef enum Player {PLAYER_EX, PLAYER_OH} Player;


typedef enum GameStatus {PLAYING, TIE, WIN} GameStatus;


typedef struct ActionLogElem {
  Action action;
  ActionLogElem* next_action_log_elem;
} ActionLogElem;

typedef struct ActionLog {
  Player starting_player;
  GameStatus result;
  ActionLogElem* action_list;
  ActionLogElem* end_action_list;
} ActionLog;

ActionLogElem* create_action_log_elem(Action action);
ActionLog create_action_log(Player starting_player);
void free_action_log(ActionLog action_log);
void free_action_list(ActionLogElem* action_list);
void append_action_log(ActionLog* action_log, Action action);
ActionLogElem* create_action_log_elem(Action action);


/* ActionLog related declarations. */
ActionLog create_action_log(Player starting_player) {
  ActionLog action_log;
  action_log.starting_player = starting_player;
  action_log.result = PLAYING;
  action_log.action_list = 0;
  action_log.end_action_list = 0;
  return action_log;
}

void free_action_log(ActionLog action_log) {
  free_action_list(action_log.action_list);
  action_log.end_action_list = 0;
}

void free_action_list(ActionLogElem* action_list) {
  ActionLogElem* tmp;

  while (action_list) {
    tmp = action_list;
    action_list = action_list->next_action_log_elem;
    free(tmp);
  }
}

void append_action_log(ActionLog* action_log, Action action) {
  ActionLogElem* new_action_log_elem = create_action_log_elem(action);
  if (action_log->action_list) {
    action_log->end_action_list->next_action_log_elem = new_action_log_elem;
    action_log->end_action_list = new_action_log_elem
  } else {
    action_log->action_list = new_action_log_elem;
    action_log->end_action_list = new_action_log_elem;
  }
}


ActionLogElem* create_action_log_elem(Action action) {
  ActionLogElem* new_elem = malloc(sizeof(ActionLogElem));
  new_elem->action = action;
  new_elem->next_action_log_elem = 0;
  return new_elem;
}


/**************************************************************** 
 ** Forward Declarations
 ****************************************************************/
  
void run_game();
void print_board(board_mark board[BOARD_SIZE]);
void print_board_row(board_mark board[BOARD_SIZE], int row);
void apply_action(board_mark board[BOARD_SIZE], Action action, Player active_player);
GameStatus check_game_status(board_mark board[BOARD_SIZE]);
void print_winning_player_message(Player active_player);
board_mark mark_from_player(Player p);
Player change_players(Player old_player);
Action get_action();
Action action_from_str(const char action_str[]);
bool is_valid_action(board_mark board[BOARD_SIZE], Action action, char reason[]);
void get_user_line(char response[], int response_size);



/**************************************************************** 
 ** Begin Program
 ****************************************************************/

int main(int argc, char** argv) {
  run_game();
  return 0;
}


void run_game() {
  static board_mark board[BOARD_SIZE] = {EMPTY};
  static char reason[256];
  static GameStatus status = PLAYING;

  printf(WELCOME_MESSAGE);
  Player active_player = PLAYER_OH;

  while(1) {
    print_board(board);
  
    printf(ACTION_PROMPT);
    Action action = get_action();
    printf("DEBUG: Your action is [%s]\n", action.label);

    if (action.type == COMMAND_QUIT) break;

    bool valid_action = is_valid_action(board, action, reason);

    if (valid_action) {
      apply_action(board, action, active_player);
      status = check_game_status(board);

      if (status == PLAYING) {
	active_player = change_players(active_player);
      } else {
	/* If not playing then end game. */
	break;
      }
    } else {
      printf(reason);
    }
  }

  print_board(board);

  
  /* If the game was not terminated by quit, show the result. */
  switch (status)  {
  case PLAYING:
    printf("\nThanks for playing!\n");
    break;
  case TIE:
    printf("\nThe game was a tie.\n");
    break;
  case WIN:
    print_winning_player_message(active_player);
    break;
  };    
}


void print_board(board_mark board[BOARD_SIZE]) {
  static char board_top[] = " 1 2 3";
  static char board_middle[] = " -+-+-";

  printf("%s\n", board_top);

  int i;
  for(i = 0; i < N_ROWS-1; ++i) {
    print_board_row(board, i);
    printf("%s\n", board_middle);
  }
  print_board_row(board, i);
}



void print_board_row(board_mark board[BOARD_SIZE], int row) {
  static char board_row_markers[] = "abc";
  printf("%c", board_row_markers[row]);
  printf("%c|", BOARD_SYMBOLS[board[row*N_COLS + 0]]);
  printf("%c|", BOARD_SYMBOLS[board[row*N_COLS + 1]]);
  printf("%c\n", BOARD_SYMBOLS[board[row*N_COLS + 2]]);
}


Action get_action() {
  char action_str[ACTION_SIZE];
  Action action;
  
  /* Ask for input until a valid action is found. */
  while(1) {
    get_user_line(action_str, 3);
    action = action_from_str(action_str);
    
    if (action.type != INVALID) {
      break;
    } else {
      printf("Your action [%s] is not valid. Please enter 'q' or \"[abc][123]\", e.g. a1\n",
	     action_str);
    }
  }

  return action;
}


void apply_action(board_mark board[BOARD_SIZE], Action action, Player active_player) {
  board_mark mark = mark_from_player(active_player);
  board[action.index] = mark;
}


GameStatus check_game_status(board_mark board[BOARD_SIZE]) {
  static int oh = 0;
  static int ex = 1;
  static int a1_diag = 0;
  static int anti_diag = 1;
  bool rows_all[2][N_ROWS] = {
    {true, true, true},
    {true, true, true}
  };
  bool cols_all[2][N_COLS] = {
    {true, true, true},
    {true, true, true}
  };
  bool diags_all[2][2] = {
    {true, true},
    {true, true}
  };
  bool any_empty = false;

  /* Check each cell and update states. */
  for (int i = 0; i < BOARD_SIZE; ++i) {
    int row = i / N_COLS;
    int col = i % N_ROWS;
    bool in_a1_diag = row == col;
    bool in_anti_diag = row + col == N_ROWS - 1;
    
    /* Check rows and columns. */
    switch (board[i]) {
    case EMPTY:
      any_empty = true;
      rows_all[ex][row] = false;
      cols_all[ex][col] = false;
      rows_all[oh][row] = false;
      cols_all[oh][col] = false;
      break;
    case OH:
      rows_all[ex][row] = false;
      cols_all[ex][col] = false;
      break;
    case EX:
      rows_all[oh][row] = false;
      cols_all[oh][col] = false;
      break;
    };

    /* Check diagonals. */
    if (in_a1_diag) {
      switch (board[i]) {
      case EMPTY:
	any_empty = true;
	diags_all[ex][a1_diag] = false;
	diags_all[oh][a1_diag] = false;
	break;
      case OH:
	diags_all[ex][a1_diag] = false;
      case EX:
	diags_all[oh][a1_diag] = false;
	break;
      };
    }
    if (in_anti_diag) {
      switch (board[i]) {
      case EMPTY:
	any_empty = true;
	diags_all[ex][anti_diag] = false;
	diags_all[oh][anti_diag] = false;
	break;
      case OH:
	diags_all[ex][anti_diag] = false;
      case EX:
	diags_all[oh][anti_diag] = false;
	break;
      };
    }
  }

  /* Check to see which condition holds. */
  bool is_win = false;

  /* rows and cols */
  for (int i = 0; i < N_ROWS; ++i) {
    is_win =
      rows_all[oh][i] ||
      cols_all[oh][i] ||
      rows_all[ex][i] ||
      cols_all[ex][i];
    if (is_win) break;
  }

  is_win = is_win ||
    diags_all[oh][a1_diag] ||
    diags_all[oh][anti_diag] ||
    diags_all[ex][a1_diag] ||
    diags_all[ex][a1_diag];

  if (is_win) {
    printf("DEBUG: is_win [true]\n");
    return WIN;
  } else if (!any_empty) {
    printf("DEBUG: is_win [false]\n");
    return TIE;
  }

  printf("DEBUG: PLAYING\n");
  return PLAYING;
}


void print_winning_player_message(Player active_player) {
  static char win_message[] =
    "\nPlayer %s won! Congratulations!\n";
  
  switch (active_player) {
  case PLAYER_OH:
    printf(win_message, "O");
    break;
  case PLAYER_EX:
    printf(win_message, "X");
    break;
  };
}


board_mark mark_from_player(Player p) {
  board_mark mark;

  switch (p) {
  case PLAYER_OH:
    mark = OH;
    break;
  case PLAYER_EX:
    mark = EX;
    break;
  }

  return mark;
}


Player change_players(Player old_player) {
  Player new_player;

  switch (old_player) {
  case PLAYER_OH:
    new_player = PLAYER_EX;
    break;
  case PLAYER_EX:
    new_player = PLAYER_OH;
    break;
  }

  return new_player;
}


void get_user_line(char response[], int response_size) {
  int c;
  int n_char = 0;
  while ((c = getchar()) != '\n' && c != EOF) {
    /* Capture characters up to requested response size, but don't
       capture the newline.  This gobbles any extra characters on the
       input line*/
    if (n_char < response_size-1 && c !='\n') {
      response[n_char] = c;
    }
    ++n_char;
  }
  response[n_char] = '\0';
}


Action action_from_str(const char action_str[]) {
  bool valid_action = false;
  const Action* action;
  for (action = ACTIONS; action->type != INVALID; ++action) {
    /* printf("DEBUG: action_str [%s], action->label [%s], cmp [%d]\n", */
    /* 	   action_str, */
    /* 	   action->label, */
    /* 	   strcmp(action_str, action->label)); */
    
    valid_action = strcmp(action_str, action->label) == 0;
    if (valid_action) break;
  }

  return *action;
}


bool is_valid_action(board_mark board[BOARD_SIZE], Action action, char reason[]) {
  static char reason_already_marked[] =
    "Square %s already has a mark. Please try again.\n";
  
  /* Cannot place mark over existing mark. */
  if (board[action.index] != EMPTY) {
    sprintf(reason, reason_already_marked, action.label);
    return false;
  }

  return true;
}
