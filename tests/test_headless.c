// test_headless.c
//
// bin/logomotive --headless's own runner (src/headless.c), driven
// through run_headless_script_with so both halves of what a caller sees
// can be checked: the exit code, and what was printed. The script's own
// output goes to capture_sink; the runner's diagnostics go to a file in
// build/, read back afterwards. Scripts are written to build/ too, same
// convention as tests/test_eval.c's file I/O tests (already gitignored,
// no dependency on /tmp being writable), and each test removes what it
// wrote.
//
// Run via `make test-headless`.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/headless.h"

static int failures = 0;
static const char *current_test = "";
static char captured_output[4096];
static char captured_err[4096];

static void capture_sink(LogoApp *app, const char *text) {
    (void)app;
    strncat(captured_output, text, sizeof(captured_output) - strlen(captured_output) - 1);
}

#define TEST(name) static void name(void)
#define RUN(name) do { current_test = #name; name(); } while (0)

#define CHECK(cond)                                                          \
    do {                                                                     \
        if (!(cond)) {                                                       \
            failures++;                                                      \
            printf("FAIL %s: %s (line %d)\n", current_test, #cond, __LINE__); \
        }                                                                    \
    } while (0)

#define SCRIPT_PATH "build/test_headless_script.logo"
#define ERR_PATH "build/test_headless_err.txt"

// Runs `path` headless, capturing the script's output and the runner's
// diagnostics, and returns the exit code the process would have had.
static int run_path(const char *path) {
    captured_output[0] = '\0';
    captured_err[0] = '\0';
    FILE *err = fopen(ERR_PATH, "w+");
    if (err == NULL) {
        failures++;
        printf("FAIL %s: could not open %s\n", current_test, ERR_PATH);
        return -1;
    }
    int code = run_headless_script_with("logomotive", path, capture_sink, err);
    rewind(err);
    size_t n = fread(captured_err, 1, sizeof(captured_err) - 1, err);
    captured_err[n] = '\0';
    fclose(err);
    remove(ERR_PATH);
    return code;
}

// Writes `source` as a script file, runs it, and removes it again.
static int run_script(const char *source) {
    g_file_set_contents(SCRIPT_PATH, source, -1, NULL);
    int code = run_path(SCRIPT_PATH);
    remove(SCRIPT_PATH);
    return code;
}

TEST(test_a_clean_script_exits_zero) {
    CHECK(run_script("PRINT 1 + 1") == 0);
    CHECK(strcmp(captured_output, "2\n") == 0);
    CHECK(captured_err[0] == '\0');
}

// The top level recovers from an uncaught THROW and runs the rest of
// the script, exactly as it does in the window, but the run as a whole
// has failed and the exit code says so.
TEST(test_an_uncaught_throw_exits_one_and_the_script_still_finishes) {
    CHECK(run_script("PRINT 1\nTHROW \"boom\nPRINT 2") == 1);
    CHECK(strcmp(captured_output, "1\nTHROW: no CATCH found for \"boom\n2\n") == 0);
}

TEST(test_a_caught_throw_exits_zero) {
    CHECK(run_script("CATCH \"boom [THROW \"boom]\nPRINT \"after") == 0);
    CHECK(strcmp(captured_output, "after\n") == 0);
}

TEST(test_an_uncaught_throw_inside_a_launched_agent_exits_one) {
    CHECK(run_script("TO worker\n  THROW \"boom\nEND\n"
                     "LAUNCH \"worker []\nAWAIT\nPRINT \"main-done") == 1);
    CHECK(strstr(captured_output, "THROW: no CATCH found for \"boom\n") != NULL);
    CHECK(strstr(captured_output, "main-done\n") != NULL);
}

// fopen succeeds on a directory on macOS and Linux; the runner used to
// read it as an empty script and exit 0.
TEST(test_a_directory_exits_one_with_a_message) {
    CHECK(run_path("build") == 1);
    CHECK(strstr(captured_err, "logomotive: ") == captured_err);
    CHECK(strstr(captured_err, "build") != NULL);
    CHECK(captured_output[0] == '\0');
}

TEST(test_a_missing_file_exits_one_with_a_message) {
    CHECK(run_path("build/test_headless_no_such_file.logo") == 1);
    CHECK(strstr(captured_err, "logomotive: ") == captured_err);
    CHECK(strstr(captured_err, "build/test_headless_no_such_file.logo") != NULL);
}

TEST(test_a_parse_error_exits_one_and_runs_nothing) {
    CHECK(run_script("PRINT 1\nnosuchproc") == 1);
    CHECK(strstr(captured_err, "unknown word: nosuchproc") != NULL);
    CHECK(captured_output[0] == '\0');
}

int main(void) {
    RUN(test_a_clean_script_exits_zero);
    RUN(test_an_uncaught_throw_exits_one_and_the_script_still_finishes);
    RUN(test_a_caught_throw_exits_zero);
    RUN(test_an_uncaught_throw_inside_a_launched_agent_exits_one);
    RUN(test_a_directory_exits_one_with_a_message);
    RUN(test_a_missing_file_exits_one_with_a_message);
    RUN(test_a_parse_error_exits_one_and_runs_nothing);

    if (failures == 0) {
        printf("All tests passed.\n");
        return 0;
    }
    printf("%d test(s) failed.\n", failures);
    return 1;
}
