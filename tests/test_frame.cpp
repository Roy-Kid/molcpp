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
    EXPECT_EQ(frame.get_natoms(), 3);
    EXPECT_EQ(frame.get_current_step(), current_step);
    EXPECT_TRUE(frame.get_xyz()==xt::xarray<double>({{1, 0, 0}, {0, 0, 0}, {0, 1, 0}}));

    auto topo = frame.get_topology();

    auto atoms = topo.get_atoms();
    EXPECT_EQ(atoms.size(), 3);

    auto bonds = topo.get_bonds();
    EXPECT_EQ(bonds.size(), 2);

}