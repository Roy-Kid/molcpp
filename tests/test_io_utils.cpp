#include <gtest/gtest.h>
#include "io_utils.h"

// Demonstrate some basic assertions.
TEST(TestIOUtils, BasicAssertions) {
    MolCpp::read("tests-data/pdb/hemo.pdb");
}