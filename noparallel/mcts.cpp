#include "mcts.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

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
  return selectBestChild(root)->simulatedMove;
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

Node* MCTS::selectBestChild(Node* node)
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
  for (uint_fast8_t i = idx+1; i < node->inserted; ++i)
  {
    if (node->children[i] != nullptr)
    {
      if (best->uctScore < node->children[i]->uctScore)
        best = node->children[i];
    }
  }
  assert(best != nullptr);
  return best;
}

Node* MCTS::select(Node* node)
{
  if (node->inserted == moves) // expand current tree
  {
    while (node->inserted != 0) // find leaf node node is null b/c there is a full column here
    {
      node = selectBestChild(node);
    }
  }
  return node;
}

void MCTS::backpropogate(Node* node, float reward)
{
  while (node != nullptr)
  {
    node->visits++;
    node->score += reward;
    reward *= -1;
    if (node->root != nullptr)
    {
      float UCT = 1.0 * node->score / node->visits + (2 - sqrt(2))
                * sqrt(2*log(node->root->visits) / node->visits);
      node->uctScore = UCT;
    }
    node = node->root;
  }
}

uint_fast8_t MCTS::run(Board& b, uint_fast16_t iter)
{
  root = new Node(b);
  for (uint16_t i = 0; i < iter; ++i)
  {
    Node* selected = select(root);
    expand(selected);
  }
  uint_fast8_t move = selectBestChild(root)->simulatedMove;
  delete root;
  return move;
}

// take into account for terminal state!!!!
void MCTS::expand(Node* node)
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

      int_fast8_t res = simulate(newNode);
      backpropogate(newNode, res);
    }
  }
}
