#pragma once
#include "../libdisasm/libdis.h"
#include <string>

typedef x86_insn_type x86it;

namespace x86
{
  class Instruction : public x86_insn_t
  {
  public:
    Instruction();
    ~Instruction();

    std::string toString();
  };

}
