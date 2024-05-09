#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MEMORY_SIZE 2048 // Words (each word is 4 bytes)
#define NUM_REGISTERS 33
#define INSTRUCTION_SIZE 32 // Size of each instruction in bits
#define MAX_TOKENS 100      // Define a maximum number of tokens

// Memory and registers
int memory[MEMORY_SIZE];
int registers[NUM_REGISTERS];
int instruction_count = 0;
int pc = 0;

// Function prototypes
int tokenize(char *line, char *tokens[], const char *delimiter);
void parse_instructions(FILE *assembly_file);
void fetch();
void decode(int instruction);

// Instruction fields
int opcode = 0;    // (bits 31:28) ~~ Opcode value (0-11)
int rs = 0;        // (bits 27:23) ~~ Source register 1 (for R-type and I-type instructions)
int rt = 0;        // (bits 22:18) ~~ Source register 2 (for R-type and I-type instructions)
int rd = 0;        // (bits 17:13) ~~ Destination register (for R-type instructions)
int shamt = 0;     // (bits 12:0)  ~~ Shift amount (for R-type instructions)
int immediate = 0; // (bits 17:0)  ~~ Immediate value (for I-type instructions)
int address = 0;   // (bits 27:0)  ~~ Address (for J-type instructions)

void parse_instructions(FILE *assembly_file)
{

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
        if (strcmp(tokens[0], "ADD") == 0)
        {
            // ADD instruction format: ADD rs,rt,rd
            // opcode for ADD is 0
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            rd = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | rs | rt | rd;
        }
        else if (strcmp(tokens[0], "SUB") == 0)
        {
            // SUB instruction format: SUB rs,rt,rd
            opcode = 1 << 28; // opcode for SUB is 1
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            rd = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | rs | rt | rd;
        }
        else if (strcmp(tokens[0], "MUL") == 0)
        {
            // MUL instruction format: MUL rd, rs, rt
            opcode = 2 << 28; // opcode for MUL is 2
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            rd = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | rs | rt | rd;
        }
        else if (strcmp(tokens[0], "MOVI") == 0)
        {
            // MOVI instruction format: MOVI rd, immediate
            opcode = 3 << 28; // opcode for MOVI is 3
            rs = atoi(tokens[1] + 1) << 23;
            immediate = atoi(tokens[2]);
            memory[instruction_count++] = opcode | rs | ((immediate & 0x3FFFF));
        }
        else if (strcmp(tokens[0], "JEQ") == 0)
        {
            // JEQ instruction format: JEQ rs, rt, address
            opcode = 4 << 28; // opcode for JEQ is 4
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | immediate;
        }
        else if (strcmp(tokens[0], "AND") == 0)
        {
            // AND instruction format: AND rd, rs, rt
            opcode = 5 << 28; // opcode for AND is 5
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            rd = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | rs | rt | rd;
        }
        else if (strcmp(tokens[0], "XORI") == 0)
        {
            // XORI instruction format: XORI rs, rt, immediate
            opcode = 6 << 28; // opcode for XORI is 6
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | ((immediate & 0x3FFFF));
        }
        else if (strcmp(tokens[0], "JMP") == 0)
        {
            // JMP instruction format: JMP address
            opcode = 7 << 28; // opcode for JMP is 7
            address = atoi(tokens[1]);
            memory[instruction_count++] = opcode | address;
        }
        else if (strcmp(tokens[0], "LSL") == 0)
        {
            // LSL instruction format: LSL rs, rt, shamt
            opcode = 8 << 28; // opcode for LSL is 8
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            shamt = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | shamt;
        }
        else if (strcmp(tokens[0], "LSR") == 0)
        {
            // LSR instruction format: LSR rs, rt, shamt
            opcode = 9 << 28; // opcode for LSR is 9
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            shamt = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | shamt;
        }
        else if (strcmp(tokens[0], "MOVR") == 0)
        {
            // MOVR instruction format: MOVR rs, rt, immediate
            opcode = 10 << 28; // opcode for MOVR is 10
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | ((immediate & 0x3FFFF));
        }
        else if (strcmp(tokens[0], "MOVM") == 0)
        {
            // MOVM instruction format: MOVM rs, rt, immediate
            opcode = 11 << 28; // opcode for MOVM is 11
            rs = atoi(tokens[1] + 1) << 23;
            rt = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | rs | rt | ((immediate & 0x3FFFF));
        }
        else
        {
            // Handle error: Invalid opcode
            printf("Error: Invalid opcode\n");
        }
    }
}

int tokenize(char *line, char *tokens[], const char *delimiter)
{
    if (line == NULL || tokens == NULL || delimiter == NULL)
    {
        printf("Error: NULL pointer passed to tokenize function.\n");
        exit(1);
    }

    int num_tokens = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        if (num_tokens >= MAX_TOKENS)
        {
            printf("Error: too many tokens. Increase MAX_TOKENS.\n");
            exit(1);
        }
        tokens[num_tokens] = token;
        num_tokens++;
        token = strtok(NULL, delimiter);
    }
    return num_tokens;
}

void fetch()
{
    int instruction = 0;
    for (int i = 0; i < instruction_count; i++)
    {
        instruction = memory[pc];
        decode(instruction);
        pc++;
    }
}

void decode(int instruction)
{
    opcode = (instruction & 0b11110000000000000000000000000000) >> 28;
    rs = (instruction & 0b00001111100000000000000000000000) >> 23;
    rt = (instruction & 0b00000000011111000000000000000000) >> 18;
    rd = (instruction & 0b00000000000000111110000000000000) >> 13;
    shamt = (instruction & 0b00000000000000000001111111111111);
    immediate = (instruction & 0b00000000000000111111111111111111);
    address = (instruction & 0b00001111111111111111111111111111);

    int sign_bit = immediate >> 17;
    if (sign_bit == 1)
    {
        immediate = immediate + 0b11111111111111000000000000000000;
    }

    printf("Instruction %i\n", pc);
    printf("opcode = %i\n", opcode);
    printf("rs = %i\n", rs);
    printf("rt = %i\n", rt);
    printf("rd = %i\n", rd);
    printf("shift amount = %i\n", shamt);
    printf("immediate = %i\n", immediate);
    printf("address = %i\n", address);
    printf("---------- \n");
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
    // Open the assembly file
    FILE *file = fopen("assembly.txt", "r");
    if (file == NULL)
    {
        printf("Error: Could not open file\n");
        return -1;
    }

    parse_instructions(file);
    fetch();

    // printf("%d\n", memory[0]);
    // printf("%d\n", memory[1]);

    // Close the file
    fclose(file);

    // MISSING IMPLEMENTATION
    //  ability to deal with Registers with numbers > 10 in parsing instructions and handle negative immediate values
    //  execute();
    //  memory_access();
    //  write_back();
    //  Pipleline stages
    //  Printings

    return 0;
}
