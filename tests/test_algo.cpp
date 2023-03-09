#include "utils.h"
#include "algo.h"

TEST(TestAlgo, test_combination)
{
    std::vector<size_t> candidiate = {0, 1};
    auto combinations = MolCpp::combination(candidiate, 2);
    EXPECT_EQ(combinations.size()/2, 1);

    candidiate = {0, 1, 2};
    combinations = MolCpp::combination(candidiate, 2);
    EXPECT_EQ(combinations.size()/2, 3);
}

TEST(TestAlgo, test_find_in_container)
{
    std::vector<size_t> candidiate = {0, 1, 2};
    auto index = MolCpp::find_in_container(candidiate, 1);
    EXPECT_EQ(index, 1);
}
