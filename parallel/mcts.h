#pragma once

#include "board.h"
#include <cstdint>
#include <thread>

struct Node;

static constexpr uint8_t moves = 7;

struct MCTS
{
  std::thread workers[moves];
  int createdNodes = 0;
  int deletedNodes = 0;
  Node* root = nullptr;

  int_fast8_t getBestAction();
  [[nodiscard]] int_fast8_t simulate(Node* node);
  uint_fast8_t run(Board& b, uint_fast16_t iter = UINT_FAST16_MAX);


  Node* selectBestChild(Node* node);
  Node* select(Node* node);

  void threadRun(uint_fast8_t child, uint_fast16_t iter = UINT_FAST16_MAX);
  void backpropogate(Node* node, float reward);
  void expand(Node* node);
};

struct Node
{
  bool terminal;
  bool expanded;
  float uctScore = 0.0;
  uint8_t inserted = 0;
  int_fast32_t score = 0; // wins, draws, losses
  uint_fast32_t visits = 0;
  uint_fast8_t simulatedMove;
  Board b;
  Node* root = nullptr;
  Node* children[moves]; // only 7 children per node

  Node();
  Node(bool turn);
  Node(Node* r);
  Node(const Board& b);
  Node(const Node* other);
  ~Node();
};
