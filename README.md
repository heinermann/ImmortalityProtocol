Immortality Protocol
====================

A 32-bit Windows DLL that tries to prevent its host process from crashing.

## How It Works ##
It works by
 1. Creating an Unhandled Exception Filter with Windows API's `SetUnhandledExceptionFilter`.
 2. Using libdisasm to obtain the size of the instruction being pointed to by the `eip` register.
 3. Modifying the context's `eip` register to skip the instruction.
 4. Continuing program execution on the modified context with the `EXCEPTION_CONTINUE_EXECUTION` constant.

## NOTICE ##
Using the program WILL cause UNDEFINED BEHAVIOUR. I take no responsibility for any damages caused by this program. You have been warned.
