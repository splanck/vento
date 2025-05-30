#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "file_ops.h"
#include "syntax.h"

int main(void){
    const char *pfile = "tmp_py.sh";
    FILE *fp = fopen(pfile, "w");
    fprintf(fp, "#!/usr/bin/env python\n");
    fclose(fp);
    assert(set_syntax_mode(pfile) == PYTHON_SYNTAX);
    unlink(pfile);

    const char *sfile = "tmp_sh.sh";
    fp = fopen(sfile, "w");
    fprintf(fp, "#!/bin/bash\n");
    fclose(fp);
    assert(set_syntax_mode(sfile) == SHELL_SYNTAX);
    unlink(sfile);
    return 0;
}
