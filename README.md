# Mini-Processor

## Overview
This project is a simple implementation of a processor in C. It simulates the fetch-decode-execute cycle of a processor and supports a subset of assembly instructions.

## File Structure
- `processor.c`: The main source file that contains the implementation of the processor.
- `assembly.txt`: A text file that contains the assembly instructions to be executed by the processor.

## How to Run
1. Compile the `processor.c` file using a C compiler. For example, you can use gcc: `gcc -o processor processor.c`
2. Run the compiled program: `./processor`

## Features
- Fetch, decode, and execute cycle
- Support for R-type, I-type, and J-type instructions
- Pipeline stages: fetch, decode, execute, memory access, and write back
- Support pipeline hazard: control hazard

## Instructions Supported
- ADD
- SUB
- MUL
- MOVI
- JEQ
- AND
- XORI
- JMP
- LSL
- LSR
- MOVR
- MOVM

More information about the instructions can be found in the `Description.pdf` file.

## Limitations
- No HALT instruction, program stops when pc reach last instruction.