#include "Instruction.h"

namespace x86
{
  Instruction::Instruction()
  {
    memset(this,0,sizeof(x86_insn_t));
  }
  Instruction::~Instruction()
  {
    x86_oplist_free(this);
  }
  std::string Instruction::toString()
  {
    char szRet[128];
    szRet[0] = '\0';
    x86_format_insn(this, szRet, sizeof(szRet), x86_asm_format::intel_syntax);
    return std::string(szRet);
  }
}
