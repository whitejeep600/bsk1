#define  _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <regex.h>

#include "pam_gfb.h"
#include "input_gfb.h"
#include "types_gfb.h"


// common functions

void print_success(){
  printf("Succeeded.\n");
}

bool date_earlier(date_t* a, date_t* b){
  if(a->year < b->year) return true;
  else if (a->year > b->year) return false;
  if (a->month < b->month) return true;
  else if (a->month > b->month) return false;
  if (a->day < b->day) return true;
  else if (a->day > b->day) return false;
}

date_t* get_date_from_line(char* line){
  unsigned year;
  unsigned month;
  unsigned day;
  sscanf(line+6, "%u.%u.%u", &day, &month, &year);
  date_t* res = malloc(sizeof(date_t));
  res->year = year;
  res->month = month;
  res->day = day;
  return res;
}

void print_date_to_file(FILE* f, date_t* date){
  fprintf(f, "%u.%u.%u", date->day, date->month, date->year);
}

void print_sum_to_file(FILE* f, sum_t* sum){
  fprintf(f, "%u,%u", sum->zl, sum->gr);
}

// functions related to contract listing (contracts := common name for deposits and credits)

bool user_owns(char* username, char* filepath){
  struct stat st;
  stat(filepath, &st);
  return strcmp(getpwuid(st.st_uid)->pw_name, username) == 0;
}

period_list* get_new_period_list(char* date_line, char* procent_line){
  period_list* res = safe_malloc(sizeof(period_list));
  res->date_line = date_line;
  res->procent_line = procent_line;
  res->next = NULL;
}

// extracts all the accounting periods from a given file.
period_list* get_period_list_from_file(FILE* file){
  size_t len = 128;
  char* temp = safe_malloc(len*sizeof(char));
  char* date_line;
  char* procent_line;
  period_list* list = NULL;
  period_list* temp_list;
  while(getline(&temp, &len, file) > 0){
    if(temp[0] == 'D'){
      date_line = safe_malloc(len*sizeof(char));
      strcpy(date_line, temp);
      if(getline(&temp, &len, file) > 0){
        procent_line = safe_malloc(len*sizeof(char));
        strcpy(procent_line, temp);
      }
      else procent_line = NULL;
      temp_list = get_new_period_list(date_line, procent_line);
      temp_list->next = list;
      list = temp_list;
    }
  }
  free(temp);
  return list;
}

bool list_date_later(period_list* a, period_list* b){
  date_t* from_a = get_date_from_line(a->date_line);
  date_t* from_b = get_date_from_line(b->date_line);
  bool res = date_earlier(from_b, from_a);
  free(from_a);
  free(from_b);
  return res;
}

// merges period lists in descending order of element dates.
period_list* merge_period_lists(period_list* a, period_list* b){
  period_list* dummy = safe_malloc(sizeof(period_list));
  period_list* last = dummy;
  while(a != NULL && b != NULL){
    if(list_date_later(a, b)){
      last->next = a;
      last = last->next;
      a = a->next;
    }
    else{
      last->next = b;
      last = last->next;
      b = b->next;
    }
  }
  if(a != NULL){
    last->next = a;
  }
  else{
    last->next = b;
  }
  last = dummy->next;
  free(dummy);
  return last;
}

void print_and_destroy(period_list* list){
  printf("%s", list->date_line);
  printf("%s", list->procent_line);
  free(list->date_line);
  free(list->procent_line);
  free(list);
}

void display_period_list(period_list* list){
  period_list* temp;
  while(list != NULL){
    temp = list;
    list = list->next;
    print_and_destroy(temp);
  }
}


// the task is not clear as to what exactly the format of the displayed files should be, in particular:
// should each contract be sorted and displayed separately? If not, what information to include for
// each accounting period, if most of them consist mostly of a date and percent change? How exactly
// is an accounting period defined? Does a new sum start a new accounting period? Isn't the example file
// impossible to obtain by contract modifications as described in the task, given that providing a date
// without a percent change is supposed to end a contract, which it doesn't in the file?
// I had doubts like this and eventually I decided to collect data from all the files, sort and
// display the dates and percent changes only.
void display_contracts(char* current_client_id){
  DIR *deposits_dir = opendir("/deposits");
  DIR *credits_dir = opendir("/credits");
  FILE* current_file;
  char path_buffer[128];
  struct dirent *file;
  period_list* plist = NULL;
  period_list* temp = NULL;
  while((file = readdir(deposits_dir)) != NULL){
    sprintf(path_buffer, "/deposits/%s", file->d_name);
    if(user_owns(current_client_id, path_buffer)){
      current_file = fopen(path_buffer, "r");
      temp = get_period_list_from_file(current_file);
      fclose(current_file);
      plist = merge_period_lists(plist, temp);
    }
  }
  
  while((file = readdir(credits_dir)) != NULL){
    sprintf(path_buffer, "/credits/%s", file->d_name);
    if(user_owns(current_client_id, path_buffer)){
      current_file = fopen(path_buffer, "r");
      temp = get_period_list_from_file(current_file);
      fclose(current_file);
      plist = merge_period_lists(plist, temp);
    }
  }
  display_period_list(plist);
  closedir(deposits_dir);
  closedir(credits_dir);
}

// functions related to contract adding

// gets the least unused contract number
unsigned get_new_contract_number(char* dirname){
  DIR *target_dir = opendir(dirname);
  struct dirent *file;
  char* filename;
  unsigned temp;
  unsigned max = 0;
  while((file = readdir(target_dir)) != NULL){
    filename = file->d_name;
    temp = strtoul(filename+1, NULL, 10);
    if(temp > max){
      max = temp;
    }
  }
  closedir(target_dir);
  return max+1;
}

char* get_path_to_deposit(unsigned deposit_no){
  char* file_path = safe_malloc(44*sizeof(char));
  strcpy(file_path, "/deposits/d");
  char number_buffer[32];
  sprintf(number_buffer, "%d\0", deposit_no);
  strcat(file_path, number_buffer);
  return file_path;
}

char* get_path_to_credit(unsigned credit_no){
  char* file_path = safe_malloc(44*sizeof(char));
  strcpy(file_path, "/credits/c");
  char number_buffer[32];
  sprintf(number_buffer, "%d\0", credit_no);
  strcat(file_path, number_buffer);
  return file_path;
}

void save_initial_contract_info_to_file(char* file_path, char* full_name, unsigned number,
                                        sum_t* sum, date_t* date, unsigned procent){
  FILE* new_file = fopen(file_path, "w");
  fprintf(new_file, "Name: ");
  fprintf(new_file, "%s", full_name);
  fprintf(new_file, "\nNumber: ");
  fprintf(new_file, "%d", number);
  fprintf(new_file, "\nSum: ");
  print_sum_to_file(new_file, sum);
  fprintf(new_file, "\nDate: ");
  print_date_to_file(new_file, date);
  fprintf(new_file, "\nProcent: ");
  fprintf(new_file, "%d\n", procent);
  fclose(new_file);
}


void add_contract(char* current_client_id){
  bool adding_credit = ask_if_credit();
  char* client_id;
  if(current_client_id == NULL){
    printf("You must still specify the target client.\n");
    client_id = read_client_id();
  }
  else{
    printf("Target client already specified.\n");
    client_id = current_client_id;
  }
  sum_t* sum = read_sum();
  date_t* date = read_date();
  unsigned procent = read_procent();
  char* full_name = NULL;
  full_name = getpwnam(client_id)->pw_gecos;
  char dir_path[11];
  if(adding_credit){
    strcpy(dir_path, "/credits/");
  }
  else{
    strcpy(dir_path, "/deposits/");
  }
  unsigned number = get_new_contract_number(dir_path);
  char* file_path = adding_credit? get_path_to_credit(number) : get_path_to_deposit(number);
  FILE* new_file = fopen(file_path, "w");
  fclose(new_file);
  char command[256];
  sprintf(command, "sudo chown %s %s", client_id, file_path);
  system(command);
  save_initial_contract_info_to_file(file_path, full_name, number, sum, date,procent);
  if(current_client_id != client_id){
    free(client_id);
  }
  free(sum);
  free(date);
  free(file_path);
}

// functions related to contract modifying

// extracts last accounting period end date from a given file.
date_t* get_date_from_file(FILE* file){
  size_t len = 128;
  char* last_line = safe_malloc(len*sizeof(char));
  char* penultimate_line = safe_malloc(len*sizeof(char));
  char* temp = safe_malloc(len*sizeof(char));
  getline(&penultimate_line, &len, file);
  getline(&last_line, &len, file);
  while(getline(&temp, &len, file) > 0){
    strcpy(penultimate_line, last_line);
    strcpy(last_line, temp);
  }
  if(penultimate_line[0] != 'D'){
    return false;
  }
  date_t* res = get_date_from_line(penultimate_line);
  free(last_line);
  free(penultimate_line);
  free(temp);
  return res;
}

// checks if the appended accounting period start is later than the start of the previous one.
bool date_valid_for_file(date_t* date, FILE* modified_file){
  date_t* file_date = get_date_from_file(modified_file);
  if(file_date == NULL){
    return false;
  }
  rewind(modified_file);
  bool res = date_earlier(file_date, date);
  free(file_date);
  return res;
}

// adds a new sum, date, and procent
void add_balance_change(FILE* modified_file){
  sum_t* sum = read_sum();
  date_t* date = read_date();
  unsigned procent = read_procent();
  if(date_valid_for_file(date, modified_file)){
    fseek(modified_file, 0, SEEK_END);
    fprintf(modified_file, "Sum: ");
    print_sum_to_file(modified_file, sum);
    fprintf(modified_file, "\nDate: ");
    print_date_to_file(modified_file, date);
    fprintf(modified_file, "\nProcent: ");
    fprintf(modified_file, "%d\n", procent);
  }
  else{
    printf("Invalid date or contract ended! Failed.\n");
  }
  free(sum);
  free(date);
}

// adds a new date and procent.
void add_percentage_change(FILE* modified_file){
  date_t* date = read_date();
  unsigned procent = read_procent();
  if(date_valid_for_file(date, modified_file)){
    fseek(modified_file, 0, SEEK_END);
    fprintf(modified_file, "Date: ");
    print_date_to_file(modified_file, date);
    fprintf(modified_file, "\nProcent: ");
    fprintf(modified_file, "%d\n", procent);
  }
  else{
    printf("Invalid date or contract ended! Failed.\n");
  }
  free(date);
}

// adds a new date.
void add_period_end(FILE* modified_file){
  date_t* date = read_date();
  if(date_valid_for_file(date, modified_file)){
    fseek(modified_file, 0, SEEK_END);
    fprintf(modified_file, "Date: ");
    print_date_to_file(modified_file, date);
    fprintf(modified_file, "\n");
    printf("printed date\n");
  }
  else{
    printf("Invalid date or contract ended! Failed.\n");
  }
  free(date);
}

void modify_contract(){
  bool adding_credit = ask_if_credit();
  printf("Input the number of the ");
  adding_credit? printf("credit") : printf("deposit");
  unsigned contract_number;
  printf(" that you want to modify. ");
  char* file_path;
  FILE* modified_file;
  do{
    contract_number = read_number();
    file_path = adding_credit? get_path_to_credit(contract_number) : get_path_to_deposit(contract_number);
    modified_file = fopen(file_path, "r+");
    if(modified_file == NULL){
      free(file_path);
      printf("Not found, try again:\n");
    }
  }
  while(modified_file == NULL);
  printf("Press 1 to add a balance change, 2 to add a new accounting period");
  printf(" and percentage, 3 to add end date of the last accounting period.\n");
  switch(get_option_choice()){
      case 1:
      add_balance_change(modified_file);
        break;
      case 2:
        add_percentage_change(modified_file);
        break;
      case 3:
        add_period_end(modified_file);
        break;
      default:
        break;
  }
  free(file_path);
  fclose(modified_file);
}

void run_tui(){
  char* current_client_id = NULL;
  while(1){
    printf("Choose one of the following options:\n");
    printf("1. Choose the client to perform operations for.\n");
    printf("2. Display deposits and credits for the chosen client.\n");
    printf("3. Add deposits and credits for the chosen client.\n");
    printf("4. Modify deposits and credits for the chosen client.\n");
    printf("5. Quit the Green Forest Bank application.\n");
    char c;
    switch(c = get_option_choice()){
      case 1:
        current_client_id = read_client_id();
        break;
      case 2:
        display_contracts(current_client_id);
        break;
      case 3:
        add_contract(current_client_id);
        break;
      case 4:
        modify_contract();
        break;
      case 5:
      if(current_client_id != NULL){
        free(current_client_id);
      }
        return;
      default:
        printf("Invalid option!\n");
        break;
    }
  }
}

int main(){
  printf("Welcome to the Green Forest Bank employee application.\n");
  authorize();
  compile_regexes();
  run_tui();
  free_regexes();
  return 0;
}

