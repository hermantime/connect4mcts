#include "board.h"
#include "mcts.h"

int main()
{
  for (int i = 0; i < 100; ++i)
  {
    Board b;
    do
    {
      MCTS m(b);
      uint_fast8_t move = m.run(1000);
      b.dropPiece(move);
      //b.printBoard();
    }
    while (!b.isDraw() && !b.isWin());
    b.printBoard();
  }
  return 0;
}

// Uncomment printBT if you want to see the Tree after each run
//printT(m.root);
//std::cout << "Move " << (int) move << "\n";
//std::cout << "Turn " << b.turn << "\n";
//std::cout << "Created " << m.createdNodes << "\n";
