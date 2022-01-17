#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <time.h>

#include "pam_gfb.h"

unsigned long difference(unsigned long a, unsigned long b){
  if(a > b){
    return a - b;
  }
  else{
    return b - a;
  }
}

int system_time_auth(){
  static int called = 0;
  if(called == 1){
    printf("provide your system time:\n");
    unsigned long provided_time;
    scanf("%lu", &provided_time);
    unsigned long checked_time = time(NULL);
    if(difference(checked_time, provided_time) > 15){
      return -1;
    }
  }
  ++called;
  return 0;
}

int custom_conv(int num_msg, const struct pam_message **msgm, struct pam_response **response, void *appdata_ptr){
  if(misc_conv(num_msg, msgm, response, appdata_ptr) != 0){
    return PAM_AUTH_ERR;
  }
  if(system_time_auth() != 0){
    return PAM_AUTH_ERR;
  }
  return 0;
}
 
static struct pam_conv login_conv = {
  custom_conv,
  NULL
};

// code below based on the example from the labs
void authorize(){
  pam_handle_t* pamh = NULL;
  int retval;
  char *username = NULL;
 
  retval = pam_start("login", username, &login_conv, &pamh);
  if (pamh == NULL || retval != PAM_SUCCESS) {
    fprintf(stderr, "Error when starting: %d\n", retval);
    exit(1);
  }
 
  retval = pam_authenticate(pamh, 0);
  if (retval != PAM_SUCCESS) {
    fprintf(stderr, "Authentication failure.\n");
    exit(2);
  }
  else
    fprintf(stderr, "Authenticated.\n");
 
  pam_end(pamh, PAM_SUCCESS);
}
