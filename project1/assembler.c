#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

/*
 * For debug option. If you want to debug, set 1.
 * If not, set 0.
 */
#define DEBUG 1






#define MAX_SYMBOL_TABLE_SIZE   1024
#define MEM_TEXT_START          0x00400000
#define MEM_DATA_START          0x10000000
#define BYTES_PER_WORD          4
#define INST_LIST_LEN           22



/******************************************************
 * Structure Declaration
 *******************************************************/

typedef struct inst_struct {
    char *name;
    char *op;
    char type;
    char *funct;
} inst_t;

typedef struct symbol_struct {
    char name[32];
    uint32_t address;
} symbol_t;

enum section {
    DATA = 0,
    TEXT,
    MAX_SIZE
};

/******************************************************
 * Global Variable Declaration
 *******************************************************/

inst_t inst_list[INST_LIST_LEN] = {       //  idx
    {"add",     "000000", 'R', "100000"}, //    0
    {"sub",     "000000", 'R', "100010"}, //    1
    {"addiu",   "001001", 'I', ""},       //    2
    {"addu",    "000000", 'R', "100001"}, //    3
    {"and",     "000000", 'R', "100100"}, //    4
    {"andi",    "001100", 'I', ""},       //    5
    {"beq",     "000100", 'I', ""},       //    6
    {"bne",     "000101", 'I', ""},       //    7
    {"j",       "000010", 'J', ""},       //    8
    {"jal",     "000011", 'J', ""},       //    9
    {"jr",      "000000", 'R', "001000"}, //   10
    {"lui",     "001111", 'I', ""},       //   11
    {"lw",      "100011", 'I', ""},       //   12
    {"nor",     "000000", 'R', "100111"}, //   13
    {"or",      "000000", 'R', "100101"}, //   14
    {"ori",     "001101", 'I', ""},       //   15
    {"sltiu",   "001011", 'I', ""},       //   16
    {"sltu",    "000000", 'R', "101011"}, //   17
    {"sll",     "000000", 'R', "000000"}, //   18
    {"srl",     "000000", 'R', "000010"}, //   19
    {"sw",      "101011", 'I', ""},       //   20
    {"subu",    "000000", 'R', "100011"}  //   21
};

symbol_t SYMBOL_TABLE[MAX_SYMBOL_TABLE_SIZE]; // Global Symbol Table

uint32_t symbol_table_cur_index = 0; // For indexing of symbol table

/* Temporary file stream pointers */
FILE *data_seg;
FILE *text_seg;

/* Size of each section */
uint32_t data_section_size = 0;
uint32_t text_section_size = 0;

/******************************************************
 * Function Declaration
 *******************************************************/

 uint32_t find_label_address(char *var){
    // Find the address of the variable in the symbol table
    for (int i = 0; i < symbol_table_cur_index; i++) {
        if (strcmp(SYMBOL_TABLE[i].name, var) == 0) {
            return SYMBOL_TABLE[i].address;
        }
    }
    return 0;
 }

 int is_la_instruction(const char* line) {
    // Skip leading whitespace
    while (*line && isspace(*line)) {
        line++;
    }

    // Check if the line starts with "la"
    if (strncmp(line, "la", 2) == 0) {
        // Ensure "la" is followed by a space or tab
        if (isspace(line[2])) {
            return 1;
        }
        // Check if it's the end of the line (in case of no arguments)
        if (line[2] == '\0' || line[2] == '#') {
            return 1;
        }
    }

    return 0;
}      


/* Change file extension from ".s" to ".o" */
char* change_file_ext(char *str) {
    char *dot = strrchr(str, '.');

    if (!dot || dot == str || (strcmp(dot, ".s") != 0))
        return NULL;

    str[strlen(str) - 1] = 'o';
    return "";
}

/* Add symbol to global symbol table */
void symbol_table_add_entry(symbol_t symbol)
{
    SYMBOL_TABLE[symbol_table_cur_index++] = symbol;
#if DEBUG
    printf("---Added symbol table entry : ---\n");
    printf("%s: 0x%08x\n", symbol.name, symbol.address);
    printf("---------------------\n");
#endif
}

/* Convert integer number to binary string */
char* num_to_bits(unsigned int num, int len)
{
    char* bits = (char *) malloc(len+1);
    int idx = len-1, i;
    while (num > 0 && idx >= 0) {
        if (num % 2 == 1) {
            bits[idx--] = '1';
        } else {
            bits[idx--] = '0';
        }
        num /= 2;
    }
    for (i = idx; i >= 0; i--){
        bits[i] = '0';
    }
    bits[len] = '\0';
    return bits;
}

/* Record .text section to output file */
void record_text_section(FILE *output)
{
    uint32_t cur_addr = MEM_TEXT_START;
    char line[1024];

    /* Point to text_seg stream */
    rewind(text_seg);

    /* Print .text section */
    while (fgets(line, 1024, text_seg) != NULL) {
        char inst[0x1000] = {0};
        char op[32] = {0};
        char label[32] = {0};
        char type = '0';
        int i, idx = 0;
        int rs, rt, rd, imm, shamt;
        int addr;

        rs = rt = rd = imm = shamt = addr = 0;
#if DEBUG
        printf("0x%08x: ", cur_addr);
        printf("Curr line-->%s", line);
#endif
        /* Find the instruction type that matches the line */

        switch (type) {
            case 'R':
                /* blank */
#if DEBUG
                printf("op:%s rs:$%d rt:$%d rd:$%d shamt:%d funct:%s\n",
                        op, rs, rt, rd, shamt, inst_list[idx].funct);
#endif
                break;

            case 'I':
                /* blank */
#if DEBUG
                printf("op:%s rs:$%d rt:$%d imm:0x%x\n",
                        op, rs, rt, imm);
#endif
                break;

            case 'J':
                /* blank */
#if DEBUG
                printf("op:%s addr:%i\n", op, addr);
#endif
                break;

            default:
                break;
        }
        fprintf(output, "\n");

        cur_addr += BYTES_PER_WORD;
    }
}

/* Record .data section to output file */
void record_data_section(FILE *output)
{
    uint32_t cur_addr = MEM_DATA_START;
    char line[1024];

    /* Point to data segment stream */
    rewind(data_seg);

    /* Print .data section */
    while (fgets(line, 1024, data_seg) != NULL) {
        /* blank */
#if DEBUG
        printf("0x%08x: ", cur_addr);
        printf("%s", line);
#endif
        cur_addr += BYTES_PER_WORD;
    }
}

/* Fill the blanks */
void make_binary_file(FILE *output)
{
#if DEBUG
    char line[1024] = {0};
    rewind(text_seg);
    /* Print line of text segment */
    while (fgets(line, 1024, text_seg) != NULL) {
        printf("%s",line);
    }
    printf("text section size: %d, data section size: %d\n",
            text_section_size, data_section_size);
#endif

    /* Print text section size and data section size */
    fputs(num_to_bits(text_section_size, 32), output);
    fputs("\n", output);
    fputs(num_to_bits(data_section_size, 32), output);
    fputs("\n", output);

    /* Print .text section */
    record_text_section(output);

    /* Print .data section */
    record_data_section(output);
}

void make_symbol_table(FILE *input)
{
    char line[1024] = {0};
    uint32_t address = 0;
    symbol_t data_symbol;
    char *label;
    enum section cur_section = MAX_SIZE;
    char* saveptr;
    char *inst, *reg, *var;
    char lui_inst[64];
    int32_t var_address = 0;
    char *delim = ", \t\n";

    /* Read each section and put the stream. Reading stops if meet newline, or end of file. */
    while (fgets(line, 1024, input) != NULL) {
        char *temp;
        char _line[1024] = {0}; /* tmp read line*/
        strcpy(_line, line);
        temp = strtok(_line, "\t\n");

        /* Check section type */
        if (!strcmp(temp, ".data")) {
            data_seg = tmpfile();
            cur_section = DATA;
            continue;
        }
        else if (!strcmp(temp, ".text")) { /* temp -> first occured word */
            text_seg = tmpfile();
            cur_section = TEXT;
            address = 0;
            continue;
        }
        /* Put the line into each segment stream */
        /* DATA segement */
        if (cur_section == DATA) {
            fputs(line, data_seg);
            if (strstr(temp, ":") != NULL){
                label = strtok(temp, ":");
                data_symbol.address = address + (uint32_t) MEM_DATA_START;
                strcpy(data_symbol.name, label);
                symbol_table_add_entry(data_symbol);

                if (strstr(line, ".word") != NULL){
                    address += BYTES_PER_WORD;
                    data_section_size += 4;
                }
            }
            if (strcmp(temp, ".word") == 0){
                address += BYTES_PER_WORD;
                data_section_size += 4;
            }
        } 
        /* TEXT segment */
        else if (cur_section == TEXT) {
            /* check label */
            if (strstr(temp, ":") != NULL){
                fputs(line, text_seg);
                label = strtok(temp, ":");
                data_symbol.address = address + (uint32_t) MEM_TEXT_START;
                strcpy(data_symbol.name, label);
                symbol_table_add_entry(data_symbol);
            }
            /* not label, instructions */
            else {
                /* Not la instruction, just puts line*/
                if (is_la_instruction(line) == 0){
                    fputs(line, text_seg);
                    address += BYTES_PER_WORD;
                    text_section_size += 4;
                    continue;
                }
                /* la instruction parsing */
                inst = strtok_r(line, delim, &saveptr);
                if (inst != NULL) {
                    reg = strtok_r(NULL, delim, &saveptr);
                    var = strtok_r(NULL, delim, &saveptr);
                    if (reg != NULL && var != NULL) {
                        var_address = find_label_address(var);
                        if (var_address != 0) {
                            uint16_t upper = (var_address >> 16) & 0xFFFF;
                            uint16_t lower = var_address & 0xFFFF;
                            // Generate lui instruction
                            sprintf(lui_inst, "\tlui\t%s, 0x%04x\n", reg, upper);
                            fputs(lui_inst, text_seg);
                            address += BYTES_PER_WORD;
                            text_section_size += 4;
                        // Generate ori instruction if lower 16 bits are non-zero
                        if (lower != 0) {
                            char ori_inst[64];
                            sprintf(ori_inst, "\tori\t%s, %s, 0x%04x\n", reg, reg, lower);
                            fputs(ori_inst, text_seg);
                            address += BYTES_PER_WORD;
                            text_section_size += 4;
                            }
                        } 
                    }
                }
            }
        }
    }
}


/******************************************************
 * Function: main
 *
 * Parameters:
 *  int
 *      argc: the number of argument
 *  char*
 *      argv[]: array of a sting argument
 *
 * Return:
 *  return success exit value
 *
 * Info:
 *  The typical main function in C language.
 *  It reads system arguments from terminal (or commands)
 *  and parse an assembly file(*.s).
 *  Then, it converts a certain instruction into
 *  object code which is basically binary code.
 *
 *******************************************************/

int main(int argc, char* argv[])
{
    FILE *input, *output;
    char *filename;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <*.s>\n", argv[0]);
        fprintf(stderr, "Example: %s sample_input/example?.s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Read the input file */
    input = fopen(argv[1], "r");
    if (input == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    /* Create the output file (*.o) */
    filename = strdup(argv[1]); // strdup() is not a standard C library but fairy used a lot.
    if(change_file_ext(filename) == NULL) {
        fprintf(stderr, "'%s' file is not an assembly file.\n", filename);
        exit(EXIT_FAILURE);
    }

    output = fopen(filename, "w");
    if (output == NULL) {
        perror("ERROR");
        exit(EXIT_FAILURE);
    }

    /******************************************************
     *  Let's complete the below functions!
     *
     *  make_symbol_table(FILE *input)
     *  make_binary_file(FILE *output)
     *  ├── record_text_section(FILE *output)
     *  └── record_data_section(FILE *output)
     *
     *******************************************************/
    make_symbol_table(input);
    make_binary_file(output);

    fclose(input);
    fclose(output);

    return 0;
}

