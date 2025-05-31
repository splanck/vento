#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "file_ops.h"
#include "syntax.h"

int main(void){
    const char *file = "tmp_py_case.sh";
    FILE *fp = fopen(file, "w");
    fprintf(fp, "#!/usr/bin/Python\n");
    fclose(fp);
    assert(set_syntax_mode(file) == PYTHON_SYNTAX);
    unlink(file);
    return 0;
}
