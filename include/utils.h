#ifndef UTILS_H
#define UTILS_H

namespace MolCpp
{

    // flag related functions

    inline bool has_flag(int flags, int flag) { return (flags & flag) == flag; }
    inline void set_flag(int &flags, int flag) { flags |= flag; }
    inline void unset_flag(int &flags, int flag) { flags &= ~flag; }
    inline void switch_flag(int &flags, int flag) { flags ^= flag; }

}

#endif
