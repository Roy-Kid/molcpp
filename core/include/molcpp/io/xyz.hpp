// Unified XYZ IO public API (previously in formats/xyz.hpp)
#pragma once

#include <string>
#include <string_view>
#include <expected>
#include <istream>
#include <ostream>
#include <memory>
#include "molcpp/core/frame.hpp"
#include "molcpp/io/errors.hpp"
#include "molcpp/io/xyz/xyz_types.hpp"
#include "molcpp/io/xyz/xyz_iter.hpp"

namespace molcpp {

std::expected<Frame, xyz_parse_error> parse_one_frame(std::istream& in);
Frame parse_one_frame_throw(std::istream& in);

class XYZFrameReader {
public:
	explicit XYZFrameReader(const std::string& path);
	~XYZFrameReader();
	std::expected<Frame, xyz_parse_error> read();
	static std::expected<Frame, xyz_parse_error> from_string(std::string_view text);
private:
	std::string path_;
	std::unique_ptr<std::ifstream> file_;
};

class XYZTrajectoryReader {
public:
	explicit XYZTrajectoryReader(const std::string& path);
	~XYZTrajectoryReader();
	std::expected<Frame, xyz_parse_error> read();
	std::expected<Frame, xyz_parse_error> read_step(std::size_t step);
	std::size_t tell() const;
	std::size_t steps() const;
	bool is_indexed() const;
	bool build_index();
	bool load_index();
	bool save_index();
	xyz_trajectory_iterator begin();
	xyz_trajectory_sentinel end();
	// Added convenience API (legacy-style expectations from tests)
	std::size_t n_frames() const { return steps(); }
	bool empty() const { return steps() == 0; }
	Frame read_frame(std::size_t i);
	std::vector<Frame> read_all();
	std::vector<Frame> read_range(std::size_t start, std::size_t end, std::size_t stride = 1);
	std::vector<Frame> read_frames(const std::vector<std::size_t>& indices);
private:
	std::string path_;
	std::unique_ptr<std::ifstream> file_;
	std::unique_ptr<trajectory_index> index_;
	std::size_t current_step_{};
	bool indexed_{};
};

class XYZFrameWriter {
public:
	static std::expected<void, std::string> write(const std::string& path, const Frame& frame);
	static std::expected<void, std::string> write(std::ostream& out, const Frame& frame);
};

class XYZTrajectoryWriter {
public:
	explicit XYZTrajectoryWriter(const std::string& path);
	~XYZTrajectoryWriter();
	bool is_open() const;
	std::expected<void, std::string> write(const Frame& frame);
	void close();
private:
	std::string path_;
	std::unique_ptr<std::ofstream> file_;
	std::unique_ptr<trajectory_index> index_;
	std::size_t frame_count_{};
};

} // namespace molcpp