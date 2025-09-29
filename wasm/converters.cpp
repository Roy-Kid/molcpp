#include "converters.hpp"
#include <string>

using namespace emscripten;

namespace molcpp_wasm {

val any_to_js(const std::any& a) {
    if (a.type() == typeid(double)) return val(std::any_cast<double>(a));
    if (a.type() == typeid(int)) return val(std::any_cast<int>(a));
    if (a.type() == typeid(size_t)) return val(static_cast<double>(std::any_cast<size_t>(a)));
    if (a.type() == typeid(std::string)) return val(std::any_cast<std::string>(a));
    if (a.type() == typeid(xt::xarray<double>)) return xarray_to_typed(std::any_cast<const xt::xarray<double>&>(a));
    if (a.type() == typeid(xt::xarray<bool>)) {
        const auto &arr = std::any_cast<const xt::xarray<bool>&>(a);
        std::vector<bool> buf(arr.size());
        for(size_t i=0;i<arr.size();++i) buf[i]=arr.data()[i];
        val jsArr = val::array();
        for(size_t i=0;i<buf.size();++i) jsArr.call<void>("push", val(buf[i]));
        return jsArr;
    }
    return val("<unsupported>");
}

} // namespace molcpp_wasm