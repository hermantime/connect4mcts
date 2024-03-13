#pragma once

#include <cstdint>

/*
!!!! NOTE !!!!!
THIS CODE IS FROM:  https://xoroshiro.di.unimi.it/xoroshiro128plus.c
!!!!!!!!!!!!!!!
*/

struct xoroshiro128plus
{
  uint64_t s[2];

  xoroshiro128plus();

  inline uint64_t rotl(const uint64_t x, int k);
  uint64_t next();
  void jump();
  void long_jump();

  uint_fast8_t move();
};
