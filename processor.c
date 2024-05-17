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
int registers[NUM_REGISTERS] = {0};

int instruction_count = 0;
int clock_cycle = 1;
int pc = 0;

// Function prototypes
int tokenize(char *line, char *tokens[], const char *delimiter);
void parse_instructions(FILE *assembly_file);
void fetch();
void decode();
void print_registers();

// Instruction fields
typedef struct
{
    int flush_flag;
    int instruction;
    int opcode;    // (bits 31:28) ~~ Opcode value (0-11)
    int R1;        // (bits 27:23) ~~ Source register 1 (for R-type and I-type instructions)
    int R2;        // (bits 22:18) ~~ Source register 2 (for R-type and I-type instructions)
    int R3;        // (bits 17:13) ~~ Destination register (for R-type instructions)
    int shamt;     // (bits 12:0)  ~~ Shift amount (for R-type instructions)
    int immediate; // (bits 17:0)  ~~ Immediate value (for I-type instructions)
    int address;   // (bits 27:0)  ~~ Address (for J-type instructions)
    int pc;
    int valueR1;
    int valueR2;
    int valueR3;
    int ALU_result_memory_location;
} instruction_t;

instruction_t pipeline[5];

void pass_pipeline(instruction_t *from, instruction_t *to)
{
    to->flush_flag = from->flush_flag;
    to->instruction = from->instruction;
    to->opcode = from->opcode;
    to->R1 = from->R1;
    to->R2 = from->R2;
    to->R3 = from->R3;
    to->shamt = from->shamt;
    to->immediate = from->immediate;
    to->address = from->address;
    to->pc = from->pc;
    to->valueR1 = from->valueR1;
    to->valueR2 = from->valueR2;
    to->valueR3 = from->valueR3;
    to->ALU_result_memory_location = from->ALU_result_memory_location;
}

void parse_instructions(FILE *assembly_file)
{
    int opcode = 0;
    int R1 = 0;
    int R2 = 0;
    int R3 = 0;
    int shamt = 0;
    int immediate = 0;
    int address = 0;

    char line[100]; // Assuming each instruction fits on a single line under 100
                    // characters

    while (fgets(line, sizeof(line), assembly_file) != NULL)
    {

        // Tokenize the line (separate opcode, operands)
        char *tokens[4]; // Assuming maximum of 4 tokens (opcode, R3, R1,
                         // R2/immediate)
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
            opcode = 0;
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

// Print the register contents
void print_registers()
{
    printf("Registers:\n");
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        printf("R%d: %d\n", i, registers[i]);
    }
}
// Print the memory contents
void print_memory()
{
    printf("Registers:\n");
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        printf("position%d: %d, ", i, memory[i]);
    }
    printf("\n");
}

void fetch()
{
    pipeline[0].flush_flag = 0;
    pipeline[0].instruction = memory[pc];
    pipeline[0].pc = ++pc;
    if (pc <= instruction_count)
        printf("instruction %d has fetched\n", pipeline[0].pc);
}

void decode()
{
    if (pipeline[1].flush_flag == 1)
    {
        return;
    }
    pipeline[1].opcode = (pipeline[1].instruction >> 28) & 0xF;
    pipeline[1].R1 = (pipeline[1].instruction >> 23) & 0x1F;
    pipeline[1].R2 = (pipeline[1].instruction >> 18) & 0x1F;
    pipeline[1].R3 = (pipeline[1].instruction >> 13) & 0x1F;
    pipeline[1].shamt = pipeline[1].instruction & 0x1FFF;
    pipeline[1].immediate = pipeline[1].instruction & 0x3FFFF;
    pipeline[1].address = pipeline[1].instruction & 0xFFFFFFF;

    if (pipeline[1].immediate & (1 << 17)) // Check if the sign bit is set
    {
        pipeline[1].immediate |= 0xFFFC0000; // Sign extend the immediate value
    }

    pipeline[1].valueR1 = registers[pipeline[1].R1];
    pipeline[1].valueR2 = registers[pipeline[1].R2];
    pipeline[1].valueR3 = registers[pipeline[1].R3];

    printf("instruction %d has decoded\n", pipeline[1].pc);
}

void execute()
{
    if (pipeline[2].flush_flag == 1)
    {
        return;
    }
    switch (pipeline[2].opcode)
    {
    case 0:
        pipeline[2].valueR1 = pipeline[2].valueR2 + pipeline[2].valueR3;
        break;
    case 1:
        pipeline[2].valueR1 = pipeline[2].valueR2 - pipeline[2].valueR3;
        break;
    case 2:
        pipeline[2].valueR1 = pipeline[2].valueR2 * pipeline[2].valueR3;
        break;
    case 3:
        pipeline[2].valueR1 = pipeline[2].immediate;
        break;
    case 4:
        if (registers[pipeline[2].R1] == registers[pipeline[2].R2])
        {
            if (pipeline[2].immediate == 1)
            {
                pipeline[1].flush_flag = 1;
            }
            else if (pipeline[2].immediate > 1)
            {
                pipeline[0].flush_flag = 1;
                pipeline[1].flush_flag = 1;
                pc = pipeline[2].pc + pipeline[2].immediate;
            }
        }
        break;
    case 5:
        pipeline[2].valueR1 = pipeline[2].valueR2 & pipeline[2].valueR3;
        break;
    case 6:
        pipeline[2].valueR1 = pipeline[2].valueR2 ^ pipeline[2].immediate;
        break;
    case 7:
        // Treating JMP to the next pc address immediately and any other cases than the two correct ones below as useless...

        if (pipeline[2].address == pipeline[2].pc + 1)
        {
            pipeline[1].flush_flag = 1;
        }
        else if (pipeline[2].address != pipeline[2].pc)
        {
            pipeline[0].flush_flag = 1;
            pipeline[1].flush_flag = 1;
            pc = (pc & 0xF0000000) | pipeline[2].address;
        }
        break;
    case 8:
        pipeline[2].valueR1 = pipeline[2].valueR2 << pipeline[2].shamt;
        break;
    case 9:
        pipeline[2].valueR1 = pipeline[2].valueR2 >> pipeline[2].shamt;
        break;
    default:
        pipeline[2].ALU_result_memory_location =
            pipeline[2].valueR2 + pipeline[2].immediate;
        break;
    }
    printf("instruction %d has executed\n", pipeline[2].pc);
}

void memory_access()
{
    if (pipeline[3].flush_flag == 1)
    {
        return;
    }
    if (pipeline[3].opcode == 10)
    {
        pipeline[3].valueR1 = memory[pipeline[3].ALU_result_memory_location];
        printf("instruction %d has accessed memory ", pipeline[3].pc);
        printf("position %d \n",
               pipeline[3].ALU_result_memory_location);
    }
    else if (pipeline[3].opcode == 11)
    {
        memory[pipeline[3].ALU_result_memory_location] = registers[pipeline[3].R1];
        printf("instruction %d has accessed memory ", pipeline[3].pc);
        printf("position %d and changed it with: %d\n",
               pipeline[3].ALU_result_memory_location, pipeline[3].valueR1);
    }
    else
    {
        printf("instruction %d has passed by the memory access stage\n", pipeline[3].pc);
    }
}

void write_back()
{
    if (pipeline[4].flush_flag == 1)
    {
        return;
    }
    int valid_opcodes[12] = {1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0};

    if (valid_opcodes[pipeline[4].opcode])
    {
        if (pipeline[4].R1 == 0)
        {
            pipeline[4].valueR1 = 0;
            registers[pipeline[4].R1] = 0;
        }
        else
        {
            registers[pipeline[4].R1] = pipeline[4].valueR1;
        }
        printf("Instruction %d has changed Register %d with value %d\n", pipeline[4].pc, pipeline[4].R1, pipeline[4].valueR1);
    }
    else
    {
        printf("instruction %d has passed by the write back stage\n", pipeline[4].pc);
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

    // Close the file
    fclose(file);

    printf("Instruction count:%d\n", instruction_count);
    print_registers();

    while (1)
    {
        pass_pipeline(&pipeline[3], &pipeline[4]); // memory - write back
        if (pipeline[4].pc > instruction_count)
        {
            break;
        }

        printf("\n\n---------------------------------\n");
        printf("Clock cycle: %d\n", clock_cycle);
        pass_pipeline(&pipeline[2], &pipeline[3]); // execute - memory

        if (clock_cycle % 2 == 0)
            pass_pipeline(&pipeline[1], &pipeline[2]); // decode - execute

        if (clock_cycle % 2 == 0)
            pass_pipeline(&pipeline[0], &pipeline[1]); // fetch - decode

        if (clock_cycle % 2 == 1 && pc <= instruction_count)
        {
            fetch();
        }
        if (clock_cycle % 2 == 0 && pipeline[1].pc <= instruction_count && pipeline[1].flush_flag != 1)
        {
            printf("instruction %d has started decode\n", pipeline[0].pc);
        }
        if (clock_cycle > 2 && clock_cycle % 2 == 1 &&
            pipeline[1].pc <= instruction_count)
        {
            decode();
        }
        if (clock_cycle > 3 && clock_cycle % 2 == 0 &&
            pipeline[2].pc <= instruction_count && pipeline[2].flush_flag != 1)
        {
            printf("instruction %d has started execute\n", pipeline[2].pc);
        }

        if (clock_cycle > 4 && clock_cycle % 2 == 1 &&
            pipeline[2].pc <= instruction_count)
        {
            execute();
        }
        if (clock_cycle >= 6 && clock_cycle % 2 == 0 &&
            pipeline[3].pc <= instruction_count)
        {
            memory_access();
        }
        if (clock_cycle >= 7 && clock_cycle % 2 == 1 &&
            pipeline[4].pc <= instruction_count)
        {
            write_back();
        }

        clock_cycle++;
    }

    printf("\n\n---------------------------------\n");
    printf("Final registers:\n");
    print_registers();
    printf("\n\n");
    printf("PC: %d\n", pc);
    printf("\n\n");
    printf("Final memory:\n");
    print_memory();

    return 0;
}
