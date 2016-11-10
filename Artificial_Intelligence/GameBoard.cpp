#include "othello.h"

/*
 * This is the default constructor for the GameBoard class.
 */
GameBoard::GameBoard() {
  game_board = new int*[BOARD_SIZE];
  for (int i = 0; i < BOARD_SIZE; ++i) {
    game_board[i] = new int[BOARD_SIZE];
  }
  
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      game_board[i][j] = 0;
    }
  }
  game_board[(BOARD_SIZE / 2) - 1][(BOARD_SIZE / 2) - 1] = 2; // black
  game_board[BOARD_SIZE / 2][BOARD_SIZE / 2] = 2;
  game_board[BOARD_SIZE / 2][(BOARD_SIZE / 2) - 1] = 1; // white
  game_board[(BOARD_SIZE / 2) - 1][BOARD_SIZE / 2] = 1;
}

/*
 * This is the copy constructor for the GameBoard class.
 */
GameBoard::GameBoard(const GameBoard &gb) {
  game_board = new int*[BOARD_SIZE];
  for (int i = 0; i < BOARD_SIZE; ++i) {
    game_board[i] = new int[BOARD_SIZE];
  }

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      game_board[i][j] = gb.game_board[i][j];
    }
  }
}

/*
 * This is the destructor for the GameBoard class.
 */
GameBoard::~GameBoard() {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    delete[] game_board[i];
  }
  delete[] game_board;
}

/*
 * Function: place_piece
 *
 * Description: This function places a given piece of a given color on the
 *              board and flips all pieces that will now need to be flipped.
 *
 * Inputs:
 *  - color: The color of the piece to be placed.  1 represents white, and 2
 *           represents black.
 *  - x: The x-coordinate of the space onto which to place the piece (indexed
 *       starting from 0).
 *  - y: The y-coordinate of the space onto which to place the piece (indexed
 *       starting from 0).
 *  - do_flip: A piece will be placed, and existing pieces will be flipped, only
 *    if this boolean is true.
 *
 * Return value:
 *  - If true, that means the move made was a legal move.
 *  - If false, the move that would have been made was an illegal move.
 */
bool GameBoard::place_piece(int color, int x, int y, bool do_flip) {
  
  // If the space is already occupied, do nothing and return false.
  if (game_board[x][y] != 0) {
    return false;
  }

  // For all directions away from (x, y), check to see if there are pieces of
  // the opposite color that can be flipped, and flip them (if do_flip).
  bool is_legal = false;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (((i != 0) || (j != 0)) && flip_pieces(color, x, y, i, j, do_flip)) {
        is_legal = true;
      }
    }
  }

  // Place a piece at (x, y).
  if (is_legal && do_flip) game_board[x][y] = color;
  
  return is_legal;
}

/*
 * Function: flip_pieces
 *
 * Description: This function will flip pieces in the direction specified by
 *  dir_x and dir_y, changing them to a given color, if do_flip is true.  It was
 *  written as a helper function for place_piece.
 *
 * Inputs:
 *  - color: The color to which to change specified pieces.  1 represents white,
 *           and 2 represents black.
 *  - x: The x-coordinate of the space onto which to place a piece (indexed
 *       starting from 0).
 *  - y: The y-coordinate of the space onto which to place a piece (indexed
 *       starting from 0).
 *  - dir_x: This is an integer ranging from -1 to 1 indicating the direction in
 *           which, from (x, y), to flip pieces.  A value of -1 indicates that
 *           pieces should be flipped in the direction of x decreasing, a value
 *           of 1 indicates that pieces should be flipped in the direction of x
 *           increasing, and a value of 0 indicates that flips will not occur
 *           along the x axis.
 *  - dir_y: This is an integer ranging from -1 to 1.  It is defined analogously
 *           to dir_x.
 *  - do_flip: Pieces will not be flipped unless this boolean is true.
 *
 * Return value:
 *  - If true, the flipping of pieces will have been part of a legal move.
 *  - If false, the flipping of pieces will have been part of an illegal move.
 */
bool GameBoard::flip_pieces(int color, int x, int y, int dir_x, int dir_y,
			    bool do_flip) {

  int orig_x = x, orig_y = y;
  int piece_counter = 0;
  
  while (true) {
    // Move x and y in the specified direction.
    x += dir_x;
    y += dir_y;

    // If we hit an empty space or the edge of the board, this wouldn't be a
    // legal move in this direction.
    if ((x >= BOARD_SIZE) || (x < 0)) return false;
    if ((y >= BOARD_SIZE) || (y < 0)) return false;
    if (game_board[x][y] == 0) return false;

    // If we run into a piece of our own color, that means we're flanking pieces
    // of the opposite color, and this is would be a legal move.
    if (game_board[x][y] == color) {
      break;
    }
    ++piece_counter;
  }

  // This statement kicks in if there aren't any pieces that would be flipped,
  // meaning the move wouldn't be a legal move in this direction.
  if (piece_counter == 0) return false;

  // If it would be legal, flip the pieces.
  if (do_flip) {
    for (x = orig_x + dir_x, y = orig_y + dir_y; game_board[x][y] != color;
	 x += dir_x, y += dir_y) {
      game_board[x][y] = color;
    }
  }

  return true;
}

/*
 * Function: raw_score_of_board
 *
 * Description: This function tallies up the score of the board for the purpose
 *              of deciding who has won the game.
 *
 * Return value: The number of white pieces subtracted from the number of black
 *               pieces.  If positive, black has won; if negative, white has
                 won; if zero, the game has ended in a tie.
 */
int GameBoard::raw_score_of_board() {
  int black_count = 0, white_count = 0;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (game_board[i][j] == 2) ++black_count;
      if (game_board[i][j] == 1) ++white_count;
    }
  }
  return black_count - white_count;
}

/*
 * Function: weighted_score_of_board
 *
 * Description: This function is used to measure the score of the board for the
 *              purpose of minimax move prediction.  Spaces on the board are
 *              assigned a heuristic score.  Corner spaces are worth 5 points,
 *              spaces next to corners are worth 2 points, spaces on sides are
 *              worth 3 points, and all other spaces are worth 1 point.
 *
 * Return value: the score of the board in its current state as measured by the
 *               aforementioned heuristic.  A positive value indicates that
 *               black is in a better position than white, and a negative value
 *               indicates that white is in a better position than black.
 */
int GameBoard::weighted_score_of_board() {
  int total = 0;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (Othello::is_corner(i, j)) {
	if (game_board[i][j] == 2) total += 5;
	if (game_board[i][j] == 1) total -= 5;
      }
      else if (Othello::is_next_to_corner(i, j)) {
	if (game_board[i][j] == 2) total += 2;
	if (game_board[i][j] == 1) total -= 2;
      }
      else if (Othello::is_side(i, j)) {
	if (game_board[i][j] == 2) total += 3;
	if (game_board[i][j] == 1) total -= 3;
      }
      else {
	if (game_board[i][j] == 2) total += 1;
	if (game_board[i][j] == 1) total -= 1;
      }
    }
  }
  return total;
}

/*
 * Function: is_legal
 *
 * Description: This function determines whether hypothetically placing a piece
 *              of a given color on a given space would be a legal move.
 *
 * Inputs:
 *  - color: The color of the piece to be placed.  2 means black, 1 means white.
 *  - x: The x-coordinate of the space onto which the piece would be placed.
 *  - y: The y-coordinate of the space onto which the piece would be placed.
 *
 * Return value:
 *  - If true, the move would be legal.  If false, the move would be illegal.
 */
bool GameBoard::is_legal(int color, int x, int y) {
  return place_piece(color, x, y, false);
}
