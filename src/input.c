#include "input.h"
#include "editor.h"
#include <ncurses.h>
#include "clipboard.h"
#include "syntax.h"
#include "undo.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Input dispatch overview
 * -----------------------
 * All key presses and mouse events originate in the main loop inside
 * editor.c. That loop polls ncurses for input and then calls the
 * appropriate handler declared in input.h. Keyboard related routines
 * live in input_keyboard.c while mouse actions are handled in
 * input_mouse.c. Those files implement the actual editing commands and
 * cursor movement logic.
 *
 * This compilation unit is intentionally small. It exists primarily as
 * a placeholder for future dispatch helpers so it builds alongside the
 * rest of the project. No functions are currently defined here, but the
 * file is kept to maintain a consistent structure and to allow for
 * expansion.
 */

