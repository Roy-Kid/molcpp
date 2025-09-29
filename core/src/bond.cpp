#include "molcpp/core/bond.hpp"
#include "molcpp/core/atom.hpp"
#include "molcpp/core/ecs/components.hpp"

namespace molcpp {

Bond::Bond(const Atom& atom1, const Atom& atom2) {
    // Automatically create BondInfo component with atom IDs
    add_component<ecs::components::BondInfo>(atom1.get_id(), atom2.get_id());
}

} // namespace molcpp
