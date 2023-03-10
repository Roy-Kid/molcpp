#include "utils.h"
#include "parsmart.h"

TEST(TestParsmart, test_AE_TRUE) {
    std::string s = "[*]";

    MolCpp::SmartsPattern sp;
    bool isValid = sp.Init(s);
    EXPECT_TRUE(isValid);

    MolCpp::Pattern* p = sp.GetPattern();

    EXPECT_EQ(p->acount, 1);
    EXPECT_EQ(p->bcount, 0);
    EXPECT_EQ(p->atom->expr->type, 6);

}

// TEST(TestParsmart, test_C1CCCCC1) {

//     std::string s = "[c1ccccc1]";

//     MolCpp::SmartsPattern sp;
//     bool isValid = sp.Init(s);
//     EXPECT_TRUE(isValid);

//     MolCpp::Pattern* p = sp.GetPattern();

//     EXPECT_EQ(p->acount, 6);
//     EXPECT_EQ(p->bcount, 6);
//     EXPECT_EQ(p->atom->expr->type, 6);    

// }