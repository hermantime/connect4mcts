#include "board.h"
#include <cassert>
#include <cstdint>
#include <iostream>

void Board::printBoard()
{
  for (uint_fast8_t i = 0; i < size; ++i)
  {
    std::cout << static_cast<unsigned>(getPiece(i)) << ' ';
    if ((i+1) % cols == 0)
      std::cout << "\n";
  }
}
// guaranteed no error, get moves from getLegalMoves
void Board::dropPiece(int_fast8_t col)
{
  for (;;)
  {
    assert(col+35 >= 0); // if not column full
    if (getPiece(col+35) == 0)
    {
      board[(col+35)*2] = !turn ? 0 : 1;
      board[(col+35)*2+1] = !turn ? 1 : 0;
      lastMove = col+35;
      turn = !turn;
      return;
    }
    col -= cols;
  }
  assert(false); // something went wrong
}

// index 0-41
int_fast8_t Board::getPiece(uint_fast8_t idx, int_fast8_t dx, int_fast8_t dy)
{
  int_fast8_t newIdx = ((idx / cols + dx) * cols + ((idx % cols) + dy));
  return (board[newIdx * 2] << 1) + board[newIdx * 2 + 1];
}

bool Board::isDraw()
{
  uint_fast8_t val = 0;
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    val += getPiece(i) == 0 ? 0 : 1;
  }
  state = 0;
  return val >= cols;
}

bool Board::validIndex(int_fast8_t dx, int_fast8_t dy)
{
  int_fast8_t move = (lastMove / cols + dx) * cols + ((lastMove % cols) + dy);
  bool cond = getPiece(move) == getPiece(lastMove) && move > 0;
  if (dx && dy)
  {
    return cond && move / cols - dx == lastMove / cols
                && (move % cols) - dy == lastMove % cols;
  }
  else if (!dx && dy)
  {
    return cond && move / cols == lastMove / cols;
  }
  else // dx && !dy
  {
    return cond && move % cols == lastMove % cols;
  }
}

bool Board::checkConsecutive(int_fast8_t dx, int_fast8_t dy)
{
  uint_fast8_t total = 1;
  auto consec = [this, &total](int_fast8_t dx, int_fast8_t dy)
  {
    for (int_fast8_t cnt = 1, ndx = dx, ndy = dy;
         cnt < 4 && validIndex(ndx, ndy);
         ndx += dx, ndy += dy, cnt++)
    total++;
  };
  consec(dx, dy);
  if (total > 3) return true;
  consec(-dx, -dy);
  return total > 3;
}

bool Board::checkWin()
{
  if (lastMove == UINT_FAST8_MAX)
    return false;
  state = turn == ogTurn ? 1 : -1;
  return checkConsecutive(0, 1) || checkConsecutive(1, 0) ||
         checkConsecutive(1, 1) || checkConsecutive(-1, 1);
}

// returns bitset where, 0 => cannot drop, 1 => can drop
std::bitset<Board::cols> Board::legalMoves()
{
  uint_fast8_t cs = 0;
  for (uint_fast8_t i = 0; i < cols; ++i)
  {
    cs = cs * 2 + (getPiece(i) ? 0 : 1);
  }
  return std::bitset<cols> (cs);
}

uint_fast8_t Board::randomLegalMove()
{
  assert(!isDraw());
  std::bitset<cols> moves = legalMoves();
  assert(!moves.none()); // draw
  uint_fast8_t move;
  do
  {
    move = std::rand() % cols;
  }
  while (moves.to_string()[move] == '0'); // edit, this sucks
  return move;
}
