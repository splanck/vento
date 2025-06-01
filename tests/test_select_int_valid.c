#include <assert.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "config.h"
#include "editor_state.h"
int select_int(EditorContext *ctx, const char *prompt, int current, WINDOW *parent);

// stub create_dialog to provide valid numeric input
void create_dialog(EditorContext *ctx, const char *message, char *output, int max_input_len){
    (void)ctx;
    (void)message;
    strncpy(output, "42", max_input_len);
    if(max_input_len>0) output[max_input_len-1]='\0';
}

int main(void){
    EditorContext ctx = {0};
    int result = select_int(&ctx, "Num", 7, NULL);
    assert(result == 42);
    return 0;
}
