#ifndef INPUT_GFB
#define INPUT_GFB

// module for input functions

#include <stdbool.h>

#include "types_gfb.h"

void compile_regexes();

void free_regexes();

date_t* read_date();

sum_t* read_sum();

unsigned read_number();

unsigned read_procent();

int get_option_choice();

bool ask_if_credit();

char* read_client_id();

void* safe_malloc(size_t size);

#endif
