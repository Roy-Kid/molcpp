#include "utils.h"
#include "frame.h"

TEST(TestFrame, test_construct_from_chemfiles_frame) {
    
    auto chflFrame = chemfiles::Frame(chemfiles::UnitCell({10, 10, 10}));
    int current_step = 42;
    chflFrame.set_step(current_step);
    chflFrame.add_atom(chemfiles::Atom("H"), {1, 0, 0});
    chflFrame.add_atom(chemfiles::Atom("O"), {0, 0, 0});
    chflFrame.add_atom(chemfiles::Atom("H"), {0, 1, 0});
    chflFrame.add_bond(0, 1);
    chflFrame.add_bond(2, 1);    

    MolCpp::Frame frame(chflFrame);
    EXPECT_EQ(frame.GetNatoms(), 3);
    EXPECT_EQ(frame.GetCurrentStep(), current_step);
    EXPECT_TRUE(frame.GetXYZ()==xt::xarray<double>({{1, 0, 0}, {0, 0, 0}, {0, 1, 0}}));

}