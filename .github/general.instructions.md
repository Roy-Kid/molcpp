# Agent Instructions — General Code Principles

Mission
- Keep behavior identical. Do not change numerical results or public APIs unless explicitly requested.
- Prefer refactor + tests improvement. If behavior must change, add/adjust tests first and justify.

Always
- Use CMake to build:
  - Configure: cmake -S . -B build
  - Build: cmake --build build
  - Enable tests: -DMOLCPP_BUILD_TESTS=ON
- Write vectorized code (no loops over array dimensions).
- Keep interfaces consistent with Region::isin signature:
  - xt::xarray<bool> isin(const xt::xarray<double>& coords) const
  - Return a boolean mask of shape (n,) for coords (n, 3). No mask() API elsewhere.
- Add unit tests for edge cases and numerical stability (empty inputs, large arrays, tolerance).
- Keep changes minimal, readable, and documented with brief Doxygen comments.

Never
- Do not introduce explicit loops over tensors for math that can be vectorized.
- Do not reintroduce a mask() method. Use isin only.
- Do not use non-existent xtensor APIs (e.g., xt::all(expr, axis)).
- Do not materialize expressions prematurely (avoid unnecessary xt::eval).

Code Style
- Use const, explicit, and noexcept where appropriate.
- Prefer auto for long template types but keep readability.
- Use std::array<size_t, N> for xtensor shapes.
- Use namespaces carefully; avoid using-directives in headers.

Testing
- Use Catch2 assertions:
  - Numeric tolerance: Approx(...)
  - Boolean arrays: compare elementwise or use reductions
- Seed all randomness deterministically in tests.

Review Checklist
- Behavior unchanged and tests updated.
- No loops over tensor dimensions; vectorization used.
- Shapes/broadcasting correct; no unintended materialization.
- CMake targets build on Linux; tests compile and run.

// EOF