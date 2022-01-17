#ifndef TYPES_GFB
#define TYPES_GFB

typedef struct sum_s{
  unsigned zl; // PLN whole units
  unsigned gr; // PLN cents
} sum_t;

typedef struct date_s{
  unsigned year;
  unsigned month;
  unsigned day;
} date_t;

typedef struct period_list_s{
  char* date_line;
  char* procent_line;
  struct period_list_s* next;
} period_list;

#endif
