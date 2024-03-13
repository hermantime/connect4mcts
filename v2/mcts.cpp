#include <cmath>
#include <cassert>
#include "mcts.h"
#include "xoroshiro128plus.h"
#include "printtree.h"
#include <cstdint>
#include <iostream>
#include <thread>

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

  int cnt = 0;
  float UCT;
  Node* temproot = node;
  while (node->expanded) // this is nullptr, better conditionals
  {
    if (cnt == 300)
    {
      std::cout << "sel\n";
      printT(temproot);
      std::cout << '\n';
      assert(node != nullptr);
      printT(node);
      assert(cnt != 300);
    }
    cnt++;
    Node* best;
    UCT = -INFINITY;
    for (uint_fast8_t i = 0; i < cols; ++i)
    {
      if (node->children[i])
      {
        if (UCT < node->children[i]->UCT)
        {
          best = node->children[i];
          UCT = node->children[i]->UCT;
        }
      }
    }
    if (UCT == -INFINITY) // case where all children are terminal, go up the tree, try to find non-terminal node
    {
      if (node == temproot) // search has hit the root node of this subtree
        break;

      node->UCT = UCT;
      node->terminal = true;
      node = node->root;
    }
    else
      node = best;
  }
  if (UCT == -INFINITY || node->terminal)
  {
    spare = node;
    return nullptr;
  }
  return node;
}

Node* MCTS::expand(Node* node)
{
  assert(!node->terminal);
  assert(!node->expanded);
  xoroshiro128plus prng;
  uint_fast8_t move;
  int i = 0;
  do
  {
    i++;
    move = prng.next() % cols;
  }
  while (node->moves[move] && i != 200);
  if (i == 200)
  {
    std::cout << "exp\n";
    node->b.printBoard();
    printT(root);
    std::cout << "\n";
    printT(node);
    std::cout << "\n";
    assert(i != 200);
  }
  //createdNodes++; wow something as simple as this can create a datarace!
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
  }
  newNode->root = node;
  node->moves[move] = true;
  node->children[node->inserted++] = newNode;
  if (node->inserted == cols)
  {
    node->expanded = true;
  }
  return newNode;
}

int_fast16_t MCTS::simulate(Node* node, uint_fast32_t iter)
{
  if (node->b.isDraw())
  {
    return -INFINITY;
  }
  int_fast16_t score = 0;
  for (uint_fast16_t i = 0; i < iter; ++i)
  {
    int cnt = 0;
    Board copy(node->b);
    xoroshiro128plus prng;
    while (!copy.isDraw() && !copy.isWin())
    {
      uint_fast8_t move;
      assert(!copy.isDraw());
      do
      {
        move = prng.next() % cols;
      }
      while (!copy.legalMove(move));
      copy.dropPiece(move);
      assert(cnt != 43);
      cnt++;
    }
    score += copy.state;
  }
  return score;
}

float MCTS::calcUCT(Node* node)
{
  return 1.0 * node->score / node->visits + EXPL
     * sqrt(2 * log(node->root->visits) / node->visits);
}

void MCTS::backpropagate(Node* node, float reward)
{
  int cnt = 0;
  while (node) // != nullptr
  {
    assert(cnt != 200);
    node->visits++;
    if (node->root)
    {
      node->score += reward;
      node->UCT = calcUCT(node);
      reward *= reward == -INFINITY ? 1 : -1;
    }
    node = node->root;
    cnt++;
  }
}

void MCTS::task(uint_fast32_t iter, uint_fast8_t who)
{
  for (uint_fast32_t i = 0; i < iter; ++i)
  {
    Node* spare; // temp solution
    Node* selected = select(root->children[who], spare);
    if (!selected)
    {
      assert(spare != nullptr);
      backpropagate(spare, -spare->score);
      if (spare != root->children[who])
        spare->UCT = -INFINITY;
      continue;
    }
    Node* expanded = expand(selected);
    float score = simulate(expanded, 85);
    backpropagate(expanded, score);
  }
}

uint_fast8_t MCTS::run(uint_fast16_t iter)
{
  // expand base 7 children
  for (uint_fast8_t i = 0; i < cols; ++i)
    expand(root);
  /*
  for (int i = 0; i < cols; ++i)
  {
    std::cout << root->children[i]->terminal << "@" << (int) root->children[i]->move << " ";
  }
  std::cout << "\n";
   */
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    if (!root->children[i]->terminal)
    {
      root->children[i]->root = nullptr;
      workers[i] = std::thread([&, iter, i](){task(iter, i);});
    }
  }
  for (uint_fast8_t i = 0; i < cols; ++i)
    if (workers[i].joinable())
    {
      workers[i].join();
    }

  for (uint_fast8_t i = 0; i < cols; ++i)
    root->children[i]->root = root; // switching these two lines prevented a data race?

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
