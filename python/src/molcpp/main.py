from ._c_lib import _get_library

_lib = _get_library()
print(_lib.molcpp_add(1, 2))