#ifndef MOLCPP_WASM_CONVERTERS_HPP
#define MOLCPP_WASM_CONVERTERS_HPP

#include <emscripten/val.h>
#include <any>
#include <xtensor/containers/xarray.hpp>

namespace molcpp_wasm {

emscripten::val any_to_js(const std::any& a);

template<typename T>
emscripten::val xarray_to_typed(const xt::xarray<T>& arr) {
    using namespace emscripten;
    std::vector<T> buf(arr.size());
    std::copy(arr.begin(), arr.end(), buf.begin());
    val ctor = val::global("Array");
    val jsArr = ctor.new_();
    for (size_t i=0;i<buf.size();++i) jsArr.call<void>("push", val(buf[i]));
    return jsArr;
}

} // namespace molcpp_wasm

#endif