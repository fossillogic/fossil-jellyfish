
// Generated Fossil Logic Test Runner
#include <fossil/pizza/framework.h>

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test List
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_EXPORT(cpp_jellyfish_tests);
FOSSIL_TEST_EXPORT(c_jellyfish_tests);

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Runner
// * * * * * * * * * * * * * * * * * * * * * * * *
int main(int argc, char **argv) {
    FOSSIL_TEST_START(argc, argv);
    FOSSIL_TEST_IMPORT(cpp_jellyfish_tests);
    FOSSIL_TEST_IMPORT(c_jellyfish_tests);

    FOSSIL_TEST_RUN();
    FOSSIL_TEST_SUMMARY();
    FOSSIL_TEST_END();
} // end of main
