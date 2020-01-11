#include <iostream>
#include <stdlib.h>
#include <time.h>
#include "rules.h"
#include "algorithm.h"

using namespace std;

/******************************************************
 * In your algorithm, you can just use the the funcitons
 * listed by TA to get the board information.(functions
 * 1. ~ 4. are listed in next block)
 *
 * The STL library functions is not allowed to use.
******************************************************/

/*************************************************************************
 * 1. int board.get_orbs_num(int row_index, int col_index)
 * 2. int board.get_capacity(int row_index, int col_index)
 * 3. char board.get_cell_color(int row_index, int col_index)
 * 4. void board.print_current_board(int row_index, int col_index, int round)
 *
 * 1. The function that return the number of orbs in cell(row, col)
 * 2. The function that return the orb capacity of the cell(row, col)
 * 3. The function that return the color fo the cell(row, col)
 * 4. The function that print out the current board statement
*************************************************************************/


#define WEST -1
#define EAST 1
#define NORTH 1
#define SOUTH -1
int algorithm_A_round = 0;
int first_stepX, first_stepY;
bool corner_tragety=true;
int num_to_be_full(int orbX, int orbY, Board board) {
	return board.get_capacity(orbX,orbY) - board.get_orbs_num(orbX, orbY);
}

double cell_value_estimate(int capacity, int orbs_num) {
	if (capacity == 2) {
		return 2;
	}
	else if (capacity == 3) {
		if (orbs_num == 1)
			return 0.75;
		else if (orbs_num == 2)
			return 2.5;
	}
	else if (capacity == 4) {
		if (orbs_num == 1)
			return 0.5;
		else if (orbs_num == 2)
			return 1.2;
		else if (orbs_num == 3)
			return 3;
	}
}

double orbcompare_est(int orbX, int orbY, int com_orbX, int com_orbY,Board board) {
	if (num_to_be_full(orbX, orbY, board) <= num_to_be_full(com_orbX, com_orbY, board))
		return -cell_value_estimate(board.get_capacity(orbX, orbY), board.get_orbs_num(orbX, orbY));
	else
		return cell_value_estimate(board.get_capacity(com_orbX, com_orbY), board.get_orbs_num(com_orbX, com_orbY));
}



class MyTreeNode {
private:
	Board boardState;
	MyTreeNode* nextNode;
	MyTreeNode* nextRoundNode;
	int index[2];    // Record which orb result in this MyTreeNode
public:
	MyTreeNode();
	MyTreeNode(Board board);
	MyTreeNode(Board board, int x, int y);
	void setNextNode(MyTreeNode* MyTreeNode);
	void setNextRoundNode(MyTreeNode* MyTreeNode);
	MyTreeNode* getNextNode();
	MyTreeNode* getNextRoundNode();
	MyTreeNode* getTail();
	Board getBoardState();
	int getX();
	int getY();
};

class MygameTree {
private:
	MyTreeNode* root;
	Player* player;
	Player* opponent;
public:
	MygameTree(Board board, Player* player);
	int* chooseOrb();   // Use the four-level look-ahead game tree to determine the next step of orb
	int insertNextRound(MyTreeNode* Node, Player currentTurn);
	int levelLookAhead();  // Add a new level of look-ahead in MygameTree
	int numAtLevel(MyTreeNode* Node);
	float BoardEvaluator(MyTreeNode* Node);
};

/*-------------------------Implement class MyTreeNode functions---------------------------*/
MyTreeNode::MyTreeNode(Board board) {
	this->boardState = board;
	nextNode = NULL;
	nextRoundNode = NULL;
	index[0] = 0; index[1] = 0;
}

MyTreeNode::MyTreeNode(Board board, int x, int y) {
	this->boardState = board;
	nextNode = NULL;
	nextRoundNode = NULL;
	this->index[0] = x;
	this->index[1] = y;
}

void MyTreeNode::setNextNode(MyTreeNode* Node) {
	this->nextNode = Node;
}

void MyTreeNode::setNextRoundNode(MyTreeNode* Node) {
	this->nextRoundNode = Node;
}

MyTreeNode* MyTreeNode::getNextNode() {
	return this->nextNode;
}

MyTreeNode* MyTreeNode::getNextRoundNode() {
	return this->nextRoundNode;
}

MyTreeNode* MyTreeNode::getTail() {
	MyTreeNode* temp = NULL;
	MyTreeNode* tempNext = this;
	while (tempNext != NULL) {
		temp = tempNext;
		tempNext = tempNext->getNextNode();
	}
	return temp;
}

Board MyTreeNode::getBoardState() {
	return this->boardState;
}

int MyTreeNode::getX() {
	return this->index[0];
}

int MyTreeNode::getY() {
	return this->index[1];
}

/*------------------------Implement class MygameTree functions----------------------------*/
MygameTree::MygameTree(Board board, Player* player) {
	this->root = new MyTreeNode(board);
	this->player = player;
	char colorPlayer = player->get_color();
	char colorOpponent;
	if (colorPlayer == 'r')
		colorOpponent = 'b';
	else
		colorOpponent = 'r';
	this->opponent = new Player(colorOpponent);
}

int* MygameTree::chooseOrb() {
	this->insertNextRound(this->root, *this->player);
	MyTreeNode* firstLevelNode = this->root->getNextRoundNode()->getNextNode();
	int index[2];
	index[0] = firstLevelNode->getX();
	index[1] = firstLevelNode->getY();
	float bestScore = -100;
	float tempScore = 0;
	long numRoundNode = 0;
	
	for (firstLevelNode; firstLevelNode != NULL; firstLevelNode = firstLevelNode->getNextNode()) {
		tempScore = this->BoardEvaluator(firstLevelNode);
		numRoundNode = insertNextRound(firstLevelNode, *this->opponent);
		MyTreeNode* secondLevelNode = firstLevelNode->getNextRoundNode()->getNextNode();
		for (secondLevelNode; secondLevelNode != NULL; secondLevelNode = secondLevelNode->getNextNode()) {
			tempScore += 0.8 * this->BoardEvaluator(secondLevelNode) / numRoundNode;
			numRoundNode = numRoundNode * insertNextRound(secondLevelNode, *this->player);
			MyTreeNode* thirdLevelNode = secondLevelNode->getNextRoundNode()->getNextNode();
			for (thirdLevelNode; thirdLevelNode != NULL; thirdLevelNode = thirdLevelNode->getNextNode()) {
				tempScore += 0.6 * this->BoardEvaluator(thirdLevelNode) / numRoundNode;
			}
		}
		if (tempScore > bestScore) {
			bestScore = tempScore;
			index[0] = firstLevelNode->getX();
			index[1] = firstLevelNode->getY();
		}
		/*MyTreeNode* tmp = firstLevelNode;
		MyTreeNode* tmp2 = tmp->getNextRoundNode();
		while (tmp2 != NULL) {
			MyTreeNode* tmp3 = tmp2->getNextRoundNode();
			while (tmp3 != NULL) {
				MyTreeNode* temp = tmp3;
				tmp3 = tmp3->getNextNode();
				delete temp;
			}
			MyTreeNode* temp = tmp2;
			tmp2 = tmp2->getNextNode();
			delete temp;
		}*/
	}
	return index;
}

int MygameTree::insertNextRound(MyTreeNode* Node, Player currentTurn) {
	// Insert all the cases of next round in MygameTree
	// Input : currentTurn indicates which player is going to put the orb in this turn
	int numNewState = 0;
	char color = currentTurn.get_color();
	Board cur_boardState = Node->getBoardState();
	
	// Find first next round node using random

	MyTreeNode* virtualNode= new MyTreeNode(cur_boardState, 0, 0);
	Node->setNextRoundNode(virtualNode);

	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			if (cur_boardState.get_cell_color(i, j) == color || cur_boardState.get_cell_color(i, j) == 'w') {
				numNewState++;
				Board tmp = cur_boardState;
				tmp.place_orb(i, j, &currentTurn);
				MyTreeNode* newNode = new MyTreeNode(tmp, i, j);
				MyTreeNode* tail = Node->getNextRoundNode()->getTail();
				tail->setNextNode(newNode);
			}
		}
	}
	return numNewState;
}

float MygameTree::BoardEvaluator(MyTreeNode* Node) {
	// This is the simple board evaluator
	Board board = Node->getBoardState();
	char colorPlayer = player->get_color();
	char colorOpponent = opponent->get_color();
	float orbPlayer = 0, orbOpponent = 0;
	float orbScore = 0;
	int orbX = Node->getX();
	int orbY = Node->getY();

	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			if (board.get_cell_color(i, j) == colorPlayer) {
				orbScore += cell_value_estimate(board.get_capacity(i,j), board.get_orbs_num(i,j));
			}
			else if (board.get_cell_color(i, j) == colorOpponent) {
				orbScore -= cell_value_estimate(board.get_capacity(i, j), board.get_orbs_num(i,j));
			}
		}
	}

	if (board.get_cell_color(orbX, orbY) == colorPlayer) {
		if (index_range_illegal(orbX + EAST, orbY) == true) {
			if (board.get_cell_color(orbX + EAST, orbY) == colorOpponent) {
				orbScore += orbcompare_est(orbX,orbY, orbX + EAST, orbY,board);
			}
		}
		if (index_range_illegal(orbX - WEST, orbY) == true) {
			if (board.get_cell_color(orbX - WEST, orbY) == colorOpponent) {
				if (board.get_cell_color(orbX + EAST, orbY) == colorOpponent) {
					orbScore += orbcompare_est(orbX, orbY, orbX + EAST, orbY, board);
				}
			}
		}
		if (index_range_illegal(orbX, orbY + NORTH) == true) {
			if (board.get_cell_color(orbX, orbY + NORTH) == colorOpponent) {
				if (board.get_cell_color(orbX + EAST, orbY) == colorOpponent) {
					orbScore += orbcompare_est(orbX, orbY, orbX + EAST, orbY, board);
				}
			}
		}
		if (index_range_illegal(orbX, orbY - SOUTH) == true) {
			if (board.get_cell_color(orbX, orbY - SOUTH) == colorOpponent) {
				if (board.get_cell_color(orbX + EAST, orbY) == colorOpponent) {
					orbScore += orbcompare_est(orbX, orbY, orbX + EAST, orbY, board);
				}
			}
		}
	}

	return orbScore;
}


void algorithm_A(Board board, Player player, int index[]) {
	MygameTree gt(board, &player);
	algorithm_A_round++;
	int row, col;
	if (algorithm_A_round == 1) {
		if (board.get_cell_color(0, 0) == 'w') {
			row = 0;
			col = 0;
		}
		else if (board.get_cell_color(4, 0) == 'w') {
			row = 4;
			col = 0;
		}
		else if (board.get_cell_color(0, 5) == 'w') {
			row = 0;
			col = 5;
		}
		else if (board.get_cell_color(4, 5) == 'w') {
			row = 4;
			col = 5;
		}
		index[0] = row;
		index[1] = col;
		return;
	}
	else if (algorithm_A_round ==2) {
		if (board.get_cell_color(0, 0) == 'w') {
			row = 0;
			col = 0;
		}
		else if (board.get_cell_color(4, 0) == 'w') {
			row = 4;
			col = 0;
		}
		else if (board.get_cell_color(0, 5) == 'w') {
			row = 0;
			col = 5;
		}
		else if (board.get_cell_color(4, 5) == 'w') {
			row = 4;
			col = 5;
		}
		index[0] = row;
		index[1] = col;
		return;
	}
	int* choose = gt.chooseOrb();
	index[0] = choose[0];
	index[1] = choose[1];

}
