#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <vector>
#include <sstream>
#include <any>
#include <unordered_map>
#include "molcpp/io/xyz.hpp"
#include "molcpp/core/frame.hpp"
#include "molcpp/core/block.hpp"
#include "molcpp/core/paramdict.hpp"
#include "converters.hpp"

using namespace emscripten;
using namespace molcpp;

namespace {

static val build_metadata_object(const Frame &f) {
    auto dict = f.metadata_dict();
    val obj = val::object();
    for (auto &k : dict.keys()) {
        if (auto ptr = dict.raw(k)) {
            obj.set(k, molcpp_wasm::any_to_js(*ptr));
        }
    }
    return obj;
}

// Extract xt::xarray<double> or <std::string> etc. into JS typed array / array
static val variable_to_js_const(const Frame &f, const std::string &block, const std::string &var) {
    // Try numeric double array
    try {
        const auto &arrd = f.template operator()<double>(block, var);
        std::vector<double> buf(arrd.size());
        std::copy(arrd.begin(), arrd.end(), buf.begin());
        val jsArr = val::global("Float64Array").new_(buf.size());
        // Fill via JS loop (simpler now; can optimize with HEAP copy later)
        for (size_t i=0;i<buf.size();++i) jsArr.set(i, buf[i]);
        return jsArr;
    } catch(...) {}
    // Try string array
    try {
        const auto &arrs = f.template operator()<std::string>(block, var);
        val jsArr = val::array();
        for (auto &s : arrs) jsArr.call<void>("push", val(s));
        return jsArr;
    } catch(...) {}
    return val::undefined();
}

static val variable_shape(const Frame &f, const std::string &block, const std::string &var) {
    try {
        const auto &arrd = f.template operator()<double>(block, var);
        val shape = val::array();
        for (auto s: arrd.shape()) shape.call<void>("push", val(double(s)));
        return shape;
    } catch(...) {}
    try {
        const auto &arrs = f.template operator()<std::string>(block, var);
        val shape = val::array();
        for (auto s: arrs.shape()) shape.call<void>("push", val(double(s)));
        return shape;
    } catch(...) {}
    return val::array();
}

// parse single frame from string
static Frame parseXYZFrame(const std::string &text) {
    std::istringstream iss(text);
    auto result = parse_one_frame(iss);
    if (!result) {
        const auto &err = result.error();
        // Throw JS exception through embind
        std::string msg = "XYZParseError line " + std::to_string(err.line) + ": " + err.message;
        throw std::runtime_error(msg);
    }
    return result.value();
}

// Frame writer wrapper for WASM
static std::string writeXYZFrame(const Frame& frame) {
    std::ostringstream oss;
    auto result = XYZFrameWriter::write(oss, frame);
    if (!result) {
        throw std::runtime_error("XYZWriteError: " + result.error());
    }
    return oss.str();
}

// XYZFrameReader wrapper for WASM
class XYZFrameReaderWrapper {
public:
    explicit XYZFrameReaderWrapper(const std::string& path) : reader_(path) {}
    
    Frame read() {
        auto result = reader_.read();
        if (!result) {
            const auto& err = result.error();
            std::string msg = "XYZParseError line " + std::to_string(err.line) + ": " + err.message;
            throw std::runtime_error(msg);
        }
        return result.value();
    }
    
    static Frame fromString(const std::string& text) {
        auto result = XYZFrameReader::from_string(text);
        if (!result) {
            const auto& err = result.error();
            std::string msg = "XYZParseError line " + std::to_string(err.line) + ": " + err.message;
            throw std::runtime_error(msg);
        }
        return result.value();
    }
    
private:
    XYZFrameReader reader_;
};

// XYZTrajectoryReader wrapper for WASM
class XYZTrajectoryReaderWrapper {
public:
    explicit XYZTrajectoryReaderWrapper(const std::string& path) : reader_(path) {}
    
    Frame read() {
        auto result = reader_.read();
        if (!result) {
            const auto& err = result.error();
            std::string msg = "XYZParseError line " + std::to_string(err.line) + ": " + err.message;
            throw std::runtime_error(msg);
        }
        return result.value();
    }
    
    Frame readStep(size_t step) {
        auto result = reader_.read_step(step);
        if (!result) {
            const auto& err = result.error();
            std::string msg = "XYZParseError line " + std::to_string(err.line) + ": " + err.message;
            throw std::runtime_error(msg);
        }
        return result.value();
    }
    
    Frame readFrame(size_t i) {
        return reader_.read_frame(i);
    }
    
    std::vector<Frame> readAll() {
        return reader_.read_all();
    }
    
    std::vector<Frame> readRange(size_t start, size_t end) {
        return reader_.read_range(start, end);
    }
    
    std::vector<Frame> readRangeWithStride(size_t start, size_t end, size_t stride) {
        return reader_.read_range(start, end, stride);
    }
    
    std::vector<Frame> readFrames(const std::vector<size_t>& indices) {
        return reader_.read_frames(indices);
    }
    
    size_t tell() const { return reader_.tell(); }
    size_t steps() const { return reader_.steps(); }
    size_t nFrames() const { return reader_.n_frames(); }
    bool empty() const { return reader_.empty(); }
    bool isIndexed() const { return reader_.is_indexed(); }
    bool buildIndex() { return reader_.build_index(); }
    bool loadIndex() { return reader_.load_index(); }
    bool saveIndex() { return reader_.save_index(); }
    
private:
    XYZTrajectoryReader reader_;
};

// XYZTrajectoryWriter wrapper for WASM
class XYZTrajectoryWriterWrapper {
public:
    explicit XYZTrajectoryWriterWrapper(const std::string& path) : writer_(path) {}
    
    bool isOpen() const { return writer_.is_open(); }
    
    void write(const Frame& frame) {
        auto result = writer_.write(frame);
        if (!result) {
            throw std::runtime_error("XYZWriteError: " + result.error());
        }
    }
    
    void close() { writer_.close(); }
    
private:
    XYZTrajectoryWriter writer_;
};

} // namespace

EMSCRIPTEN_BINDINGS(molcpp_xyz_module) {
    // Frame class
    class_<Frame>("Frame")
        .function("blockNames", &Frame::block_names)
        .function("variableNames", &Frame::variable_names)
        .function("metadata", &build_metadata_object)
        .function("get", &variable_to_js_const)
        .function("shape", &variable_shape)
        ;
    
    // XYZFrameReader class
    class_<XYZFrameReaderWrapper>("XYZFrameReader")
        .constructor<const std::string&>()
        .function("read", &XYZFrameReaderWrapper::read)
        .class_function("fromString", &XYZFrameReaderWrapper::fromString)
        ;
    
    // XYZTrajectoryReader class
    class_<XYZTrajectoryReaderWrapper>("XYZTrajectoryReader")
        .constructor<const std::string&>()
        .function("read", &XYZTrajectoryReaderWrapper::read)
        .function("readStep", &XYZTrajectoryReaderWrapper::readStep)
        .function("readFrame", &XYZTrajectoryReaderWrapper::readFrame)
        .function("readAll", &XYZTrajectoryReaderWrapper::readAll)
        .function("readRange", &XYZTrajectoryReaderWrapper::readRange)
        .function("readRangeWithStride", &XYZTrajectoryReaderWrapper::readRangeWithStride)
        .function("readFrames", &XYZTrajectoryReaderWrapper::readFrames)
        .function("tell", &XYZTrajectoryReaderWrapper::tell)
        .function("steps", &XYZTrajectoryReaderWrapper::steps)
        .function("nFrames", &XYZTrajectoryReaderWrapper::nFrames)
        .function("empty", &XYZTrajectoryReaderWrapper::empty)
        .function("isIndexed", &XYZTrajectoryReaderWrapper::isIndexed)
        .function("buildIndex", &XYZTrajectoryReaderWrapper::buildIndex)
        .function("loadIndex", &XYZTrajectoryReaderWrapper::loadIndex)
        .function("saveIndex", &XYZTrajectoryReaderWrapper::saveIndex)
        ;
    
    // XYZTrajectoryWriter class
    class_<XYZTrajectoryWriterWrapper>("XYZTrajectoryWriter")
        .constructor<const std::string&>()
        .function("isOpen", &XYZTrajectoryWriterWrapper::isOpen)
        .function("write", &XYZTrajectoryWriterWrapper::write)
        .function("close", &XYZTrajectoryWriterWrapper::close)
        ;
    
    // Vector bindings for JavaScript arrays
    register_vector<Frame>("VectorFrame");
    register_vector<size_t>("VectorSizeT");
    register_vector<std::string>("VectorString");
    
    // Standalone functions
    function("parseXYZFrame", &parseXYZFrame);
    function("writeXYZFrame", &writeXYZFrame);
}
