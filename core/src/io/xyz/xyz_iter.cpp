#include "molcpp/io/xyz/xyz_iter.hpp"
#include "molcpp/io/xyz.hpp"
#include <optional>
#include <memory>

namespace molcpp {

class xyz_trajectory_iterator::impl {
public:
    molcpp::XYZTrajectoryReader* reader_ptr;
    std::optional<molcpp::Frame> current_frame;
    bool at_end;
    
    impl(molcpp::XYZTrajectoryReader* reader) : reader_ptr(reader), at_end(false) {
        if (reader_ptr) {
            std::expected<molcpp::Frame, xyz_parse_error> result = reader_ptr->read();
            if (result) {
                current_frame = std::move(result.value());
            } else {
                at_end = true;
            }
        } else {
            at_end = true;
        }
    }
    
    void advance() {
        if (!reader_ptr || at_end) {
            at_end = true;
            return;
        }
        
        std::expected<molcpp::Frame, xyz_parse_error> result = reader_ptr->read();
        if (result) {
            current_frame = std::move(result.value());
        } else {
            at_end = true;
        }
    }
};

xyz_trajectory_iterator::xyz_trajectory_iterator() 
    : impl_(std::make_unique<impl>(nullptr)) {
}

xyz_trajectory_iterator::xyz_trajectory_iterator(molcpp::XYZTrajectoryReader* reader_ptr) 
    : impl_(std::make_unique<impl>(reader_ptr)) {
}

xyz_trajectory_iterator::xyz_trajectory_iterator(const xyz_trajectory_iterator& other) 
    : impl_(std::make_unique<impl>(*other.impl_)) {
}

xyz_trajectory_iterator::xyz_trajectory_iterator(xyz_trajectory_iterator&& other) noexcept 
    : impl_(std::move(other.impl_)) {
}

xyz_trajectory_iterator::~xyz_trajectory_iterator() = default;

xyz_trajectory_iterator& xyz_trajectory_iterator::operator=(const xyz_trajectory_iterator& other) {
    if (this != &other) {
        impl_ = std::make_unique<impl>(*other.impl_);
    }
    return *this;
}

xyz_trajectory_iterator& xyz_trajectory_iterator::operator=(xyz_trajectory_iterator&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
    }
    return *this;
}

xyz_trajectory_iterator::reference xyz_trajectory_iterator::operator*() const {
    if (!impl_->current_frame) {
        throw std::runtime_error("Dereferencing end iterator");
    }
    return impl_->current_frame.value();
}

xyz_trajectory_iterator::pointer xyz_trajectory_iterator::operator->() const {
    if (!impl_->current_frame) {
        throw std::runtime_error("Dereferencing end iterator");
    }
    return &impl_->current_frame.value();
}

xyz_trajectory_iterator& xyz_trajectory_iterator::operator++() {
    impl_->advance();
    return *this;
}

xyz_trajectory_iterator xyz_trajectory_iterator::operator++(int) {
    xyz_trajectory_iterator temp = *this;
    impl_->advance();
    return temp;
}

bool xyz_trajectory_iterator::operator==(const xyz_trajectory_iterator& other) const {
    return impl_->at_end == other.impl_->at_end;
}

bool xyz_trajectory_iterator::operator!=(const xyz_trajectory_iterator& other) const {
    return !(*this == other);
}

bool xyz_trajectory_iterator::operator==(const xyz_trajectory_sentinel& sentinel) const {
    return impl_->at_end;
}

bool xyz_trajectory_iterator::operator!=(const xyz_trajectory_sentinel& sentinel) const {
    return !(*this == sentinel);
}

bool xyz_trajectory_sentinel::operator==(const xyz_trajectory_iterator& iter) const {
    return iter.impl_->at_end;
}

bool xyz_trajectory_sentinel::operator!=(const xyz_trajectory_iterator& iter) const {
    return !(*this == iter);
}

} // namespace molcpp