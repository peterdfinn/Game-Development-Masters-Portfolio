#include "othello.h"

int main(int argc, char **argv) {

  // Determine whether the input command is valid.
  string input_string;
  cin >> input_string;
  if (input_string.compare(0, 5, "game ")) {
    cout << "Invalid command\n";
    return 0;
  }

  
  //Scanner in = new Scanner(System.in); // do something about this
  //String[] parameters = input.split(" ");


  
  // Create and initialize the main game board.
  int** board = new int*[BOARD_SIZE]; //NOTE TO SELF: NEEDS TO BE DELETED/FREED
  for (int i = 0; i < BOARD_SIZE; ++i) {
    board[i] = new int[BOARD_SIZE];
  }
  
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      board[i][j] = 0;
    }
  }
  board[(BOARD_SIZE / 2) - 1][(BOARD_SIZE / 2) - 1] = 2; // black
  board[BOARD_SIZE / 2][BOARD_SIZE / 2] = 2;
  board[BOARD_SIZE / 2][(BOARD_SIZE / 2) - 1] = 1; // white
  board[(BOARD_SIZE / 2) - 1][BOARD_SIZE / 2] = 1;

  // Parse input line to determine game variables.
  int idxOfSpace1, idxOfSpace2, idxOf0;
  int idx;
  for (idx = 8; input_string[idx] != ' '; ++idx);
  idxOfSpace1 = idx;
  //for (idx = idxOfSpace1 + 1; input_string[idx] != ' '; ++idx); TIME
  //idxOfSpace2 = idx; TIME
  //for (idx = idxOfSpace2 + 1; input_string[idx] != (char) 0; ++idx); TIME
  //idxOf0 = idx; TIME
  
  string depthLimit_string = input_string.substr(7, idxOfSpace1 - 7);
  int depthLimit = stoi(depthLimit_string);

  //string timeLimit1_string = input_string.substr(idxOfSpace1 + 1, idxOfSpace2 - idxOfSpace1 - 1); TIME
  //int timeLimit1 = atoi(timeLimit1_string); TIME

  //string timeLimit2_string = input_string.substr(idxOfSpace2 + 1, idxOf0 - idxOfSpace2 - 1); TIME
  //int timeLimit2 = atoi(timeLimit2_string); TIME

  bool isDepthLimited;//, isTime1Limited, isTime2Limited; TIME
  //isDepthLimited = ((depthLimit != 0) && (timeLimit1 == 0) && (timeLimit2 == 0)); TIME
  isDepthLimited = (depthLimit != 0);
  //isTime1Limited = ((timeLimit1 != 0) && (timeLimit2 == 0)); TIME
  //isTime2Limited = (timeLimit2 != 0); TIME
		
  int player;
  bool ourTurn;
  if (input_string[5] == 'B') {// we move first
    ourTurn = true;
    player = 2;
  }
  else if (input_string[5] == 'W') {// they move first
    ourTurn = false;
    player = 1;
  }
  else {
    cout << "Invalid command\n";
    for (int i = 0; i < BOARD_SIZE; ++i) {
      delete[] board[i];
    }
    delete[] board;
    return 0;
  }


  // Now that the initial input line has been parsed, we must...
  int noAvailableMoves = 0;
  queue<TreeNode*> queue;
  //long gameStartTime = System.currentTimeMillis(); TIME
  while (true) {

    // Is the game over?  If so, break out of this while loop.  If not, we either take our turn or wait for the opponent's turn.
    //if (noAvailableMoves >= 2 || (isTime2Limited && System.currentTimeMillis() - gameStartTime >= timeLimit2)) {// legal game end TIME
    if (noAvailableMoves >= 2) {
      int whiteCount = 0, blackCount = 0;
      for (int i = 0; i < 8; ++i) {
	for (int j = 0; j < 8; ++j) {
	  if (board[i][j] == 1) ++whiteCount;
	  if (board[i][j] == 2) ++blackCount;
	}
      }
      char winner;
      if (whiteCount > blackCount) winner = 'W';
      else if (blackCount > whiteCount) winner = 'B';
      else winner = 'T';
      cout << "winner " << winner << " " << (whiteCount > blackCount ? whiteCount - blackCount : blackCount - whiteCount) << " legal\n";
      return 0;
    }

    // If it's our turn, we must...
    if (ourTurn) {
      //long startTime = System.currentTimeMillis(); TIME
      TreeNode* treeRoot = new TreeNode(board, NULL, 0, 1);
      queue.push(treeRoot);

      // Create tree, within constraints.
      int counter = 0;
      while (true /* within depth limit and time limit */) {
	if (queue.empty()) // if no more moves to be made
	  break;
	TreeNode* temp = queue.front(); // Consider a single move that can be made...
	if (isDepthLimited && (temp->depth == depthLimit)) {
	  break;
	}
	/* TIME
	if (isTime1Limited && System.currentTimeMillis() - startTime >= ((long) (0.9 * (double) timeLimit1)))
	  break;*/
	queue.pop();
	for (int i = 0; i < BOARD_SIZE; ++i) {
	  for (int j = 0; j < BOARD_SIZE; ++j) {
	    if (Othello::isLegal(temp->board, 1, i, j)) {
	      TreeNode* temp2 = new TreeNode(Othello::makeMove(temp->board, 1, i, j), temp, temp->depth + 1, 3 - temp->player);
	      // For each new move that can be made from the 'temp' TreeNode, add it to the queue of moves to be added to the tree, and add it to temp's list of children.
	      // This kills two birds with one stone.
	      temp2->x = i;
	      temp2->y = j;
	      queue.push(temp2);
	      temp->children.insert(temp2);
	      ++counter;
	    }
	  }
	}
      }
      if (counter > 0)
	noAvailableMoves = 0;
      else {
	++noAvailableMoves;
	cout << "pass\n";
	continue;
      }

      
      TreeNode* bestNeighbor = Othello::findBestNeighbor(treeRoot, 1);
      // print move
      cout << bestNeighbor->x << " " << bestNeighbor->y << "\n";
      board = Othello::makeMove(board, bestNeighbor->x, bestNeighbor->y, 1);
      ourTurn = false;
      /* TIME
      if (System.currentTimeMillis() - startTime > timeLimit1 && isTime1Limited) {
	int whiteCount = 0;
	int blackCount = 0;
	for (int i = 0; i < 8; ++i) {
	  for (int j = 0; j < 8; ++j)
	    {
	      if (board[i][j] == 1) ++whiteCount;
	      if (board[i][j] == 2) ++blackCount;
	    }
	}
	System.out.printf("winner %c %d timeout\n", player == 1 ? 'B' : 'W', player == 2 ? whiteCount - blackCount : blackCount - whiteCount);
	break;
      }*/
    }
    else {
      // long otherPlayerStartTime = System.currentTimeMillis(); TIME
      cin >> input_string;
      /* TIME
      if (System.currentTimeMillis() - otherPlayerStartTime > timeLimit1) { // Did the other player take too long?
	int whiteCount = 0;
	int blackCount = 0;
	for (int i = 0; i < 8; ++i) {
	  for (int j = 0; j < 8; ++j) {
	    if (board[i][j] == 1) ++whiteCount;
	    if (board[i][j] == 2) ++blackCount;
	  }
	}
	System.out.printf("winner %c %d timeout\n", player == 2 ? 'B' : 'W', player == 1 ? whiteCount - blackCount : blackCount - whiteCount);
	break;
      }*/
      if (!input_string.compare(0, 4, "pass")) { // Did the other player pass?
	++noAvailableMoves;
	continue;
      }
      noAvailableMoves = 0;

      // Now we must parse their move.
      
      int idxOfSpace;
      for (idxOfSpace = 0; input_string[idxOfSpace] != ' '; ++idxOfSpace);
      int X = stoi(input_string.substr(0, idxOfSpace));
      int Y = stoi(input_string.substr(idxOfSpace + 1, input_string.size() - idxOfSpace - 1));
      if (!Othello::isLegal(board, 3 - player, X, Y)) {
	int whiteCount = 0;
	int blackCount = 0;
	for (int i = 0; i < BOARD_SIZE; ++i) {
	  for (int j = 0; j < BOARD_SIZE; ++j) {
	    if (board[i][j] == 1) ++whiteCount;
	    if (board[i][j] == 2) ++blackCount;
	  }
	}
	cout << "winner " << (player == 1 ? 'W' : 'B') << " " << (player == 1 ? whiteCount - blackCount : blackCount - whiteCount) << " illegal\n";
	break;
      }
      board = Othello::makeMove(board, X, Y, 3 - player);
      ourTurn = true;
    }
  }
}

