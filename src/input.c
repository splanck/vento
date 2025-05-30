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

/* Input dispatch logic is kept here.  Keyboard and mouse handlers
 * are implemented in input_keyboard.c and input_mouse.c.
 */

