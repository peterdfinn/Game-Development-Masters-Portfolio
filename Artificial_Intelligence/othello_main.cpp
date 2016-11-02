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
  int** board = new int[BOARD_SIZE][BOARD_SIZE]; //NOTE TO SELF: NEEDS TO BE DELETED/FREED
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
  int idx = 8;
  for (; input_string[idx] != ' '; ++idx);
  idxOfSpace1 = idx;
  for (idx = idxOfSpace1 + 1; input_string[idx] != ' '; ++idx);
  idxOfSpace2 = idx;
  for (idx = idxOfSpace2 + 1; input_string[idx] != (char) 0; ++idx);
  idxOf0 = idx;
  
  string depthLimit_string = input_string.substr(7, idxOfSpace1 - 7);
  int depthLimit = atoi(depthLimit_string);

  string timeLimit1_string = input_string.substr(idxOfSpace1 + 1, idxOfSpace2 - idxOfSpace1 - 1);
  int timeLimit1 = atoi(timeLimit1_string);

  string timeLimit2_string = input_string.substr(idxOfSpace2 + 1, idxOf0 - idxOfSpace2 - 1);
  int timeLimit2 = atoi(timeLimit2_string);

  bool isDepthLimited, isTime1Limited, isTime2Limited;
  isDepthLimited = ((depthLimit != 0) && (timeLimit1 == 0) && (timeLimit2 == 0));
  isTime1Limited = ((timeLimit1 != 0) && (timeLimit2 == 0));
  isTime2Limited = (timeLimit2 != 0);
		
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
    delete[][] board;
    return 0;
  }


  // Now that the initial input line has been parsed, we must...
  int noAvailableMoves = 0;
  ArrayList<TreeNode> queue = new ArrayList<>(); // oh god
  long gameStartTime = System.currentTimeMillis();
  while (true) {

    // Is the game over?  If so, break out of this while loop.  If not, we either take our turn or wait for the opponent's turn.
    if (noAvailableMoves >= 2 || (isTime2Limited && System.currentTimeMillis() - gameStartTime >= timeLimit2)) {// legal game end
      int whiteCount = 0;
      int blackCount = 0;
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
      System.out.printf("winner %c %d legal\n", winner, whiteCount > blackCount ? whiteCount - blackCount : blackCount - whiteCount);
      return 0;
    }

    // If it's our turn, we must...
    if (ourTurn) {
      long startTime = System.currentTimeMillis();
      TreeNode treeRoot = new TreeNode(board, null, 0, 1);
      queue.add(treeRoot);

      // Create tree, within constraints.
      int counter = 0;
      while (true /* within depth limit and time limit */) {
	if (queue.isEmpty()) // if no more moves to be made
	  break;
	TreeNode temp = queue.get(0); // Consider a single move that can be made...
	if (isDepthLimited && temp.depth == depthLimit) {
	  break;
	}
	if (isTime1Limited && System.currentTimeMillis() - startTime >= ((long) (0.9 * (double) timeLimit1)))
	  break;
	queue.remove(0);
	for (int i = 0; i < BOARD_SIZE; ++i) {
	  for (int j = 0; j < BOARD_SIZE; ++j) {
	    if (Othello::isLegal(temp.board, 1, i, j)) {
	      TreeNode temp2 = new TreeNode(Othello::makeMove(temp.board, 1, i, j), temp, temp.depth + 1, 3 - temp.player);
	      // For each new move that can be made from the 'temp' TreeNode, add it to the queue of moves to be added to the tree, and add it to temp's list of children.
	      // This kills two birds with one stone.
	      temp2.x = i;
	      temp2.y = j;
	      queue.add(temp2);
	      temp.children.add(temp2);
	      ++counter;
	    }
	  }
	}
      }
      if (counter > 0)
	noAvailableMoves = 0;
      else {
	++noAvailableMoves;
	System.out.println("pass");
	continue;
      }

      
      TreeNode bestNeighbor = Othello::findBestNeighbor(treeRoot, 1);
      // print move
      System.out.println(bestNeighbor.x + " " + bestNeighbor.y);
      board = makeMove(board, bestNeighbor.x, bestNeighbor.y, 1);
      ourTurn = false;
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
      }
    }
    else {
      long otherPlayerStartTime = System.currentTimeMillis();
      cin >> input_string;
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
      }
      if (!input_string.compare(0, 4, "pass")) { // Did the other player pass?
	++noAvailableMoves;
	continue;
      }
      noAvailableMoves = 0;

      // Now we must parse their move.
      
      int idxOfSpace = 0;
      for (; input_string[idxOfSpace] != ' '; ++ idxOfSpace);
      int X = atoi(input_string.substr(0, idxOfSpace));
      int Y = atoi(input_string.substr(idxOfSpace + 1, input_string.size() - idxOfSpace - 1));
      if (!Othello::isLegal(board, 3 - player, X, Y)) {
	int whiteCount = 0;
	int blackCount = 0;
	for (int i = 0; i < BOARD_SIZE; ++i) {
	  for (int j = 0; j < BOARD_SIZE; ++j) {
	    if (board[i][j] == 1) ++whiteCount;
	    if (board[i][j] == 2) ++blackCount;
	  }
	}
	System.out.printf("winner %c %d illegal\n", player == 1 ? 'W' : 'B', player == 1 ? whiteCount - blackCount : blackCount - whiteCount);
	break;
      }
      board = makeMove(board, X, Y, 3 - player);
      ourTurn = true;
    }
  }
}

