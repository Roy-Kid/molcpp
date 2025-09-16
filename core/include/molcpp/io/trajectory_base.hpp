// Deprecated & removed: trajectory_base has been eliminated per refactor.
// This header now intentionally empty to avoid build breaks if any stale include remains.
// Please remove all includes of <molcpp/io/trajectory_base.hpp>.
// New design: format-specific providers (e.g., XYZTrajectoryReader) implement
// the FrameProvider concept directly (see <molcpp/io/formats/xyz/trajectory.hpp>).
#pragma once
// (intentionally empty)
