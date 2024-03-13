#include "mcts.h"
#include "board.h"
#include <iostream>

int main()
{
  while (1)
  {
    int move;
    Board b;
    do
    {
      std::cin >> move;
      b.dropPiece(move);
      b.printBoard();

      if (b.isDraw() || b.isWin())
        break;

      MCTS m(b);
      move = m.run(10000);
      b.dropPiece(move);
      b.printBoard();
    }
    while (!b.isDraw() && !b.isWin());
  }

  return 0;
}
