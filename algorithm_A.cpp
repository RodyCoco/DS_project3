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
#define MaxScore 100000
#define MinScore -100000

int num_to_be_full(int orbX, int orbY, Board& board) {
	return board.get_capacity(orbX, orbY) - board.get_orbs_num(orbX, orbY);
}

float cell_value_estimate(int capacity, int orbs_num) {
	float cell_value = MaxScore;

	if (orbs_num == 0)
		return 0;

	if (capacity == 2) {
		cell_value = 6;
	}
	else if (capacity == 3) {
		if (orbs_num == 1)
			cell_value = 2;
		else if (orbs_num == 2)
			cell_value = 8;
	}
	else if (capacity == 4) {
		if (orbs_num == 1)
			cell_value = 1;
		else if (orbs_num == 2)
			cell_value = 3;
		else if (orbs_num == 3)
			cell_value = 10;
	}

	return cell_value;
}


float attack_value_estimate(int orbX, int  orbY, Board& board, char colorPlayer) {
	int num = num_to_be_full(orbX, orbY, board);
	float value = -2 * cell_value_estimate(board.get_capacity(orbX, orbY), board.get_orbs_num(orbX, orbY)) / 3;
	float est = 0;

	if (orbX + 1 < ROW) {
		if (board.get_cell_color(orbX + 1, orbY) != colorPlayer && num_to_be_full(orbX + 1, orbY, board) <= num)
			return value;
		if (board.get_cell_color(orbX + 1, orbY) != colorPlayer && num_to_be_full(orbX + 1, orbY, board) > num)
			est += cell_value_estimate(board.get_capacity(orbX + 1, orbY), board.get_orbs_num(orbX + 1, orbY)) / 3;
	}
	if (orbX - 1 >= 0) {
		if (board.get_cell_color(orbX - 1, orbY) != colorPlayer && num_to_be_full(orbX - 1, orbY, board) <= num)
			return value;
		if (board.get_cell_color(orbX - 1, orbY) != colorPlayer && num_to_be_full(orbX - 1, orbY, board) > num)
			est += cell_value_estimate(board.get_capacity(orbX - 1, orbY), board.get_orbs_num(orbX - 1, orbY)) / 3;
	}
	if (orbY + 1 < COL) {
		if (board.get_cell_color(orbX, orbY + 1) != colorPlayer && num_to_be_full(orbX, orbY + 1, board) <= num)
			return value;
		if (board.get_cell_color(orbX, orbY + 1) != colorPlayer && num_to_be_full(orbX, orbY + 1, board) > num)
			est += cell_value_estimate(board.get_capacity(orbX, orbY + 1), board.get_orbs_num(orbX, orbY + 1)) / 3;
	}
	if (orbY - 1 >= 0) {
		if (board.get_cell_color(orbX, orbY - 1) != colorPlayer && num_to_be_full(orbX, orbY - 1, board) <= num)
			return value;
		if (board.get_cell_color(orbX, orbY - 1) != colorPlayer && num_to_be_full(orbX, orbY - 1, board) > num)
			est += cell_value_estimate(board.get_capacity(orbX, orbY - 1), board.get_orbs_num(orbX, orbY - 1)) / 3;
	}
	return est;
}


double orbcompare_est(int orbX, int orbY, int com_orbX, int com_orbY, Board board) {
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
	float score = 0;
public:
	MyTreeNode();
	MyTreeNode(Board board);
	MyTreeNode(Board board, int x, int y);
	void setNextNode(MyTreeNode* MyTreeNode);
	void setNextRoundNode(MyTreeNode* MyTreeNode);
	void setScore(float Score);
	MyTreeNode* getNextNode();
	MyTreeNode* getNextRoundNode();
	MyTreeNode* getTail();
	Board getBoardState();
	int getX();
	int getY();
	float getScore();
};

class MygameTree {
private:
	MyTreeNode* root;
	Player* player;
	Player* opponent;
public:
	MygameTree(Board board, Player* player);
	MyTreeNode* chooseOrb();   // Use the four-level look-ahead game tree to determine the next step of orb
	void insertNextRound(MyTreeNode* Node, Player currentTurn);
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

void MyTreeNode::setScore(float Score) {
	this->score = Score;
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

float MyTreeNode::getScore() {
	return this->score;
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

MyTreeNode* MygameTree::chooseOrb() {
	/*
	使用min-max方法來選擇放置哪個位置
	*/
	this->insertNextRound(this->root, *this->player);
	MyTreeNode* firstLevelNode = this->root->getNextRoundNode()->getNextNode();
	MyTreeNode* bestNode = this->root->getNextRoundNode()->getNextNode();
	int index[2];
	index[0] = firstLevelNode->getX();
	index[1] = firstLevelNode->getY();
	float bestScore = MinScore - 1;

	for (firstLevelNode; firstLevelNode != NULL; firstLevelNode = firstLevelNode->getNextNode()) {
		float tempScore = this->BoardEvaluator(firstLevelNode);
		if (tempScore == MaxScore) {
			bestNode = firstLevelNode;
			return bestNode;
		}
		insertNextRound(firstLevelNode, *this->opponent);
		MyTreeNode* secondLevelNode = firstLevelNode->getNextRoundNode()->getNextNode();
		float maxWinOneLevelScore = MinScore - 1;//用來記住跑完整個第二層的時候 自己最多會贏多少分

		for (secondLevelNode; secondLevelNode != NULL; secondLevelNode = secondLevelNode->getNextNode()) {
			float tempScore = this->BoardEvaluator(secondLevelNode);

			if (tempScore < MaxScore) {
				insertNextRound(secondLevelNode, *this->player);
				MyTreeNode* thirdLevelNode = secondLevelNode->getNextRoundNode()->getNextNode();
				float maxLoseSecondLevelScore = MinScore - 1;//用來記住跑完整個第三層的時候 對面的玩家最多會輸多少分
				for (thirdLevelNode; thirdLevelNode != NULL; thirdLevelNode = thirdLevelNode->getNextNode()) {
					thirdLevelNode->setScore(this->BoardEvaluator(thirdLevelNode));
					if (thirdLevelNode->getScore() > maxLoseSecondLevelScore)
						maxLoseSecondLevelScore = thirdLevelNode->getScore();
				}
				secondLevelNode->setScore((-1) * maxLoseSecondLevelScore);
			}
			else
				secondLevelNode->setScore(MaxScore);

			if (secondLevelNode->getScore() > maxWinOneLevelScore)
				maxWinOneLevelScore = secondLevelNode->getScore();
		}

		firstLevelNode->setScore((-1) * maxWinOneLevelScore);
		if (firstLevelNode->getScore() > bestScore) {
			bestScore = firstLevelNode->getScore();
			bestNode = firstLevelNode;
		}

	}
	return bestNode;
}

void MygameTree::insertNextRound(MyTreeNode* Node, Player currentTurn) {
	// Insert all the cases of next round in MygameTree
	// Input : currentTurn indicates which player is going to put the orb in this turn

	char color = currentTurn.get_color();
	Board cur_boardState = Node->getBoardState();

	// Find first next round node using random

	MyTreeNode* virtualNode = new MyTreeNode(cur_boardState, 0, 0);
	Node->setNextRoundNode(virtualNode);

	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			if (cur_boardState.get_cell_color(i, j) == color || cur_boardState.get_cell_color(i, j) == 'w') {
				Board tmp = cur_boardState;
				tmp.place_orb(i, j, &currentTurn);
				MyTreeNode* newNode = new MyTreeNode(tmp, i, j);
				MyTreeNode* tail = Node->getNextRoundNode()->getTail();
				tail->setNextNode(newNode);
			}
		}
	}
	return;
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
				orbScore += cell_value_estimate(board.get_capacity(i, j), board.get_orbs_num(i, j));
				orbScore += attack_value_estimate(i, j, board, colorPlayer);
				orbPlayer++;
			}
			else if (board.get_cell_color(i, j) == colorOpponent) {
				orbScore -= cell_value_estimate(board.get_capacity(i, j), board.get_orbs_num(i, j));
				orbOpponent++;
			}
		}
	}

	if (orbOpponent == 0 && orbPlayer != 1) {
		return MaxScore;
	}

	return orbScore;
}

void algorithm_A(Board board, Player player, int index[]) {
	MygameTree gt(board, &player);
	MyTreeNode* choose = gt.chooseOrb();
	index[0] = choose->getX();
	index[1] = choose->getY();

}

