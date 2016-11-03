#include "othello.h"

TreeNode* Othello::findBestNeighbor(TreeNode* node, int player) {
  int maxScore = 0;
  TreeNode* bestMove = NULL;
  for (auto n = node->children.begin(); n != node->children.end(); ++n) { // for each TreeNode in node->children
    int score = Othello::scoreOfBoard(*n, player, 0, INT_MAX);
    if (score > maxScore) {
      maxScore = score;
      bestMove = *n;
    }
  }
  return bestMove;
}

bool Othello::canFlip(int** state, int player, int X, int Y, int dirX, int dirY) {
  bool capture = false;
  while ((X + dirX < 8) &&
	 (X + dirX >= 0) &&
	 (Y + dirY < 8) &&
	 (Y + dirY >= 0) &&
	 (state[X + dirX][Y + dirY] == 3 - player)) {
    X += dirX;
    Y += dirY;
    capture = true;
  }
  if (!capture) return false;
  if ((X + dirX < 8) &&
      (X + dirX >= 0) &&
      (Y + dirY < 8) &&
      (Y + dirY >= 0) &&
      (state[X + dirX][Y + dirY] == player)) {
    return true;
  }
  else return false;
}

bool Othello::isLegal(int** state, int player, int X, int Y) {
  if (state[X][Y] != 0) return false;
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      if (((i != 0) || (j != 0)) && Othello::canFlip(state, player, X, Y, i, j))
	return true;
    }
  }
  return false;
}

int Othello::scoreOfBoard(TreeNode* node, int player, int alpha, int beta) {
  int** state = node->board;
  int otherPlayer = 3 - player;
  int playerScore = 0;
  int otherPlayerScore = 0;
  if (node->children.empty())
    {
      for (int i = 0; i < 8; ++i) {
	for (int j = 0; j < 8; ++j) {
	  if (state[i][j] == 0)
	    continue;
	  if (isCorner(i, j)) {
	    if (state[i][j] == player)
	      playerScore += 5;
	    else if (state[i][j] == otherPlayer)
	      otherPlayerScore += 5;
	  }
	  else if (isNextToCorner(i, j)) {
	    if (state[i][j] == player)
	      playerScore += 2;
	    else if (state[i][j] == otherPlayer)
	      otherPlayerScore += 2;
	  }
	  else if (isSide(i, j)) {
	    if (state[i][j] == player)
	      playerScore += 3;
	    else if (state[i][j] == otherPlayer)
	      otherPlayerScore += 3;
	  }
	  else {
	    if (state[i][j] == player)
	      playerScore += 1;
	    else if (state[i][j] == otherPlayer)
	      otherPlayerScore += 1;
	  }
	}
      }
      return playerScore - otherPlayerScore;
    }
  else {
    for (auto it = node->children.begin(); it != node->children.end(); ++it) {
      int temp = Othello::scoreOfBoard(*it, otherPlayer, alpha, beta);
      if (temp > alpha)
	alpha = temp;
      if (alpha >= beta)
	return beta;
    }
    return alpha;
  }
}

void Othello::doFlip(int** state, int player, int X, int Y, int dirX, int dirY) {
  while ((X + dirX < BOARD_SIZE) && (X + dirX >= 0) &&
	 (Y + dirY < BOARD_SIZE) && (Y + dirY >= 0) &&
	 (state[X + dirX][Y + dirY] == 3-player)) {
    X += dirX;
    Y += dirY;
    state[X][Y] = player;
  }
}

int** Othello::makeMove(int** board, int player, int X, int Y) {
  int** modifiedBoard = new int*[BOARD_SIZE];
  for (int i = 0; i < BOARD_SIZE; ++i) {
    modifiedBoard[i] = new int[BOARD_SIZE];
  }
  
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      modifiedBoard[i][j] = board[i][j];
    }
  }
  if (modifiedBoard[X][Y] != 0) {
    cout << "Illegal move\n";
  }
  modifiedBoard[X][Y] = player;
  for (int i = 0; i <= 2; i++)
    for (int j = 0; j <= 2; j++)
      if (((i != 0) || (j != 0)) && Othello::canFlip(modifiedBoard, player, X, Y, i, j))
	Othello::doFlip(modifiedBoard, player, X, Y, i, j);

  return modifiedBoard;
}

int Othello::howManyPiecesFlipped(int X, int Y, int** state, int player) {
  int flips = 0;
  if (X < 0) return 0; /* pass move */
  if (state[X][Y] != 0) {
    cout << "Illegal move\n";
  }
  state[X][Y] = player;
  for (int i = 0; i <= 2; ++i)
    for (int j = 0; j <= 2; ++j)
      if ((i!=0 || j!=0) && Othello::canFlip(state, player, X, Y, i, j))
	flips += Othello::countFlips(state, player, X, Y, i, j);
  return flips;
}

int Othello::countFlips(int** state, int player, int X, int Y, int dirX, int dirY) {
  int count = 0;
  while ((X + dirX < 8) && (X + dirX >= 0) && (Y + dirY < 8) && (Y + dirY >= 0) && (state[X + dirX][Y + dirY] == 3 - player)) {
    X += dirX;
    Y += dirY;
    ++count;
  }
  return count;
}

bool Othello::isCorner(int x, int y) {
  return ((x == 0) || (x == BOARD_SIZE - 1)) && ((y == 0) || (y == BOARD_SIZE - 1));
}

bool Othello::isNextToCorner(int x, int y) {
  return ((x <= 1) || (x >= BOARD_SIZE - 2)) && ((y <= 1) || (y >= BOARD_SIZE - 2)) && !isCorner(x, y);
}

bool Othello::isSide(int x, int y) {
  return (((x == 0) || (x == BOARD_SIZE - 1)) && (y >= 2) && (y <= BOARD_SIZE - 3)) ||
    (((y == 0) || (y == BOARD_SIZE - 1)) && (x >= 2) && (x <= BOARD_SIZE - 3));
}
