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

      if (b.isWin() || b.isDraw())
        break;

      MCTS m(b);
      move = m.run(5000, 333, 3);
      b.dropPiece(move);
      b.printBoard();
    }
    while (!b.isDraw() && !b.isWin());
    std::cout << (b.state == 1 ? "Nice!\n\n" : "Aww man!\n\n");
  }

  return 0;
}
