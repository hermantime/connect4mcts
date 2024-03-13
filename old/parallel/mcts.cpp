#include "mcts.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <sys/types.h>




// DRAWE DRAW DRAW



constexpr float explConst = 2 - 1.4142135623731;

Node::Node() {}
Node::Node(bool turn) : b(turn) {}
Node::Node(const Board& b) : b(b) {}
Node::Node(Node* r) : b(r->b), root(r) {}
Node::~Node() {
  for (uint_fast8_t i = 0; i < moves; ++i)
  {
    if (children[i] != nullptr)
    {
      delete children[i];
      children[i] = nullptr;
    }
  }
}

int_fast8_t MCTS::getBestAction()
{
  return select(root)->simulatedMove;
}

int_fast8_t MCTS::simulate(Node* node)
{
  Board copy(node->b);
  while (!copy.isDraw() && !copy.checkWin())
  {
    uint_fast8_t move = copy.randomLegalMove();
    copy.dropPiece(move);
  }
  return copy.state;
}

Node* MCTS::selectBestUCT(Node* node)
{
  if (node->inserted < 1)
    return node;

  uint_fast8_t idx = 0;
  while (node->children[idx] == nullptr)
  {
    idx++;
    assert(idx != moves); // all children are null
  }

  Node* best = node->children[idx];
  for (uint_fast8_t i = idx+1; i < moves; ++i)
  {
    if (node->children[i] != nullptr)
    {
      if (best->uctScore < node->children[i]->uctScore)
      {
        best = node->children[i];
      }
    }
  }
  return best;
}
/*
Node* MCTS::select(Node* node)
{
  if (node->expanded)
  {
    while (node->inserted != 0) // find leaf node
    {
      node = selectBestUCT(node);
    }
  }
  assert(node->inserted == 0);
  return node;
}
*/

Node* MCTS::select(Node* node)
{
  if (!node->expanded)
  {
    return node;
  }

  uint_fast8_t idx = 0;
  while (node->children[idx] == nullptr)
  {
    idx++;
    assert(idx != moves); // all children nullptrs even though fully expanded terminal
  }
  Node* best = node->children[idx];
  for (uint_fast8_t i = idx + 1; i < moves; ++i)
  {
    if (node->children[i]->uctScore > best->uctScore)
    {
      best = node->children[i];
    }
  }
  assert(!best->expanded);
  return best;
}

void MCTS::backpropogate(Node* node, float reward) // penalty??
{
  while (node != nullptr)
  {
    node->visits++;
    if (node->root != nullptr)
    {
      node->score += reward;
      float UCT = 1.0 * node->score / node->visits + explConst
                * sqrt(2*log(node->root->visits) / node->visits);
      node->uctScore = UCT;
      reward *= -1;
    } // update formula
    node = node->root;
  }
}

uint_fast8_t MCTS::run(Board& b)
{
  root = new Node(b);
  Node* selected = select(root);
  expand(selected);

  for (uint_fast8_t i = 0; i < moves; ++i)
    if (root->children[i] != nullptr)
    {
      workers[i] = std::thread([&, i](){threadRun(i);});
    }

  for (uint_fast8_t i = 0; i < moves; ++i)
    if (root->children[i] != nullptr)
    {
      workers[i].join();
    }

  uint_fast8_t move = selectBestUCT(root)->simulatedMove;
  delete root;
  return move;
}

void MCTS::threadRun(uint_fast8_t child, uint_fast16_t iter)
{
  for (uint_fast16_t i = 0; i < 1000; ++i)
  {
    Node* selected = select(root->children[child]);
    testExpand(selected);
  }
}

void MCTS::testExpand(Node* node)
{
  Node* newNode = new Node(node->b);
  newNode->root = node;
  node->children[node->inserted++] = newNode;
  int_fast16_t res;
  std::cout << node->simulatedMoves << "\n";
  if (!node->simulatedMoves.all())
  {
    std::cout << "in\n";
    uint_fast8_t move;
    do
    {
      std::cout << "sim\n";
      move = std::rand() % moves;
    }
    while (node->simulatedMoves[moves - (move + 1)]); //  != 0
    node->simulatedMoves[moves - (move + 1)].flip();

    newNode->b.dropPiece(move);
    newNode->simulatedMove = move;
    for (uint_fast8_t i = 0; i < INT_FAST8_MAX; ++i)
      res += simulate(newNode);
  }
  else
  {
    std::cout << "ad\n";
    newNode->terminal = true;
    res = -INT_FAST8_MAX;
  }
  backpropogate(newNode, res);
}

// take into account for terminal state!!!!
void MCTS::expand(Node* node) // one move at a time
{
  // j because bitsets are indexed reverse
  // 1111101
  // b.flip(0)
  // 1111100
  std::bitset<moves> legalMoves = node->b.legalMoves();
  for (int i = 0, j = moves-1; i < moves; ++i, --j)
  {
    if (legalMoves[j])
    {
      createdNodes++;
      Node* newNode = new Node(node->b);
      newNode->b.dropPiece(i);
      newNode->simulatedMove = i;
      newNode->root = node;
      node->children[node->inserted++] = newNode;
      int_fast8_t res;
      for (uint_fast8_t i = 0; i < INT_FAST8_MAX; ++i)
        res += simulate(newNode);
      backpropogate(newNode, res);
    } // terminal state => backpropogate bad move
  }   // logs -> turn, states
  node->expanded = true;
}

/*
moves high in score for player are also favorable for the bot to move to
blocking possible winning moves

Basic Approach:
When the selection step leads to a terminal state (a leaf node), you should still perform backpropagation.
Update the tree’s score and visit count based on whether it’s a win or loss.
Essentially, treat it as if you made an instant simulation step.
This ensures that the tree statistics are updated correctly.

Refinements:
For losing terminal nodes:
Assign a very large negative score (or even negative infinity).
This ensures that the node won’t be chosen in the next selection step unless other moves are equally bad.
For winning terminal nodes:
Assign a very large positive score (or even positive infinity).
Additionally, subtract a negative score from the immediate parent.
This prevents the parent from being chosen, as it’s clearly a losing state for that player.
These refinements help save computational resources by avoiding unnecessary simulations.
*/
