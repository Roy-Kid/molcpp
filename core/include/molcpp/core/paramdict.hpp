#ifndef MOLCPP_CORE_PARAMDICT_HPP
#define MOLCPP_CORE_PARAMDICT_HPP

#include <string>
#include <vector>
#include <any>
#include <map>

namespace molcpp {

class ParamDict {
public:
    using storage_type = std::map<std::string, std::any>;
private:
    const storage_type* storage_ {nullptr};
public:
    ParamDict() = default;
    explicit ParamDict(const storage_type* ptr): storage_(ptr) {}

    bool empty() const { return !storage_ || storage_->empty(); }
    std::size_t size() const { return storage_? storage_->size():0; }
    bool has(const std::string& key) const { return storage_ && storage_->contains(key); }
    std::vector<std::string> keys() const {
        std::vector<std::string> ks; if(!storage_) return ks; ks.reserve(storage_->size());
        for (auto &p: *storage_) ks.push_back(p.first); return ks;
    }
    const std::any* raw(const std::string& key) const {
        if(!storage_) return nullptr; auto it = storage_->find(key); if(it==storage_->end()) return nullptr; return &it->second;
    }
    template<typename T>
    const T& get(const std::string& key) const { return std::any_cast<const T&>(*raw_checked(key)); }
private:
    const std::any* raw_checked(const std::string& key) const {
        auto ptr = raw(key); if(!ptr) throw std::out_of_range("ParamDict key not found: "+key); return ptr; }
};

} // namespace molcpp

#endif // MOLCPP_CORE_PARAMDICT_HPP