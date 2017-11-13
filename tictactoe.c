#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define BOARD_SIZE 3
#define ACTION_SIZE 3		/* Includes null terminator. */
#define N_ACTIONS BOARD_SIZE*BOARD_SIZE + 1 /* All moves and user commands. */
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
typedef struct action_map {
  action_type type;
  char label[ACTION_SIZE];
  int index;
} action_map;

static const action_map ACTIONS[] = {
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

typedef enum player {PLAYER_EX, PLAYER_OH} player;


/**************************************************************** 
 ** Forward Declarations
 ****************************************************************/
  
void print_board(board_mark board[BOARD_SIZE][BOARD_SIZE]);
void print_board_row(board_mark board[BOARD_SIZE][BOARD_SIZE], int row);
void apply_action(board_mark board[BOARD_SIZE][BOARD_SIZE], action_map action, player active_player);
board_mark mark_from_player(player p);
player change_players(player old_player);
action_map get_action();
action_map action_from_str(const char action_str[]);
bool is_valid_action(char* action);
void get_user_line(char response[], int response_size);


/**************************************************************** 
 ** Begin Program
 ****************************************************************/

int main(int argc, char** argv) {
  static board_mark board[BOARD_SIZE][BOARD_SIZE] = {EMPTY};

  printf(WELCOME_MESSAGE);
  player active_player = PLAYER_OH;

  while(1) {
    print_board(board);
  
    printf(ACTION_PROMPT);
    action_map action = get_action();
    printf("Your action is [%s]\n", action.label);

    if (action.type == COMMAND_QUIT) break;

    apply_action(board, action, active_player);
    active_player = change_players(active_player);
  }

  printf("\nThanks for playing!\n");
  
  return 0;
}


void print_board(board_mark board[BOARD_SIZE][BOARD_SIZE]) {
  static char board_top[] = " 1 2 3";
  static char board_middle[] = " -+-+-";

  printf("%s\n", board_top);

  int i;
  for(i = 0; i < BOARD_SIZE-1; ++i) {
    print_board_row(board, i);
    printf("%s\n", board_middle);
  }
  print_board_row(board, i);
}


void print_board_row(board_mark board[BOARD_SIZE][BOARD_SIZE], int row) {
  static char board_row_markers[] = "abc";
  printf("%c", board_row_markers[row]);
  printf("%c|%c|%c\n",
	 BOARD_SYMBOLS[board[row][0]],
	 BOARD_SYMBOLS[board[row][1]],
	 BOARD_SYMBOLS[board[row][2]]);
}


action_map get_action() {
  char action_str[ACTION_SIZE];
  action_map action;
  
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


void apply_action(board_mark board[BOARD_SIZE][BOARD_SIZE], action_map action, player active_player) {
  int row = action.index / BOARD_SIZE;
  int col = action.index % BOARD_SIZE;

  printf("DEBUG action.index [%d], row [%d], col [%d]\n",
	 action.index, row, col);

  board_mark mark = mark_from_player(active_player);
  board[row][col] = mark;
}


board_mark mark_from_player(player p) {
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


player change_players(player old_player) {
  player new_player;

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


action_map action_from_str(const char action_str[]) {
  bool valid_action = false;
  const action_map* action;
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
