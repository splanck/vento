#include <assert.h>
#include <string.h>
#include <ncurses.h>
#include "ui.h"
#include "config.h"
#include "editor_state.h"
int select_int(EditorContext *ctx, const char *prompt, int current, WINDOW *parent);

// We'll run several cases: negative, non-digit, overflow
static const char *inputs[] = {"-1", "abc", "99999999999999999999999999"};
static int case_index = 0;

void create_dialog(EditorContext *ctx, const char *message, char *output, int max_input_len){
    (void)ctx;
    (void)message;
    strncpy(output, inputs[case_index], max_input_len);
    if(max_input_len>0) output[max_input_len-1]='\0';
}

int main(void){
    for(case_index=0; case_index<3; ++case_index){
        EditorContext ctx = {0};
        int result = select_int(&ctx, "Num", 5, NULL);
        assert(result == 5); // should return current
    }
    return 0;
}
