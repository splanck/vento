#include <assert.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "config.h"
int select_int(const char *prompt, int current, WINDOW *parent);

// stub create_dialog to provide valid numeric input
void create_dialog(const char *message, char *output, int max_input_len){
    (void)message;
    strncpy(output, "42", max_input_len);
    if(max_input_len>0) output[max_input_len-1]='\0';
}

int main(void){
    int result = select_int("Num", 7, NULL);
    assert(result == 42);
    return 0;
}
