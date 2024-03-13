#pragma once

#include <string>
#include "mcts.h"

void printT(const std::string& prefix, const Node* node, bool isLeft);
void printT(const Node* node);
