This directory contains all the source files needed for a project that can play a game of Othello.

To compile this project, type “make” or “make othello” at the command line.

IMPORTANT NOTE: Compiling this project requires C++11.  Source code contains multiple instances of the “auto” feature introduced in C++11.  Ensure you have C++11 or later, or else the project will not compile.

Once the project has been compiled, type “./othello” at the command line to play a game.  You will be asked to assign a color of black (B) or white (W) to the program, and then you will be asked to specify a maximum depth for the decision tree the program will use to make decisions.  It is strongly recommended that you enter a number of no less than 1 and no more than 6.  Entering anything above 6 will slow down the game considerably, and entering anything less than 1 signifies that there should be no depth limit, which will make the game just as slow if not even slower.

Black always moves first.  If the program is black, it will print its first move to cout and then wait for you to input your move.  If the program is white, it will wait for you to input your first move, and then it will print its first move to cout.  This cycle continues until either the user makes an illegal move or neither player (user or program) has any remaining legal moves.

The game board is considered to be indexed starting from 0 and to be 8 spaces by 8 spaces square.  To enter a move to cin, type the x-coordinate of your move, followed by whitespace, followed by the y-coordinate of your move, and then press Enter.

Undefined behavior may occur if any of the following happens:
 - When prompted for B or W, the user enters a string of length >1 that begins with B or W.
 - When prompted for the maximum depth of the pruning tree, the user enters a string that cannot be recognized as an integer.
 - When prompted for a new move, the user does not adhere to the formula of [int][whitespace][int].

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The contents of this directory can be described as follows:
 - GameBoard.cpp contains the class information for the GameBoard class, which represents an Othello board’s state and contains various functions for reading/writing the state.
 - Makefile contains the compile instructions for this project.
 - othello_main.cpp is the main C++ source file for this project.  It contains the main function and a helper function.
 - Othello.cpp describes the Othello class, which consists of a variety of static functions that are needed to play Othello.
 - othello.h is the header file for this project.
 - README.md is this file.
 - TreeNode.cpp contains the class information for the TreeNode class, which represents a node in a decision tree employed in making decisions for playing Othello and contains related functions.