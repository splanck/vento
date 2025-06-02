#ifndef MINUNIT_H
#define MINUNIT_H
#include <stdio.h>
extern int tests_run;
#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *msg = test(); tests_run++; if (msg) return msg; } while (0)
#endif
