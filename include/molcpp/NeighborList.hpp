#pragma once

#include <xtensor/xtensor.hpp>
#include <cstddef>
#include <stdexcept>

namespace molcpp {

class NeighborList {
public:
    NeighborList() = default;

    NeighborList(xt::xtensor<int, 1> i, xt::xtensor<int, 1> j, xt::xtensor<float, 1> d)
        : m_i(std::move(i)), m_j(std::move(j)), m_d(std::move(d)) {
        if (m_i.size() != m_j.size() || m_i.size() != m_d.size()) {
            throw std::runtime_error("NeighborList arrays must have the same length");
        }
    }

    std::size_t size() const { return m_i.size(); }

    const xt::xtensor<int, 1>& i() const { return m_i; }
    const xt::xtensor<int, 1>& j() const { return m_j; }
    const xt::xtensor<float, 1>& distances() const { return m_d; }

private:
    xt::xtensor<int, 1> m_i;
    xt::xtensor<int, 1> m_j;
    xt::xtensor<float, 1> m_d;
};

} // namespace molcpp