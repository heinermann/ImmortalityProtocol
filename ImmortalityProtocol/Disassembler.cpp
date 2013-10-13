#include "Disassembler.h"
#include "Instruction.h"

namespace x86
{
  Disassembler dasm(x86_options::opt_ignore_nulls);


  Disassembler::Disassembler(enum x86_options options, DISASM_REPORTER reporter, void *arg)
  {
    x86_init(options, reporter, arg);
  }

  Disassembler::~Disassembler()
  {
    x86_cleanup();
  }

  int Disassembler::disasm(void *buf, unsigned int buf_len, uint32_t buf_rva, unsigned int offset, Instruction *insn) const
  {
    return x86_disasm((unsigned char*)buf, buf_len, buf_rva, offset, insn);
  }
  int Disassembler::disasm(void *buf, uint32_t buf_rva, Instruction *insn) const
  {
    return this->disasm(buf, x86_max_insn_size(), buf_rva, 0, insn);
  }
  int Disassembler::disasm(void *buf, Instruction *insn) const
  {
    return this->disasm(buf, x86_max_insn_size(), (uint32_t)buf, 0, insn);
  }

}

