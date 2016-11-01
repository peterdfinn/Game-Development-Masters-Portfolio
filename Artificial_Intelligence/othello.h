#include <unordered_set>
#include <climits>

#define BOARD_SIZE 8

using namespace std;

class TreeNode {

public:
  int** board;
  TreeNode parent;
  int depth;
  int player;
  int x, y;
  unordered_set<TreeNode*> children;

  TreeNode(int** b, TreeNode par, int d, int plr) {
    board = b;
    parent = par;
    depth = d;
    player = plr;
  }
  
};

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
