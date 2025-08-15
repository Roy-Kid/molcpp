# Agent Instructions — xtensor Rules

Research-first rule
- Before writing or refactoring xtensor code, query the official xtensor docs via MCP/context7. If that is unavailable or insufficient, then consult headers or source code. Do not assume non-existent APIs (e.g., xt::all does not support an axis parameter).

Core Constraints
- Write NumPy-style vectorized code. No explicit for loops over array elements.
- Use broadcasting, views, and reductions.
- Return masks as xt::xarray<bool> with shape (n,) for coords of shape (n, 3).

Shapes & Views
- Positions are coords with shape (n, 3).
- Use xt::view/xt::row/xt::col, xt::newaxis for shaping.
- Call xt::eval only when needed to materialize expressions for API boundaries.

Comparisons & Masks
- Elementwise logical ops: &, |, ~
- Near-equality: xt::isclose / xt::allclose with rtol/atol
- Example (sphere, center c, radius r):
  - diff = coords - c
  - mask = xt::sum(diff * diff, {1}) <= r*r  // (n,)

Boolean Reductions by Axis
- xt::all/xt::any have no axis parameter.
- Use sum-count or reduce:
  - auto cnt = xt::sum(xt::cast<int>(mask2d), {1});                  // (n,)
  - auto all_true = (cnt == int(mask2d.shape(1)));                   // (n,)
  - or: auto all_true = xt::reduce([](auto a, auto b){ return a && b; }, mask2d, {1});

Linear Algebra
- Row-wise dot with vector v (shape (3,)): auto d = xt::sum(A * v, {1}); // A: (n,3) -> (n,)
- Squared norms: auto n2 = xt::sum(A * A, {1});
- Prefer elementwise mult + sum({1}) to avoid shape-mismatch with batched dot.

Random
- #include <xtensor/xrandom.hpp>
- Deterministic seed:
  - auto& engine = xt::random::get_default_random_engine();
  - xt::random::seed(42);
- Generate:
  - std::array<size_t, 2> shape = {n, 3};
  - auto U = xt::random::rand<double>(shape, -1.0, 1.0, engine);
  - auto Z = xt::random::randn<double>(shape, 0.0, 1.0, engine);

Performance
- Avoid unnecessary temporaries; insert xt::eval only when it reduces re-evaluation.
- Prefer views over copies; keep expressions vectorizable.

// EOF