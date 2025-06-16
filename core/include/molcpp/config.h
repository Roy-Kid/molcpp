// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

// clang-format off
#ifndef MOLCPP_CONFIG_HPP
#define MOLCPP_CONFIG_HPP

/// An integer containing the major (x.0.0) version number
#define MOLCPP_VERSION_MAJOR 0
/// An integer containing the minor (0.y.0) version number
#define MOLCPP_VERSION_MINOR 0
/// An integer containing the patch (0.0.z) version number
#define MOLCPP_VERSION_PATCH 1
/// The full version of chemfiles ("x.y.z"), as a string
#define MOLCPP_VERSION "0.0.1"

#define MOLCPP_SIZEOF_VOID_P 8

/// Are we building code on Windows?
/* #undef MOLCPP_WINDOWS */

// Should we include GEMMI code?
/* #undef MOLCPP_DISABLE_GEMMI */

// clang-format on

#endif
