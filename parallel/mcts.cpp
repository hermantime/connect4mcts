#include <atomic>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <thread>

#include "mcts.h"
#include "printtree.h"
#include "xoroshiro128plus.h"

Node::Node() = default;

Node::Node(const Board& board) : b(board) {}

Node::Node(const Node* other) : root(other->root), terminal(other->terminal),
    expanded(other->expanded), UCT(other->UCT), inserted(other->inserted),
    score(other->score), visits(other->visits)
{
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    if (children[i])
      delete children[i];
    children[i] = other->children[i];
    moves[i] = other->moves[i];
  }
}

Node::~Node()
{
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    if (children[i])
      delete children[i];
  }
}


MCTS::MCTS(Board& b)
{
  root = new Node(b);
  root->visits++;
}

MCTS::~MCTS()
{
  delete root;
}

Node* MCTS::select(Node* node, Node*& spare)
{
  if (!node->expanded)
    return node;

  float UCT;
  //Node* temproot = node;
  while (node->expanded) // this is nullptr, better conditionals
  {
    Node* best;
    UCT = -INFINITY;
    for (uint_fast8_t i = 0; i < cols; ++i)
    {
      if (!node->children[i]->terminal)
      {
        if (UCT < node->children[i]->UCT)
        {
          best = node->children[i];
          UCT = node->children[i]->UCT;
        }
      }
    }
    if (UCT == -INFINITY) // case where all children are terminal, skip to backpropagation
    {
      spare = node;
      return nullptr;
    }

    else
      node = best;
  }
  //if (UCT == -INFINITY || node->terminal)
  //{
  //  spare = node;
  //  return nullptr;
  //}
  if (node->terminal || node->expanded)
  {
    std::cout << "Term " << node->terminal << " exp " << node->expanded << "\n";
    printT(root);
    printT(node);
    node->b.printBoard();
    assert(false);
  }
  return node;
}

Node* MCTS::expand(Node* node)
{
  assert(!node->terminal);
  assert(!node->expanded);
  xoroshiro128plus prng;
  uint_fast8_t move;
  do
  {
    move = prng.next() % cols;
  }
  while (node->moves[move]);
  Node* newNode = new Node();
  if (node->b.legalMove(move))
  {
    newNode->b = node->b;
    newNode->move = move;
    newNode->b.dropPiece(move);
  }
  else
  {
    newNode->terminal = true;
    newNode->UCT = -INFINITY;
  }

  newNode->root = node;
  node->moves[move] = true;
  node->children[node->inserted++] = newNode;
  if (node->inserted == cols)
    node->expanded = true;

  return newNode;
}

int_fast16_t MCTS::simulate(Node* node, uint_fast32_t iter, uint_fast8_t simThreads)
{
  if (node->b.isDraw())
    return -node->score;

  std::atomic<uint_fast64_t> score = 0;
  Board cc = node->b;
  xoroshiro128plus prng;
  auto simTask = [&cc, &score, iter](xoroshiro128plus prng) {
    int_fast32_t s = 0;
    for (uint_fast16_t i = 0; i < iter; ++i)
    {
      Board copy(cc);
      while (!copy.isDraw() && !copy.isWin())
      {
        uint_fast8_t move;
        do
        {
          move = prng.next() % cols;
        }
        while (!copy.legalMove(move));
        copy.dropPiece(move);
      }
      s += copy.state;
    }
    score += s;
  };

  std::thread simWorkers[simThreads];
  for (uint_fast8_t i = 0; i < simThreads; ++i)
  {
    xoroshiro128plus prng;
    simWorkers[i] = std::thread(simTask, prng);
  }
  for (uint_fast8_t i = 0; i < simThreads; ++i)
  {
    if (simWorkers[i].joinable())
      simWorkers[i].join();
  }

  return score;
}

float MCTS::calcUCT(Node* node)
{
  return 1.0 * node->score / node->visits + EXPL
     * sqrt(2 * log(node->root->visits) / node->visits);
}

void MCTS::backpropagate(Node* node, float reward, uint_fast8_t who)
{
  while (node) // != nullptr
  {
    node->visits++;
    if (node->root) // error here, won't fully backpropagate up to "root", stops at the temp root, which also needs to be updated
    {
      node->score += reward;
      node->UCT = calcUCT(node);
      reward *= -1;
    }
    node = node->root;
  }
}

void MCTS::task(uint_fast32_t loopIter, uint_fast32_t simIter, uint_fast8_t simThreads, uint_fast8_t who)
{
  for (uint_fast32_t i = 0; i < loopIter; ++i)
  {
    Node* spare; // temp solution
    Node* selected = select(root->children[who], spare);
    if (!selected)
    {
      assert(spare != nullptr);
      backpropagate(spare, spare->b.state * simIter, who);
      continue;
    }
    Node* expanded = expand(selected);
    float score = simulate(expanded, simIter, simThreads);
    backpropagate(expanded, score, who);
  }
}

uint_fast8_t MCTS::run(uint_fast32_t loopIter, uint_fast32_t simIter, uint_fast8_t simThreads)
{
  // expand base 7 children
  for (uint_fast8_t i = 0; i < cols; ++i)
    expand(root);

  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    if (!root->children[i]->terminal)
    {
      root->children[i]->root = new Node();
      workers[i] = std::thread([&, loopIter, simIter, i]()
                   {task(loopIter, simIter, simThreads, i);});
    }
  }
  for (uint_fast8_t i = 0; i < cols; ++i)
    if (workers[i].joinable())
    {
      workers[i].join();
      delete root->children[i]->root;
      root->children[i]->root = root;
    }

  return bestMove(root);
}

uint_fast8_t MCTS::bestMove(Node* node)
{
  float UCT = -INFINITY;
  uint_fast8_t move;
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    if (node->children[i] && !node->children[i]->terminal)
    {
      if (node->children[i]->UCT > UCT)
      {
        UCT = node->children[i]->UCT;
        move = node->children[i]->move;
      }
    }
  }
  return move;
}
