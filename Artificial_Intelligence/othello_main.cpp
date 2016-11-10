#include "othello.h"

/*
 * This is the main function for the Othello game.
 */
int main(int argc, char **argv) {

  // Parse input line.
  int our_color, depth_limit;
  parse_initial_input(&our_color, &depth_limit);
  Othello::set_color(our_color);
  bool isDepthLimited = (depth_limit <= 0);

  // Create and initialize the main game board.
  GameBoard* game_board = new GameBoard;

  // Main turn-taking loop.  If there is ever a situation in which neither player
  // can make a move, the game is over.
  bool blackPassed = false, whitePassed = false;
  bool forfeit = false;
  while (true) {
    blackPassed = Othello::take_turn(2, game_board, depth_limit, &forfeit);
    if ((blackPassed && whitePassed) || forfeit) break;
    whitePassed = Othello::take_turn(1, game_board, depth_limit, &forfeit);
    if ((blackPassed && whitePassed) || forfeit) break;
  }

  // This occurs if the user input an illegal move.
  if (forfeit) {
    cout << "That's not a legal move!  I win by forfeit!\n";
    delete game_board;
    return 0;
  }
  
  // Once the game is over, display the results.
  cout << "The game is over!\n";
  int score = game_board->raw_score_of_board();
  if (score == 0) {
    cout << "It's a tie!\n";
  }
  else if (score > 0) {
    cout << "Black wins by " << score << " points!\n";
  }
  else {
    cout << "White wins by " << score * -1 << " points!\n";
  }

  // Delete main game board.
  delete game_board;

  return 0;
}

/*
 * Function: parse_initial_input
 *
 * Description: This is a helper function for the main function that reads input
 *              provided by the user, to determine which color the program
 *              should be and what depth the program's decision-making tree
 *              should be.
 *
 * Outputs:
 *  - our_color: The color (2 for black, 1 for white) that the program should
 *               use will be assigned to this pointer.
 *  - depth_limit: The maximum depth of the decision-making tree will be stored
 *                 in this pointer.
 */
void parse_initial_input(int* our_color, int* depth_limit) {
  while (true) {
    cout << "Would you like me to be black (B) or white (W)?\n";
    char color;
    cin >> color;
    if (color == 'B') {
      *our_color = 2;
      break;
    }
    else if (color == 'W') {
      *our_color = 1;
      break;
    }
    else {
      cout << "That's not a valid color!\n";
    }
  }

  cout << "What should the maximum depth of my pruning tree be? For no " <<
    "maximum depth, enter 0 or a negative number.\n";
  cout << "NOTE: It is strongly recommended that you have a maximum depth " <<
    "of no more than 6.\n";
  cin >> *depth_limit;
  return;
}
