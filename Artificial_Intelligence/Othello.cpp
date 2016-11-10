#include "othello.h"

int Othello::our_color;

/*
 * Function: take_turn
 *
 * Description: This is the primary function that handles turn-taking for the
 *              Othello game.
 *
 * Inputs:
 *  - color: The color of the player whose turn it is.  1 is white; 2 is black.
 *  - game_board: A pointer to the GameBoard object on which to make a move.
 *  - depth_limit: The maximum allowable depth of the decision tree.
 *
 * Outputs:
 *  - forfeit: A pointer to a bool that indicates whether the user made an
 *             illegal move.  If this happens, the program declares victory by
 *             forfeit.
 *
 * Return value: True if the player passes or has no legal moves; false
 *               otherwise.
 */
bool Othello::take_turn(int color, GameBoard *game_board, int depth_limit,
			bool* forfeit) {
  
  *forfeit = false;

  if (our_color == color) {
    // If this is us, we create a TreeNode for the current board state and fill
    // out the tree...
    TreeNode* tree_root = new TreeNode(new GameBoard(*game_board), 0, color);
    create_decision_tree(tree_root, depth_limit);

    // ...and we either pass because we have no legal moves...
    if (tree_root->no_children()) {
      cout << "I pass!\n";
      delete tree_root;
      return true;
    }

    // ...or we perform alpha-beta pruning on the tree and find the best turn to
    // take.
    int best_value = alpha_beta(tree_root, INT_MIN, INT_MAX);
    TreeNode* best_move;
    for (auto child = tree_root->get_children()->begin();
	 child != tree_root->get_children()->end(); ++child) {
      if ((*child)->get_value() == best_value) {
	best_move = *child;
	break;
      }
    }

    // Once we've found the best move, we alter the main game board accordingly.
    bool legal_move = game_board->place_piece(color, best_move->get_x(),
					      best_move->get_y(), true);
    if (legal_move) {
      cout << "I placed a " << (color == 2 ? "black" : "white") <<
	" piece at (" << best_move->get_x() << ", " << best_move->get_y() <<
	")!\n";
    }
    else {
      cout << "I pass!\n";
    }
    delete tree_root;
  }
  else {
    // If this is not us, we read input from cin and adjust the main game board
    // on behalf of the user, unless the user passes or makes an illegal move.
    int x, y;
    cin >> x >> y;
    if ((x < 0) || (x >= BOARD_SIZE) || (y < 0) || (y >= BOARD_SIZE)) {
      return true;
    }
    if (!game_board->is_legal(color, x, y)) {
      *forfeit = true;
      return true;
    }
    game_board->place_piece(color, x, y, true);
  }

  return false;
  
}

/*
 * Function: create_decision_tree
 *
 * Description: This function searches for legal moves to make from
 *              root->game_board and creates child TreeNodes for root, provided
 *              depth_limit is not reached.
 *
 * Inputs:
 *  - root: A pointer to a TreeNode object that will be considered the root of
 *          the decision tree being created.
 *  - depth_limit: If positive, this is the maximum allowable depth of the tree.
 *                 Otherwise, signifies that there is no maximum depth.
 */
void Othello::create_decision_tree(TreeNode* root, int depth_limit) {

  // No maximum depth limit.
  if (depth_limit <= 0) depth_limit = INT_MAX;

  // No more levels allowed in the currently forming tree.
  if (root->get_depth() >= depth_limit) return;

  // For each space on the board, if it's a legal move, we create a child for it
  // and recursively call create_decision_tree to fill out its subtree.
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (root->get_board()->is_legal(root->get_color(), i, j)) {
	GameBoard* child_board = new GameBoard(*(root->get_board()));
	child_board->place_piece(root->get_color(), i, j, true);
	TreeNode* child = new TreeNode(child_board, root->get_depth() + 1,
				       3 - root->get_color(), i, j);
	root->add_to_children(child);
	create_decision_tree(child, depth_limit);
      }
    }
  }
  return;
}

/*
 * Function: alpha_beta
 *
 * Description: This is the main decision-making function employed by the
 *              program.  It takes the root of a decision tree, which it assumes
 *              to be as full as is allowed, and performs alpha-beta pruning to
 *              find the best possible heuristic score for the player making the
 *              decision at the root of the tree.
 *
 * Inputs:
 *  - root: This is a pointer to the root of the tree being evaluated.
 *  - alpha: The best possible heuristic score for the black player found thus
 *           far.
 *  - beta: The best possible heuristic score for the white player found thus
 *          far.
 *
 * Return value: The heuristic score that would result from the best move by the
 *               player.
 */
int Othello::alpha_beta(TreeNode* root, int alpha, int beta) {
  if (root->no_children()) {
    root->set_value(root->get_board()->weighted_score_of_board());
    return root->get_value();
  }

  if (root->get_color() == 2) {
    int value = INT_MIN;
    for (auto child = root->get_children()->begin();
	 child != root->get_children()->end(); ++child) {
      value = max(value, alpha_beta(*child, alpha, beta));
      alpha = max(alpha, value);
      if (beta <= alpha) break;
    }
    root->set_value(value);
    return value;
  }
  else {
    int value = INT_MAX;
    for (auto child = root->get_children()->begin();
	 child != root->get_children()->end(); ++child) {
      value = min(value, alpha_beta(*child, alpha, beta));
      beta = min(beta, value);
      if (beta <= alpha) break;
    }
    root->set_value(value);
    return value;
  } 
}

/*
 * Function: set_color
 *
 * Description: Changes Othello::our_color, which is the color being used by the
 *              program.
 *
 * Inputs:
 *  - c: The new value to be stored in Othello::our_color.
 */
void Othello::set_color(int c) {our_color = c;}

/*
 * Function: is_corner
 *
 * Description: This function discerns whether a given space on a board is a
 *              corner space.
 *
 * Inputs:
 *  - x: The x-coordinate of the space in question.
 *  - y: The y-coordinate of the space in question.
 *
 * Return value: True if the space is a corner space; false otherwise.
 */
bool Othello::is_corner(int x, int y) {
  return ((x == 0) || (x == BOARD_SIZE - 1)) &&
    ((y == 0) || (y == BOARD_SIZE - 1));
}

/*
 * Function: is_next_to_corner
 *
 * Description: This function discerns whether a given space on a board is
 *              adjacent to a corner space, not counting diagonally adjacent
 *              spaces.  For example, on an 8 by 8 board, (0, 1), (1, 0),
 *              (0, 6), (1, 7), (6, 7), (7, 6), (6, 0), and (7, 1) would be the
 *              only spaces that would return true.
 *
 * Inputs:
 *  - x: The x-coordinate of the space in question.
 *  - y: The y-coordinate of the space in question.
 *
 * Return value: True if the space is adjacent to a corner space; false
 *               otherwise.
 */
bool Othello::is_next_to_corner(int x, int y) {
  return ((x <= 1) || (x >= BOARD_SIZE - 2)) &&
    ((y <= 1) || (y >= BOARD_SIZE - 2)) &&
    !is_corner(x, y) && is_side(x, y);
}

/*
 * Function: is_side
 *
 * Description: This function discerns whether a given space on a board is a
 *              side space.
 *
 * Inputs:
 *  - x: The x-coordinate of the space in question.
 *  - y: The y-coordinate of the space in question.
 *
 * Return value: True if the space is a side space; false otherwise.
 */
bool Othello::is_side(int x, int y) {
  return (((x == 0) || (x == BOARD_SIZE - 1)) &&
	  (y >= 2) && (y <= BOARD_SIZE - 3)) ||
    (((y == 0) || (y == BOARD_SIZE - 1)) &&
     (x >= 2) && (x <= BOARD_SIZE - 3));
}
