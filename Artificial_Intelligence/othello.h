#include <unordered_set>
#include <climits>
#include <string>
#include <iostream>
//#include <queue>
//#include <array>
#include <iterator>

#define BOARD_SIZE 8

using namespace std;

void parse_initial_input(int*, int*);

/*
 * This class represents a game board, containing its current state.
 */
class GameBoard {

 private:
  int** game_board; // state

 public:
  // constructors and destructor
  GameBoard();
  GameBoard(const GameBoard&);
  ~GameBoard();

  // See GameBoard.cpp for descriptions.
  bool place_piece(int, int, int, bool);
  bool flip_pieces(int, int, int, int, int, bool);
  int raw_score_of_board();
  int weighted_score_of_board();
  bool is_legal(int, int, int);
};

/*
 * The TreeNode class contains information for the nodes of the tree used in
 * alpha-beta pruning in the main function, so that an optimal next move can be
 * made.  Conceptually, a node represents a move that could be made in the game.
 */
class TreeNode {
 private:
  GameBoard* board;
  int depth; // the depth in the decision tree at which this node resides
  int color; // the player who would be taking their turn
  int x, y; // the x and y coordinates of the space that would be occupied by a
            // new piece
  int value; // for alpha-beta pruning
  unordered_set<TreeNode*> *children; // a set containing all possible turns
                                      // that could be made immediately
                                      // following this one

 public:
  // constructors and destructor
  TreeNode(GameBoard*, int, int, int, int);
  TreeNode(GameBoard*, int, int);
  ~TreeNode();

  // See TreeNode.cpp for descriptions.
  GameBoard* get_board();
  int get_depth();
  int get_color();
  int get_x();
  int get_y();
  unordered_set<TreeNode*> *get_children();
  void add_to_children(TreeNode*);
  bool no_children();
  int get_value();
  void set_value(int);
};

/*
 * This class contains functions that will be necessary for the playing of the
 * game that aren't relevant to the TreeNodes or GameBoards specifically.
 */
class Othello {
 private:
  static int our_color; // the color of the program; 1 is white, 2 is black

 public:
  // See Othello.cpp for descriptions.
  static bool take_turn(int, GameBoard*, int, bool*);
  static void set_color(int);
  static int get_color();
  static void create_decision_tree(TreeNode*, int);
  static int alpha_beta(TreeNode*, int, int);
  static bool is_corner(int, int);
  static bool is_next_to_corner(int, int);
  static bool is_side(int, int);
};
