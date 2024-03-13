#pragma once

#include <cstdint>
#include <thread>
#include "board.h"
#include "xoroshiro128plus.h"

struct Node
{
  Node();
  Node(const Board& board);
  Node(const Node* other);
  ~Node();

  Board b;
  Node* root;
  Node* children[cols]; // syncs up with moves[cols]

  bool terminal;
  bool expanded;
  bool moves[cols]; // true for which moves have we used?

  float UCT;
  uint_fast8_t move = 69;
  uint_fast8_t inserted;
  int_fast32_t score;
  uint_fast32_t visits;
};

struct MCTS
{
  MCTS(Board& b);
  // copy constructor never used
  ~MCTS();

  Node* root;
  float EXPL = 0.58578643762690485; // WAY better than sqrt(2)
  int createdNodes = 0;
  std::thread workers[cols];
  //uint_fast8_t (*prngs[cols])(); // each thread has its own prng
  // or create/destroy instance of function every time running simluation?

  Node* select(Node* node, Node*& spare);
  Node* expand(Node* node);
  int_fast16_t simulate(Node* node, uint_fast32_t iter = 100);
  inline float calcUCT(Node* node);
  void backpropagate(Node* node, float reward);
  void task(uint_fast32_t iter, uint_fast8_t who);
  uint_fast8_t run(uint_fast16_t iter);
  uint_fast8_t bestMove(Node* node);

  // other functions, simulate only next 7 possible moves
  uint_fast8_t goofygoober(Node* node);
};
