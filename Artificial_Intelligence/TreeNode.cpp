#include "othello.h"

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
