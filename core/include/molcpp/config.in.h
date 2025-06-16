// Chemfiles, a modern library for chemistry file reading and writing
// Copyright (C) Guillaume Fraux and contributors -- BSD license

// clang-format off
#ifndef MOLCPP_CONFIG_HPP
#define MOLCPP_CONFIG_HPP

/// An integer containing the major (x.0.0) version number
#define MOLCPP_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
/// An integer containing the minor (0.y.0) version number
#define MOLCPP_VERSION_MINOR @PROJECT_VERSION_MINOR@
/// An integer containing the patch (0.0.z) version number
#define MOLCPP_VERSION_PATCH @PROJECT_VERSION_PATCH@
/// The full version of chemfiles ("x.y.z"), as a string
#define MOLCPP_VERSION "@PROJECT_VERSION@"

#define MOLCPP_SIZEOF_VOID_P @CMAKE_SIZEOF_VOID_P@

/// Are we building code on Windows?
#cmakedefine MOLCPP_WINDOWS

// Should we include GEMMI code?
#cmakedefine MOLCPP_DISABLE_GEMMI

// clang-format on

#endif