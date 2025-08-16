from .version import __version__  # noqa: F401

# Import the C++ bindings with xtensor support
try:
    from ._bindings import *
    from ._bindings import atom, spatial, utils
    
except ImportError as e:
    import warnings
    warnings.warn(f"Could not import molcpp C++ bindings: {e}")