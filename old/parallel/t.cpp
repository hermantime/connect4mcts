#include "board.h"
#include <iostream>
int main()
{
  Board b;
  do
  {
    int move;
    std::cin >> move;
    b.dropPiece(move);
    
    b.printBoard();
    std::cout << "\n";

    if (b.isDraw() || b.isWin())
      break;

    move = b.randomLegalMove();
    b.dropPiece(move);
    b.printBoard();
  }
  while (!b.isDraw() && !b.isWin());

  return 0;
}
