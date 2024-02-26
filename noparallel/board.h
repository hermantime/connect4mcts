#pragma once

#include <cstdint>
#include <bitset>

struct Board
{
  Board() = default;
  Board(bool t) : turn(t), ogTurn(t) {}
  Board(const Board& other)
    : turn(other.turn), ogTurn(other.ogTurn), state(other.state),
      lastMove(other.lastMove), board(other.board) {}
  static constexpr uint_fast8_t size = 42;
  static constexpr uint_fast8_t rows = 6;
  static constexpr uint_fast8_t cols = 7;

  bool turn;
  bool ogTurn; // original turn
  int_fast8_t state; // 1 is win, 0 is draw, -1 is loss
  uint_fast8_t lastMove = UINT_FAST8_MAX; // will have been done by !turn
  // maybe keep count of all pieces??
  std::bitset<84> board;

  void printBoard();
  void dropPiece(int_fast8_t col);

  // iterates through first row of bitset
  // checks whether all bits are taken or not
  bool isDraw();
  bool validIndex(int_fast8_t dx, int_fast8_t dy);
  bool checkConsecutive(int_fast8_t dx, int_fast8_t dy);
  // checks 4-in-a-row from most recent move
  bool checkWin();

  int_fast8_t getPiece(uint_fast8_t idx, int_fast8_t dx = 0, int_fast8_t dy = 0);
  // for each column (0-6) in the first row
  // returns bitset 0/1 whether spot is free
  uint_fast8_t randomLegalMove();

  std::bitset<cols> legalMoves(); // char c[7], 1 byte less
};
