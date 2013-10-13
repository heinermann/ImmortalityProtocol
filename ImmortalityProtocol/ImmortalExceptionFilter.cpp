#include "ImmortalExceptionFilter.h"
#include "TopLevelExceptionFilter.h"
#include "Main.h"

#include <map>
#include <fstream>

#include "Disassembler.h"
#include "Instruction.h"
#include "ThreadManager.h"

#include "ExceptionContext.h"

std::map<DWORD,DWORD> exceptionCounts;

// Used to relocate the instruction pointer if it's pointing to an invalid memory location
void _declspec(naked) asm_ret_full() {  __asm mov esp,ebp
                                        __asm pop ebp
                                        __asm retn
                                     }
void _declspec(naked) asm_ret() {  __asm retn  }

// The primary top level exception filter
LONG WINAPI ImmortalExceptionFilter(EXCEPTION_POINTERS *ep)
{
  // Log file
  static std::ofstream logStream(getLogName() + ".log");

  // Initialize register classes and stuff
  x86::ExceptionContext ec(ep);
  x86::Register eax(ep->ContextRecord->Eax),
                ebx(ep->ContextRecord->Ebx),
                ecx(ep->ContextRecord->Ecx),
                edx(ep->ContextRecord->Edx),
                edi(ep->ContextRecord->Edi),
                esi(ep->ContextRecord->Esi),
                ebp(ep->ContextRecord->Ebp),
                eip(ep->ContextRecord->Eip),
                esp(ep->ContextRecord->Esp);

  exceptionCounts[eip]++;
  
  // Log the last fault instruction and the number of times it occured
  static DWORD lastEip = 0;
  static std::string lastInst;
  if ( eip != lastEip )
  {
    if ( lastEip != 0 )
    {
      logStream << std::hex << std::uppercase   << std::showbase    << lastEip << ": " 
                << std::dec << std::nouppercase << std::noshowbase  << lastInst
                << "  [" << exceptionCounts[lastEip] << "]" << std::endl << std::flush;
    }
    lastEip = eip;
    lastInst = "BAD_PTR";
  }

  // Invalid call / call to nullptr, EIP is pointing to invalid memory
  if ( eip.isBadCodePtr() )
  {
    // Redirect the instruction pointer so that it can return to its caller
    ErrBox("Bad code ptr.");
    eip = &asm_ret;
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  // If eip is executing the stack
  if ( eip.isOnStack() )
  {
    ErrBox("STOOOOOOP! Stack frame may be corrupt. This is kind of important.");
    eip = &asm_ret;
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  // Disassemble instruction
  x86::Instruction insn;
  int size = x86::dasm.disasm(eip, &insn);

  // Store the last instruction string
  if ( lastInst == "BAD_PTR" && insn.type != x86it::insn_invalid )
    lastInst = insn.toString();

  // Switch for instruction type
  switch ( insn.type )
  {
  case x86it::insn_invalid:	  // invalid instruction
    // skip instruction
    ErrBox("Invalid instruction.");
    ++eip;
    // @TODO: should possibly return from here instead
    return EXCEPTION_CONTINUE_EXECUTION;

  ////////////////////  insn_controlflow
  case x86it::insn_jcc:
  case x86it::insn_call:
  case x86it::insn_callcc:
  case x86it::insn_jmp:
  case x86it::insn_return:
    ErrBox("Exception on control flow");
    break;

  ///////////////////  insn_stack
  case x86it::insn_push:
  case x86it::insn_pushflags:
  case x86it::insn_pop:
  case x86it::insn_popflags:
  case x86it::insn_popregs:
  case x86it::insn_pushregs:
  case x86it::insn_enter:
  case x86it::insn_leave:
    ErrBox("Exception on stack");
    break;

  default:
    // skip instruction
    eip += size;
    return EXCEPTION_CONTINUE_EXECUTION;
  }

  writeExceptionCount();
  ErrBox( (insn.toString() + " --> " + ec.getName()).c_str() );
  return TopExceptionFilter.DefFilterProc(ep);
} /////////////////////////////////////////////
/////////////////////////////////////////////

std::string getLogName()
{
  char szFilename[MAX_PATH];
  GetModuleFileName(nullptr, szFilename, sizeof(szFilename));
  
  // Trim path and add extension
  std::string pathStr(szFilename);
  return pathStr.substr(0, pathStr.find_last_of(".")) += ".immortal";
}

void writeExceptionCount()
{
  std::ofstream ocounts(getLogName());
  
  ocounts << std::string(32,'-') << std::endl;
  ocounts << " ADDRESS : EXCEPTION TOLL  " << std::endl;
  ocounts << std::string(32,'-') << std::endl;
  for ( auto i : exceptionCounts )
    ocounts << ((void*)i.first) << " : " << i.second << std::endl;
  ocounts << std::flush;

  ocounts.close();
}
