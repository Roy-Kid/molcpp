## molcpp_aabb

Standalone AABB neighbor finder implemented in C++ with xtensor (+xsimd), OpenMP, and xtensor-python bindings. No dependency on freud.

### Dependencies
- C++17 compiler
- Python3 with headers
- xtensor, xtensor-python (header-only)
- xsimd (header-only)
- OpenMP (optional)
- pybind11 (optional; found via CMake if available, otherwise headers through xtensor-python suffice)

### Build
```bash
mkdir -p build && cd build
cmake -DMOLCPP_BUILD_PYTHON=ON -DMOLCPP_ENABLE_OPENMP=ON -DMOLCPP_EXTERNAL_INCLUDE_DIRS="/path/to/xtensor;/path/to/xtensor-python;/path/to/xsimd;/path/to/pybind11/include" ..
cmake --build . --config Release
```

This produces `molcpp_aabb.*` (a Python extension module) in the build directory.

### Python usage
```python
import numpy as np
import molcpp_aabb as mol

box = mol.Box(10.0, 10.0, 10.0, True, True, True)
points = [mol.Vec3f(float(i), 0.0, 0.0) for i in range(5)]
query = mol.AABBNeighborQuery(box, points)
qi = np.array([[0.1, 0.0, 0.0], [4.9, 0.0, 0.0]], dtype=np.float32)
i, j, d = query.query(qi, 1.5, False)
print(i, j, d)
```

### Notes
- Uses OpenMP for parallelism and xsimd via xtensor for SIMD in leaf distance calculations.
- Implemented only the AABB kernel; no freud dependencies.
- All code is under the `molcpp` namespace.