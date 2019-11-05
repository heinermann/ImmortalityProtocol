#pragma once
#include "../libdisasm/libdis.h"

namespace x86
{
  class Instruction;

  class Disassembler
  {
  public:
    Disassembler(x86_options options = x86_options::opt_none, DISASM_REPORTER reporter = nullptr, void *arg = nullptr);
    ~Disassembler();

    int disasm(void *buf, unsigned int buf_len, uint32_t buf_rva, unsigned int offset, Instruction *insn) const;
    int disasm(void *buf, uint32_t buf_rva, Instruction *insn) const;
    int disasm(void *buf, Instruction *insn) const;
  };

  extern Disassembler dasm;
}

