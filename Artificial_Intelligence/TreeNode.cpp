#include "othello.h"

/*
 * Constructors for TreeNode.
 */
TreeNode::TreeNode(GameBoard* b, int d, int c) {
    board = b;
    depth = d;
    color = c;
    children = new unordered_set<TreeNode*>;

    // Values of -1 indicate that this TreeNode is the root of a decision tree.
    x = -1;
    y = -1;
}

TreeNode::TreeNode(GameBoard* b, int d, int c, int X, int Y) {
    board = b;
    depth = d;
    color = c;
    children = new unordered_set<TreeNode*>;
    x = X;
    y = Y;
}

/*
 * Destructor for TreeNode.
 */
TreeNode::~TreeNode() {
  for (auto child = children->begin(); child != children->end(); ++child) {
    delete *child;
  }
  delete board;
}

/*
 * Function: add_to_children
 *
 * Description: This function adds a child TreeNode to this TreeNode object.
 *
 * Inputs:
 *  - child: A pointer to the child to be added.
 */
void TreeNode::add_to_children(TreeNode* child) {
  children->insert(child);
  return;
}

unordered_set<TreeNode*> *TreeNode::get_children() {return children;}
int TreeNode::get_depth() {return depth;}
int TreeNode::get_color() {return color;}
bool TreeNode::no_children() {return children->empty();}
GameBoard* TreeNode::get_board() {return board;}
int TreeNode::get_value() {return value;}
void TreeNode::set_value(int v) {value = v;}
int TreeNode::get_x() {return x;}
int TreeNode::get_y() {return y;}
