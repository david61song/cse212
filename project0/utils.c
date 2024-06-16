#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "dir_file.h"
#include "utils.h"

int open_file(char* fname, FILE** input) {
    if (access(fname, F_OK) == -1) {
        ERR_PRINT("The '%s' file does not exists\n", fname);
        return -1;
    }

    *input = fopen(fname, "r");
    if (input == NULL) {
        ERR_PRINT("Failed open '%s' file\n", fname);
        return -1;
    }

    return 1;
}

// This function splits the input string (const char* str) to tokens
// and put the tokens in token_list. The return value must be the number
// of tokens in the given input string.
int parse_str_to_list(const char* str, char** token_list) {
    char *token;
    char *copied_str;
    int str_size;
    char **curr_ptr;
    int total_token_size;

    total_token_size = 0;
    curr_ptr = token_list;
    str_size = strlen(str + 1);

    copied_str = malloc(sizeof(char) * (str_size + 1));
    strcpy(copied_str, str + 1);

    if (copied_str[str_size - 1] == '\n')
        copied_str[str_size - 1] = '\0';

    while((token = strsep(&copied_str, "/")) != NULL){
        *curr_ptr = malloc(sizeof(char) * (strlen(token) + 1));
	    strcpy(*curr_ptr, token);
        curr_ptr ++;
	    total_token_size ++;
    }

    curr_ptr = token_list;


    return total_token_size;
}

void free_token_list(char** token_list, int num_tokens) {
    for (int i = 0; i < num_tokens; i++) {
        free(token_list[i]);
    }

    free(token_list);
}

