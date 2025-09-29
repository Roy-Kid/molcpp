# IO module sources
target_sources(molcpp_core PRIVATE
    utils.cpp
)

# Add XYZ implementation
add_subdirectory(xyz)
