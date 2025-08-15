from .version import __version__  # noqa: F401

from ._c_lib import *

# Import the C++ bindings with xtensor support
try:
    from ._bindings import *
    from ._bindings import types, atom, spatial
    
    # Make commonly used types available at top level
    from ._bindings.types import Vec3, Mat3
    from ._bindings.atom import Atom
    
except ImportError as e:
    import warnings
    warnings.warn(f"Could not import molcpp C++ bindings: {e}")

# Import legacy bindings as fallback
try:
    from ._molcpp_bindings import *
    from ._molcpp_bindings import components, spatial as legacy_spatial
except ImportError:
    pass  # Legacy bindings not available