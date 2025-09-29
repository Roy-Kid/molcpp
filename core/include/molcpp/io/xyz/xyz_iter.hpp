#pragma once

#include <iterator>
#include <optional>
#include <memory>
#include "xyz_types.hpp"

// Forward declarations
namespace molcpp {
    class Frame;
    class XYZTrajectoryReader;
}

namespace molcpp {

// Forward declaration
class xyz_trajectory_sentinel;

/**
 * @brief Input iterator for XYZTrajectoryReader range-for support.
 * 
 * This iterator provides single-pass input semantics for trajectory reading.
 * It advances by calling read() on the underlying reader and yields Frame by value.
 */
class xyz_trajectory_iterator {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = molcpp::Frame;
    using difference_type = std::ptrdiff_t;
    using pointer = const molcpp::Frame*;
    using reference = const molcpp::Frame&;

    // Forward declaration - will be defined in implementation
    class impl;

    // Friend class for sentinel access
    friend class xyz_trajectory_sentinel;

private:
    std::unique_ptr<impl> impl_;

public:
    /**
     * @brief Default constructor (end iterator).
     */
    xyz_trajectory_iterator();

    /**
     * @brief Constructor for begin iterator.
     * @param reader_ptr Pointer to the trajectory reader
     */
    explicit xyz_trajectory_iterator(molcpp::XYZTrajectoryReader* reader_ptr);

    /**
     * @brief Copy constructor.
     */
    xyz_trajectory_iterator(const xyz_trajectory_iterator& other);

    /**
     * @brief Move constructor.
     */
    xyz_trajectory_iterator(xyz_trajectory_iterator&& other) noexcept;

    /**
     * @brief Destructor.
     */
    ~xyz_trajectory_iterator();

    /**
     * @brief Copy assignment operator.
     */
    xyz_trajectory_iterator& operator=(const xyz_trajectory_iterator& other);

    /**
     * @brief Move assignment operator.
     */
    xyz_trajectory_iterator& operator=(xyz_trajectory_iterator&& other) noexcept;

    /**
     * @brief Dereference operator.
     * @return Reference to current frame
     */
    reference operator*() const;

    /**
     * @brief Arrow operator.
     * @return Pointer to current frame
     */
    pointer operator->() const;

    /**
     * @brief Pre-increment operator.
     * @return Reference to this iterator
     */
    xyz_trajectory_iterator& operator++();

    /**
     * @brief Post-increment operator.
     * @return Copy of iterator before increment
     */
    xyz_trajectory_iterator operator++(int);

    /**
     * @brief Equality comparison.
     * @param other The other iterator to compare with
     * @return true if iterators are equal
     */
    bool operator==(const xyz_trajectory_iterator& other) const;

    /**
     * @brief Inequality comparison.
     * @param other The other iterator to compare with
     * @return true if iterators are not equal
     */
    bool operator!=(const xyz_trajectory_iterator& other) const;

    /**
     * @brief Equality comparison with sentinel.
     * @param sentinel The sentinel to compare with
     * @return true if iterator is at end
     */
    bool operator==(const xyz_trajectory_sentinel& sentinel) const;

    /**
     * @brief Inequality comparison with sentinel.
     * @param sentinel The sentinel to compare with
     * @return true if iterator is not at end
     */
    bool operator!=(const xyz_trajectory_sentinel& sentinel) const;
};

/**
 * @brief Sentinel type for xyz_trajectory_iterator.
 * 
 * This class represents the end of the trajectory range.
 * It can be compared with xyz_trajectory_iterator to detect end-of-range.
 */
class xyz_trajectory_sentinel {
public:
    /**
     * @brief Equality comparison with iterator.
     * @param iter The iterator to compare with
     * @return true if iterator is at end
     */
    bool operator==(const xyz_trajectory_iterator& iter) const;

    /**
     * @brief Inequality comparison with iterator.
     * @param iter The iterator to compare with
     * @return true if iterator is not at end
     */
    bool operator!=(const xyz_trajectory_iterator& iter) const;
};

} // namespace molcpp