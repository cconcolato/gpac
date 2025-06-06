#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_TESTS 1024

int tests_passed = 0;
int tests_failed = 0;

int checks_passed = 0;
int checks_failed = 0;

static struct TestCase {
  const char *name;
  void (*test_function)(void);
} tests[MAX_NUM_TESTS];

static unsigned int test_count = 0; // global number of registered tests

// Function to register a test case
int register_test(const char *name, void (*test_function)(void))
{
  if(test_count < sizeof(tests) / sizeof(tests[0])) {
    tests[test_count].name = name;
    tests[test_count].test_function = test_function;
    test_count++;
  } else {
    fprintf(stderr, "Too many tests registered.\n");
    return 1;
  }
  return 0;
}

int run_tests(int argc, char *argv[])
{
#ifdef GPAC_MEMORY_TRACKING
  gf_sys_init(GF_MemTrackerSimple, NULL);
#endif

  unsigned selected_tests = -1; // all
  for(int i = 1; i < argc; ++i) {
    if(!strcmp(argv[i], "--list") || !strcmp(argv[i], "-l")) {
      printf("List of tests:\n");
      for(unsigned i = 0; i < test_count; i++)
        printf("\ttest[%u]: \"%s\"... \n", i, tests[i].name);

      return EXIT_SUCCESS;
    } else if(!strcmp(argv[i], "--only")) {
      if(i + 1 >= argc) {
        fprintf(stderr, "Missing argument for --only\n");
        return EXIT_FAILURE;
      }
      selected_tests = atoi(argv[++i]);
      if(selected_tests != (unsigned)-1 && selected_tests >= test_count) {
        fprintf(stderr, "Test idx %u not found. Exiting.\n", selected_tests);
        return EXIT_FAILURE;
      }
      printf("Selected test: %s... \n", tests[selected_tests].name);
    }
  }

  int ret = EXIT_SUCCESS;
  for(unsigned i = 0; i < test_count; i++) {
    printf("Test %04d: %s... ", i, tests[i].name);
    fflush(stdout);

    if(selected_tests != (unsigned)-1 && selected_tests != i) {
      printf("Skipping\n");
      continue;
    }


    int prev_checks_failed = checks_failed;
    tests[i].test_function();
    if(checks_failed > prev_checks_failed) {
      printf("Failed\n");
      ret = EXIT_FAILURE;
      tests_failed++;
      if (checks_failed & 0x8000000) {
        checks_failed &= ~0x8000000;
        printf("Failure is fatal. Aborting test execution.\n");
        break;
      }
    } else {
      printf("Success\n");
      tests_passed++;
    }
  }

  printf("\n");
  printf("Tests passed: %d\n", tests_passed);
  printf("Tests failed: %d\n", tests_failed);
  printf("Checks passed: %d\n", checks_passed);
  printf("Checks failed: %d\n", checks_failed);

  gf_sys_close();

#ifdef GPAC_MEMORY_TRACKING
	if (gf_memory_size() || gf_file_handles_count() ) {
		gf_log_set_tool_level(GF_LOG_MEMORY, GF_LOG_INFO);
		gf_memory_print();
	}
#endif

  return ret;
}
