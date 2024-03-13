#include <iostream>
#include "mcts.h"
#include "board.h"

int main()
{
  Board b(true);

  while (!b.isDraw() && !b.isWin())
  {
    int move;
    std::cin >> move;
    b.dropPiece(move);
    b.printBoard();
    std::cout << "\n";

    if (b.isDraw() || b.isWin())
      break;

    MCTS m;
    move = m.run(b);
    b.dropPiece(move);
    b.printBoard();
    std::cout << "\n";
  }

  return 0;
}
