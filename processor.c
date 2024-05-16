#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Constants
#define MEMORY_SIZE 2048    // Words (each word is 4 bytes)
#define NUM_REGISTERS 32    // Number of registers
#define INSTRUCTION_SIZE 32 // Size of each instruction in bits
#define MAX_TOKENS 100      // Define a maximum number of tokens

// Memory and registers
int memory[MEMORY_SIZE];
int registers[NUM_REGISTERS] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
int valueR1 = 0;
int valueR2 = 0;
int valueR3 = 0;
int ALU_result_memory_location = 0;
int IF_ID_pipeline_register = 0;
int instruction_count = 0;
int total_cycles = 0;
int clock_cycle = 1;
int instruction = 0;
int pc = 0;

// Function prototypes
int tokenize(char *line, char *tokens[], const char *delimiter);
void parse_instructions(FILE *assembly_file);
void fetch();
void decode(int instruction);

// Instruction fields
int opcode = 0;    // (bits 31:28) ~~ Opcode value (0-11)
int R1 = 0;        // (bits 27:23) ~~ Source register 1 (for R-type and I-type instructions)
int R2 = 0;        // (bits 22:18) ~~ Source register 2 (for R-type and I-type instructions)
int R3 = 0;        // (bits 17:13) ~~ Destination register (for R-type instructions)
int shamt = 0;     // (bits 12:0)  ~~ Shift amount (for R-type instructions)
int immediate = 0; // (bits 17:0)  ~~ Immediate value (for I-type instructions)
int address = 0;   // (bits 27:0)  ~~ Address (for J-type instructions)

void parse_instructions(FILE *assembly_file)
{

    char line[100]; // Assuming each instruction fits on a single line under 100 characters

    while (fgets(line, sizeof(line), assembly_file) != NULL)
    {

        // Tokenize the line (separate opcode, operands)
        char *tokens[4]; // Assuming maximum of 4 tokens (opcode, R3, R1, R2/immediate)
        int num_tokens = tokenize(line, tokens, " ");

        if (num_tokens < 2)
        {
            // Handle error: Line with less than 2 tokens (opcode and operand)
            printf("Error: Invalid instruction format\n");
            break;
        }

        // Parse instruction based on opcode
        if (strcmp(tokens[0], "ADD") == 0)
        {
            // ADD instruction format: ADD R1,R2,R3
            // opcode for ADD is 0
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            R3 = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | R1 | R2 | R3;
        }
        else if (strcmp(tokens[0], "SUB") == 0)
        {
            // SUB instruction format: SUB R1,R2,R3
            opcode = 1 << 28; // opcode for SUB is 1
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            R3 = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | R1 | R2 | R3;
        }
        else if (strcmp(tokens[0], "MUL") == 0)
        {
            // MUL instruction format: MUL R3, R1, R2
            opcode = 2 << 28; // opcode for MUL is 2
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            R3 = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | R1 | R2 | R3;
        }
        else if (strcmp(tokens[0], "MOVI") == 0)
        {
            // MOVI instruction format: MOVI R3, immediate
            opcode = 3 << 28; // opcode for MOVI is 3
            R1 = atoi(tokens[1] + 1) << 23;
            immediate = atoi(tokens[2]);
            memory[instruction_count++] = opcode | R1 | ((immediate & 0x3FFFF));
        }
        else if (strcmp(tokens[0], "JEQ") == 0)
        {
            // JEQ instruction format: JEQ R1, R2, address
            opcode = 4 << 28; // opcode for JEQ is 4
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | immediate;
        }
        else if (strcmp(tokens[0], "AND") == 0)
        {
            // AND instruction format: AND R3, R1, R2
            opcode = 5 << 28; // opcode for AND is 5
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            R3 = atoi(tokens[3] + 1) << 13;
            memory[instruction_count++] = opcode | R1 | R2 | R3;
        }
        else if (strcmp(tokens[0], "XORI") == 0)
        {
            // XORI instruction format: XORI R1, R2, immediate
            opcode = 6 << 28; // opcode for XORI is 6
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | ((immediate & 0x3FFFF));
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
            // LSL instruction format: LSL R1, R2, shamt
            opcode = 8 << 28; // opcode for LSL is 8
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            shamt = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | shamt;
        }
        else if (strcmp(tokens[0], "LSR") == 0)
        {
            // LSR instruction format: LSR R1, R2, shamt
            opcode = 9 << 28; // opcode for LSR is 9
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            shamt = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | shamt;
        }
        else if (strcmp(tokens[0], "MOVR") == 0)
        {
            // MOVR instruction format: MOVR R1, R2, immediate
            opcode = 10 << 28; // opcode for MOVR is 10
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | ((immediate & 0x3FFFF));
        }
        else if (strcmp(tokens[0], "MOVM") == 0)
        {
            // MOVM instruction format: MOVM R1, R2, immediate
            opcode = 11 << 28; // opcode for MOVM is 11
            R1 = atoi(tokens[1] + 1) << 23;
            R2 = atoi(tokens[2] + 1) << 18;
            immediate = atoi(tokens[3]);
            memory[instruction_count++] = opcode | R1 | R2 | ((immediate & 0x3FFFF));
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
    instruction = memory[pc];
    IF_ID_pipeline_register = pc++;
    decode(instruction);
}

void decode(int instruction)
{
    opcode = (instruction >> 28) & 0xF;
    R1 = (instruction >> 23) & 0x1F;
    R2 = (instruction >> 18) & 0x1F;
    R3 = (instruction >> 13) & 0x1F;
    shamt = instruction & 0x1FFF;
    immediate = instruction & 0x3FFFF;
    address = instruction & 0xFFFFFFF;

    if (immediate & (1 << 17)) // Check if the sign bit is set
    {
        immediate |= 0xFFFC0000; // Sign extend the immediate value
    }

    valueR1 = registers[R1];
    valueR2 = registers[R2];
    valueR3 = registers[R3];

    printf("Instruction %i\n", pc);
    printf("opcode = %i\n", opcode);
    printf("R1 = %i\n", R1);
    printf("R2 = %i\n", R2);
    printf("R3 = %i\n", R3);
    printf("shift amount = %i\n", shamt);
    printf("immediate = %i\n", immediate);
    printf("address = %i\n", address);
    printf("valueR1 = %i\n", valueR1);
    printf("valueR2 = %i\n", valueR2);
    printf("valueR3 = %i\n", valueR3);
    printf("Instruction count: %d\n", instruction_count);
    printf("---------- \n");
}

void execute()
{
    switch (opcode)
    {
    case 0:
        valueR1 = valueR2 + valueR3;
        break;
    case 1:
        valueR1 = valueR2 - valueR3;
        break;
    case 2:
        valueR1 = valueR2 * valueR3;
        break;
    case 3:
        valueR1 = immediate;
        break;
    case 4:
        if (registers[R1] == valueR2)
        {
            pc += immediate;
        }
        break;
    case 5:
        valueR1 = valueR2 & valueR3;
        break;
    case 6:
        valueR1 = valueR2 ^ immediate;
        break;
    case 7:
        pc = (pc & 0xF0000000) | address;
        break;
    case 8:
        valueR1 = valueR2 << shamt;
        break;
    case 9:
        valueR1 = valueR2 >> shamt;
        break;
    default:
        ALU_result_memory_location = valueR2 + immediate;
        break;
    }
}

void memory_access()
{
    if (opcode == 10)
    {
        valueR1 = memory[ALU_result_memory_location];
    }
    else if (opcode == 11)
    {
        memory[ALU_result_memory_location] = valueR1;
    }
}

void write_back()
{
    int valid_opcodes[12] = {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0};

    if (valid_opcodes[opcode])
    {
        if (R1 == 0)
        {
            valueR1 = 0;
            registers[R1] = 0;
        }
        else
        {
            registers[R1] = valueR1;
        }
    }
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

    // Close the file
    fclose(file);

    // MISSING IMPLEMENTATION
    //  Pipleline stages
    //  Clock Cycles
    //  Printings

    return 0;
}
