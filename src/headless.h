#ifndef LOGO_HEADLESS_H
#define LOGO_HEADLESS_H

#include <stdio.h>

#include "logo_types.h"

// Runs `path` with no GTK window/event loop at all -- bin/logomotive
// --headless script.logo (see main.c's own consume_headless_flag).
// Returns a process exit code: 0 when the script runs to the end with
// no uncaught THROW, 1 if the file can't be read, fails to parse, lets
// a THROW reach the top level with no CATCH (the rest of the script
// still runs, as it does in the window, but the run counts as failed),
// or hits a suspension with no well-defined headless meaning (a bare
// AWAIT/YIELD with no enclosing LAUNCH). Diagnostics that the language
// itself treats as recoverable ("Recursion too deep, call ignored")
// don't affect the exit code. See headless.c's own file comment for
// the full suspend-handling rules.
int run_headless_script(const char *prog, const char *path);

// The same, with the script's own output sent to `sink` instead of
// stdout and the runner's diagnostics written to `err` instead of
// stderr -- so tests/test_headless.c can check both.
int run_headless_script_with(const char *prog, const char *path,
                              void (*sink)(LogoApp *app, const char *text), FILE *err);

#endif
