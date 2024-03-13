#include <iostream>
#include "mcts.h"
#include "board.h"

int main()
{
  Board b(true);

  while (!b.isDraw() && !b.checkWin())
  {
    int move;
    MCTS m;
    move = m.run(b);
    b.dropPiece(move);
    b.printBoard();
    std::cout << "\n";
  }

  return 0;
}
