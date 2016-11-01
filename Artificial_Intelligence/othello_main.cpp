#include "othello.h"

int main(int argc, char **argv) {
  boolean ourTurn;
  int** board = new int[BOARD_SIZE][BOARD_SIZE]; //NOTE TO SELF: NEEDS TO BE DELETED/FREED
  Scanner in = new Scanner(System.in);
  String input = in.nextLine();
  String[] parameters = input.split(" ");
  if (parameters[0].equals("game"))
    {
      /* initialize board */
      for (int i = 0; i < 8; i++)
	Arrays.fill(board[i], 0);
      board[3][3] = 2; // black
      board[4][4] = 2;
      board[4][3] = 1; // white
      board[3][4] = 1;

      /* parse input line */
      int depthLimit = Integer.parseInt(parameters[2]);
      int timeLimit1 = Integer.parseInt(parameters[3]);
      int timeLimit2 = Integer.parseInt(parameters[4]);
      boolean isDepthLimited, isTime1Limited, isTime2Limited;
      isDepthLimited = (depthLimit != 0 && timeLimit1 == 0 && timeLimit2 == 0);
      isTime1Limited = (timeLimit1 != 0 && timeLimit2 == 0);
      isTime2Limited = (timeLimit2 != 0);
			
      int player;
      if (parameters[1].equals("B")) {// we move first
	  ourTurn = true;
	  player = 2;
      }
      else {// they move first - this also occurs if color is invalid
	  ourTurn = false;
	  player = 1;
      }
      System.out.println("A - " + isDepthLimited + isTime1Limited + isTime2Limited);//DEBUG
      int noAvailableMoves = 0;
      ArrayList<TreeNode> queue = new ArrayList<>();
      long gameStartTime = System.currentTimeMillis();
      while (true)
	{
	  if (noAvailableMoves >= 2 || (isTime2Limited && System.currentTimeMillis() - gameStartTime >= timeLimit2)) // legal game end
	    {
	      int whiteCount = 0;
	      int blackCount = 0;
	      for (int i = 0; i < 8; i++)
		{
		  for (int j = 0; j < 8; j++)
		    {
		      if (board[i][j] == 1) whiteCount++;
		      if (board[i][j] == 2) blackCount++;
		    }
		}
	      char winner;
	      if (whiteCount > blackCount) winner = 'W';
	      else if (blackCount > whiteCount) winner = 'B';
	      else winner = 'T';
	      System.out.printf("winner %c %d legal\n", winner, whiteCount>blackCount ? whiteCount-blackCount : blackCount-whiteCount);
	      break;
	    }
	  if ( ourTurn ) {
	    long startTime = System.currentTimeMillis();
	    TreeNode treeRoot = new TreeNode(board, null, 0, 1);
	    queue.add(treeRoot);

	    // create tree, within limits
	    int counter = 0;
	    while (true /* within depth limit and time limit */)
	      {
		if (queue.isEmpty())
		  break;
		TreeNode temp = queue.get(0);
		if (isDepthLimited && temp.depth == depthLimit) {
		  break;
		}
		if (isTime1Limited && System.currentTimeMillis() - startTime >= ((long) (0.9 * (double)timeLimit1)))
		  break;
		queue.remove(0);
		for (int i = 0; i < 8; i++)
		  {
		    for (int j = 0; j < 8; j++)
		      {
			if (isLegal(temp.board, 1, i, j))
			  {
			    TreeNode temp2 = new TreeNode(makeMove(temp.board, 1, i, j), temp, temp.depth + 1, 3 - temp.player);
			    temp2.x = i;
			    temp2.y = j;
			    queue.add(temp2);
			    temp.children.add(temp2);
			    counter++;
			  }
		      }
		  }
	      }
	    if (counter > 0)
	      noAvailableMoves = 0;
	    else
	      {
		noAvailableMoves++;
		System.out.println("pass");
		continue;
	      }
	    TreeNode bestNeighbor = findBestNeighbor(treeRoot, 1);
	    // print move
	    System.out.println(bestNeighbor.x + " " + bestNeighbor.y);
	    board = makeMove(board, bestNeighbor.x, bestNeighbor.y, 1);
	    ourTurn = false;
	    if (System.currentTimeMillis() - startTime > timeLimit1 && isTime1Limited)
	      {
		int whiteCount = 0;
		int blackCount = 0;
		for (int i = 0; i < 8; i++)
		  {
		    for (int j = 0; j < 8; j++)
		      {
			if (board[i][j] == 1) whiteCount++;
			if (board[i][j] == 2) blackCount++;
		      }
		  }
		System.out.printf("winner %c %d timeout\n", player==1 ? 'B' : 'W', player==2 ? whiteCount-blackCount : blackCount-whiteCount);
		break;
	      }
	  }
	  else {
	    long otherPlayerStartTime = System.currentTimeMillis();
	    input = in.nextLine();
	    if (System.currentTimeMillis() - otherPlayerStartTime > timeLimit1 )
	      {
		int whiteCount = 0;
		int blackCount = 0;
		for (int i = 0; i < 8; i++)
		  {
		    for (int j = 0; j < 8; j++)
		      {
			if (board[i][j] == 1) whiteCount++;
			if (board[i][j] == 2) blackCount++;
		      }
		  }
		System.out.printf("winner %c %d timeout\n", player==2 ? 'B' : 'W', player==1 ? whiteCount-blackCount : blackCount-whiteCount);
		break;
	      }
	    if (input.equals("pass"))
	      {
		noAvailableMoves++;
		continue;
	      }
	    noAvailableMoves = 0;
	    parameters = input.split(" ");
	    int X = Integer.parseInt(parameters[0]);
	    int Y = Integer.parseInt(parameters[1]);
	    if (!isLegal(board, 3-player, X, Y))
	      {
		int whiteCount = 0;
		int blackCount = 0;
		for (int i = 0; i < 8; i++)
		  {
		    for (int j = 0; j < 8; j++)
		      {
			if (board[i][j] == 1) whiteCount++;
			if (board[i][j] == 2) blackCount++;
		      }
		  }
		System.out.printf("winner %c %d illegal\n", player==1 ? 'W' : 'B', player==1 ? whiteCount-blackCount : blackCount-whiteCount);
		break;
	      }
	    board = makeMove(board, X, Y, 3-player);
	    ourTurn = true;
	  }
	}
      in.close();
    }
  else
    {
      System.out.println("Invalid command!");
      in.close();
      return;
    }
}
