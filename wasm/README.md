molcpp WASM XYZ Subset
=======================

Current scope:
 - parseXYZFrame(text) -> Frame
 - Frame (read-only): blockNames(), variableNames(block), get(block,var) -> typed array, shape(block,var), metadata()
 - Metadata exposes: lattice, pbc, time, step, comment if present.

Build (example):
  emcmake cmake -B build-wasm -S molcpp/wasm
  cmake --build build-wasm -j

Outputs: build-wasm/dist/molcpp_xyz.js + molcpp_xyz.wasm

Import:
  import ModuleFactory from './molcpp_xyz.js';
  const Module = await ModuleFactory();
  const frame = Module.parseXYZFrame(xyzText);

Limitations / TODO:
 - No multi-frame trajectory yet
 - No zero-copy variable view yet (arrays are copied)
 - Metadata enumeration limited to known keys; extend Frame API later for full listing
 - Improve error type mapping (custom JS Error subclass)
