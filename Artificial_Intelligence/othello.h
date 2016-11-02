#include <unordered_set>
#include <climits>
#include <string>
#include <iostream>
#include <cstdlib>

#define BOARD_SIZE 8

using namespace std;

/*
 * The TreeNode class contains information for the nodes of the tree used in
 * alpha-beta pruning in the main function, so that an optimal next move can be
 * made.  Conceptually, a node represents a move that could be made in the game.
 */
class TreeNode {
public:
  int** board; // a BOARD_SIZE-by-BOARD_SIZE square containing 0s, 1s, and 2s
  TreeNode parent; // the TreeNode object that represented the state of the game
                   // before this move would be made
  int depth; // the depth in the decision tree at which this node resides.
  int player; // the player who would be taking their turn
  int x, y; // the x and y coordinates of the space that would be occupied by a
            // new piece
  unordered_set<TreeNode*> children; // a set containing all possible turns that
                                     // could be made after this one


  TreeNode(int** b, TreeNode par, int d, int plr) {
    board = b;
    parent = par;
    depth = d;
    player = plr;
  }
  
};

/*
 * This class contains functions that will be necessary for the playing of the
 * game that aren't relevant to the TreeNodes specifically.
 *
 * Descriptions for the functions can be found in Othello.cpp.
 */
class Othello {
public:
  static TreeNode* findBestNeighbor(TreeNode*, int);
  static bool canFlip(int**, int, int, int, int, int);
  static bool isLegal(int**, int, int, int);
  static int scoreOfBoard(TreeNode*, int, int, int);
  static void doFlip(int**, int, int, int, int, int);
  static int** makeMove(int**, int, int, int);
  static int howManyPiecesFlipped(int, int, int, int);
  static int countFlips(int**, int, int, int, int, int);

private:
  static bool isCorner(int, int);
  static bool isNextToCorner(int, int);
  static bool isSide(int, int);
};
