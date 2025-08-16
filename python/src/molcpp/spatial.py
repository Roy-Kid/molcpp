"""
Spatial analysis and geometry module for molcpp.

This module provides classes for:
- Box: Simulation boxes with periodic boundary conditions
- Region: Geometric regions for spatial filtering 
- Boundary: Boundary condition implementations
"""

from ._bindings.spatial import Box, BoxStyle, Region, Boundary  # noqa: F401

__all__ = ['Box', 'BoxStyle', 'Region', 'Boundary']
