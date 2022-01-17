#define  _GNU_SOURCE
#include <regex.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>

#include "input_gfb.h"
#include "types_gfb.h"

regex_t date_regex;
regex_t number_regex;
regex_t sum_regex;

void compile_regexes(){
  assert(!regcomp(&date_regex, "[0-9]*.[0-9]*.[0-9]", 0)); // this is just initial validation
  assert(!regcomp(&number_regex, "[0-9]*", 0));
  assert(!regcomp(&sum_regex, "[0-9]*,[0-9][0-9]", 0));
}

void free_regexes(){
  regfree(&date_regex);
  regfree(&number_regex);
  regfree(&sum_regex);
}

void* safe_malloc(size_t size){
  void* res = malloc(size);
  if(res != NULL){
    return res;
  }
  else{
    printf("out of memory error\n");
    exit(1);
  }
}

date_t* read_date(){
  printf("Input the date: ");
  date_t* res = safe_malloc(sizeof(date_t));
  unsigned year;
  unsigned month;
  unsigned day;
  size_t len = 128;
  char* buffer = safe_malloc(128*sizeof(char));
  bool valid = false;
  int days_for_months[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  while(!valid){
    getline(&buffer, &len, stdin);
    if(regexec(&date_regex, buffer, 0, NULL, 0)){
      printf("Invalid date format! Expecting DD-MM-YYYY. Try again.\n");
      continue;
    }
    sscanf(buffer, "%u.%u.%u", &day, &month, &year);
    if(month > 12){
      printf("Invalid month number. Try again.\n");
      continue;
    }
    if(day > days_for_months[month-1]){
      printf("Invalid day number. Try again.\n");
      continue;
    }
    valid = true;
  }
  res->year = year;
  res->month = month;
  res->day = day;
  free(buffer);
  return res;
}

sum_t* read_sum(){
  printf("Input the sum: ");
  sum_t* res = safe_malloc(sizeof(sum_t));
  unsigned zl;
  unsigned gr;
  size_t len = 128;
  char* buffer = safe_malloc(128*sizeof(char));
  bool valid = false;
  while(!valid){
    getline(&buffer, &len, stdin);
    if(regexec(&sum_regex, buffer, 0, NULL, 0)){
      printf("Invalid sum format! Expecting zl,gr. Try again.\n");
      continue;
    }
    sscanf(buffer, "%u,%u", &zl, &gr);
    valid = true;
  }
  res->zl = zl;
  res->gr = gr;
  free(buffer);
  return res;
}

unsigned read_number(){
  unsigned res;
  size_t len = 128;
  char* buffer = safe_malloc(128*sizeof(char));
  bool valid = false;
  while(!valid){
    getline(&buffer, &len, stdin);
    if(regexec(&number_regex, buffer, 0, NULL, 0)){
      printf("Invalid format! Expecting a positive whole number. Try again.\n");
      continue;
    }
    sscanf(buffer, "%u", &res);
    valid = true;
  }
  free(buffer);
  return res;
}

unsigned read_procent(){
  printf("Input the percentage: ");
  return read_number();
}

int get_option_choice(){
  char* buffer = safe_malloc(128*sizeof(char));
  do{
    fgets(buffer, 128, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
  }
  while(strlen(buffer) > 1 || strlen(buffer) == 0);
  int res = buffer[0] - '0';
  free(buffer);
  return res;
}

bool ask_if_credit(){
  printf("Press 1 to add or modify a deposit, press 2 to add or modify a credit. ");
  while(true){
    switch(get_option_choice()){
      case 1:
        return false;
      case 2:
        return true;
      default:
        printf("Invalid choice, try once again: ");
        break;
    }
  }
}

char* read_client_id(){
  char* res = safe_malloc(128*sizeof(char));
  printf("Input the client identifier: ");
  fgets(res, 128, stdin);
  res[strcspn(res, "\n")] = '\0';
  while(getpwnam(res) == NULL){
    printf("Invalid client identifier. Try again: ");
    fgets(res, 128, stdin);
    res[strcspn(res, "\n")] = '\0';
  }
  return res;
}

