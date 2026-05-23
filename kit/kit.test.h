#ifndef TEST_H
#define TEST_H

int (test)(int expr, const char *text, const char *file, int line);
// extern int (*test)(int cond, const char *file, int line);
// extern int (*failures)(void);

// autotests yes/no. tests yes/no (disabled in retail builds)
#if TEST || TESTS
#define AUTOTEST   AUTORUN
#define test(cond) test(!!(cond), #cond, __FILE__, __LINE__)
#else
#define AUTOTEST   void MACRO(test_unused)(void)
#define test(cond) ifdef(KIT_RETAIL, 1, test(!!(cond), #cond, __FILE__, __LINE__))
#endif

#elif KIT_CODE
#pragma once

static unsigned tested_, failed_; //< THREAD?
static void summary_(void) {
    system(ifdef(KIT_WINDOWS,"type .tst 2> nul && del .tst","cat .tst 2> /dev/null && rm .tst"));
    printf("\n\t%u/%u tests passed (%u fails)\n", tested_-failed_, tested_, failed_);
}
int (test)(int cond, const char *text, const char *file, int line) {
    ONCE atexit(summary_);
    for( FILE *fp = cond ? NULL : fopen(".tst", "a+t"); fp; fclose(fp), fp = NULL)
        fprintf(fp, "\n\tFailed test(%s) (%s:%d)\n", text, file, line);
    return ++tested_, failed_ += !cond, cond;
}

AUTOTEST {
    test(1 < 2);
    test(__LINE__ > 0);
    test("ensure unit-tests are functional" && 1 == 1);
}

#endif
