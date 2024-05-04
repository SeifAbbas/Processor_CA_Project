#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 2048 // Words (each word is 4 bytes)
#define NUM_REGISTERS 33
#define INSTRUCTION_SIZE 32 // Size of each instruction in bits

typedef struct instruction
{
    int opcode;    // (bits 31:28) ~~ Opcode value (0-11)
    int rs;        // (bits 27:23) ~~ Source register 1 (for R-type and I-type instructions)
    int rt;        // (bits 22:18) ~~ Source register 2 (for R-type and I-type instructions)
    int rd;        // (bits 17:13) ~~ Destination register (for R-type instructions)
    int shamt;     // (bits 12:0)  ~~ Shift amount (for R-type instructions)
    int immediate; // (bits 17:0)  ~~ Immediate value (for I-type instructions)
    int address;   // (bits 27:0)  ~~ Address (for J-type instructions)
} instruction_t;

instruction_t parse_instructions(FILE *assembly_file, instruction_t *instructions)
{
    int instruction_count = 0;
    char line[100]; // Assuming each instruction fits on a single line under 100 characters

    while (fgets(line, sizeof(line), assembly_file) != NULL)
    {
        // Tokenize the line (separate opcode, operands)
        char *tokens[4]; // Assuming maximum of 4 tokens (opcode, rd, rs, rt/immediate)
        int num_tokens = tokenize(line, tokens, " ");

        if (num_tokens < 2)
        {
            // Handle error: Line with less than 2 tokens (opcode and operand)
            continue;
        }

        // Parse instruction based on opcode
        instruction_t instruction;
        if (strcmp(tokens[0], "ADD") == 0)
        {
            // ADD instruction format: ADD rs,rt,rd
            instruction.opcode = 0;           // opcode for ADD is 0
            instruction.rs = atoi(tokens[1]); // Convert register name (e.g., R1) to integer
            instruction.rt = atoi(tokens[2]);
            instruction.rd = atoi(tokens[3]);
            instructions[instruction_count] = instruction;
            instruction_count++;
        }
        else
        {
            // Handle error: Invalid instruction type
            continue;
        }
    }

    return *instructions;
}

int tokenize(char *line, char *tokens[], char *delimiter)
{
    int num_tokens = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, delimiter);
    }
    return num_tokens;
}

void fetch()
{
    // Fetch instruction from memory
}

void decode()
{
    // Decode instruction
}

void execute()
{
    // Execute instruction
}

void memory_access()
{
    // Access memory
}

void write_back()
{
    // Write back to registers
}

int main()
{
    // Memory (instruction and data segments)
    int memory[MEMORY_SIZE];

    // Registers
    int registers[NUM_REGISTERS];

    // Program Counter (PC)
    int pc = 0;

    // Loop until no more intructions to be read from the assembly file
    while (1)
    {
        // Fetch instruction from memory
        fetch();

        printf("Clock Cycle: %d\n");
        printf("IF: Instruction being fetched: ...\n");
        printf("ID: Instruction being decoded: ...\n");
        printf("EX: Instruction being executed: ...\n");
        printf("MEM: Instruction accessing memory: ...\n");
        printf("WB: Instruction writing back to registers: ...\n");

        // Print register and memory contents after the last clock cycle
        printf("Registers:\n");
        for (int i = 0; i < NUM_REGISTERS; i++)
        {
            printf("R%d: %d\n", i, registers[i]);
        }
        printf("Memory:\n");
        for (int i = 0; i < MEMORY_SIZE; i++)
        {
            // printf("Address 0x%04x: %d\n", i, memory[i]);
        }
    }

    return 0;
}
