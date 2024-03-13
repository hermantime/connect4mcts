#include <iostream>
#include "mcts.h"
#include "printtree.h"

void printT(const std::string& prefix, const Node* node, bool isLeft)
{
  if( node != nullptr )
  {
    std::cout << prefix;
    std::cout << (isLeft ? "├──" : "└──" );

    std::cout << (int) node->move << ' ' << (float) node->UCT << std::endl;

    for (int i = 0; i < cols; ++i)
    {
      if (node->children[i])
      {
        printT( prefix + (isLeft ? "│   " : "    "), node->children[i], true);
      }
    }
  }
}
void printT(const Node* node)
{
    printT("", node, false);
}
