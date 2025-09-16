#include <molcpp/io/formats/xyz/parser.hpp>
#include <molcpp/core/element.hpp>
#include <molcpp/types.hpp>
#include <algorithm>
#include <cctype>
#include <regex>
#include <unordered_map>
#include <optional>
#include <sstream>

namespace molcpp::io::xyz_parser {

namespace {

inline std::string trim_copy(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j-1]))) j--;
    return s.substr(i, j - i);
}

inline void to_lower_inplace(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                  [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
}

struct ExtProperty { 
    enum Kind { BOOL, DOUBLE, VECTOR3, STRING }; 
    std::string name; 
    Kind kind; 
};

std::vector<ExtProperty> parse_properties_list(const std::string& input) {
    std::vector<ExtProperty> out;
    if (input.rfind("species:S:1:pos:R:3", 0) != 0) {
        return out;
    }
    if (input.size() <= 20) return out;
    std::string rest = input.substr(20);
    if (!rest.empty() && rest[0] == ':') rest.erase(0, 1);
    if (rest.empty()) return out;

    std::vector<std::string> fields;
    size_t start = 0;
    for (size_t pos = 0; pos <= rest.size(); ++pos) {
        if (pos == rest.size() || rest[pos] == ':') {
            fields.emplace_back(rest.substr(start, pos - start));
            start = pos + 1;
        }
    }
    if (fields.size() % 3 != 0) return {};
    
    const size_t count = fields.size() / 3;
    out.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        std::string name = fields[3*i];
        std::string type = fields[3*i+1];
        size_t repeat = 0;
        try { 
            repeat = static_cast<size_t>(std::stoul(fields[3*i+2])); 
        } catch (...) { 
            continue; 
        }
        
        ExtProperty::Kind kind;
        if (type == "R" || type == "I") kind = ExtProperty::DOUBLE;
        else if (type == "S") kind = ExtProperty::STRING;
        else if (type == "L") kind = ExtProperty::BOOL;
        else continue;

        if (repeat == 3 && kind == ExtProperty::DOUBLE) {
            out.push_back({name, ExtProperty::VECTOR3});
            continue;
        }
        if (repeat == 0) continue;
        if (repeat == 1) {
            out.push_back({name, kind});
        } else {
            for (size_t j = 0; j < repeat; ++j) {
                out.push_back({name + "_" + std::to_string(j), kind});
            }
        }
    }
    return out;
}

std::unordered_map<std::string, std::string> parse_extended_kv(const std::string& line) {
    std::unordered_map<std::string, std::string> kv;
    const char* cur = line.c_str();
    const char* end = cur + line.size();
    
    auto skip_ws = [&]() { 
        while (cur < end && std::isspace(static_cast<unsigned char>(*cur))) cur++; 
    };
    
    auto next_token = [&]() -> std::string {
        if (cur >= end) return {};
        std::string result;
        if (*cur == '"' || *cur == '\'') {
            char quote = *cur++;
            while (cur < end && *cur != quote) { 
                result.push_back(*cur++); 
            }
            if (cur < end && *cur == quote) cur++;
        } else {
            while (cur < end && !std::isspace(static_cast<unsigned char>(*cur)) && *cur != '=') {
                result.push_back(*cur++);
            }
        }
        return result;
    };

    while (cur < end) {
        skip_ws();
        auto name = next_token();
        skip_ws();
        if (name.empty()) break;
        if (cur < end && *cur == '=') cur++;
        skip_ws();
        auto value = next_token();
        kv.emplace(std::move(name), std::move(value));
        skip_ws();
    }
    return kv;
}

void set_typed_metadata(Frame& frame, const std::string& key, const std::string& value) {
    std::string lower = value; 
    to_lower_inplace(lower);
    
    if (lower == "t" || lower == "true") { 
        frame.set_metadata(key, true); 
        return; 
    }
    if (lower == "f" || lower == "false") { 
        frame.set_metadata(key, false); 
        return; 
    }
    
    // Try to parse as vector3
    {
        std::istringstream iss(value);
        double a, b, c; 
        if (iss >> a >> b >> c && iss.rdbuf()->in_avail() == 0) {
            xt::xarray<float> v = xt::zeros<float>({3});
            v(0) = static_cast<float>(a); 
            v(1) = static_cast<float>(b); 
            v(2) = static_cast<float>(c);
            frame.set_metadata(key, v);
            return;
        }
    }
    
    // Try to parse as double
    try {
        double d = std::stod(value);
        frame.set_metadata(key, static_cast<float>(d));
        return;
    } catch (...) {}
    
    // Default to string
    frame.set_metadata(key, value);
}

} // anonymous namespace

Frame parse_xyz_frame(const std::vector<std::string>& lines) {
    if (lines.size() < 2) {
        throw std::runtime_error("XYZ frame requires at least two lines");
    }
    
    int n_atoms = 0;
    try { 
        n_atoms = std::stoi(lines[0]); 
    } catch (...) {
        throw std::runtime_error("First line must be an integer atom count");
    }
    
    if (static_cast<int>(lines.size()) != 2 + n_atoms) {
        throw std::runtime_error("Invalid number of lines for XYZ frame");
    }

    Frame frame;
    frame.set_metadata("comment", lines[1]);

    // Extended comment parsing
    auto kv = parse_extended_kv(lines[1]);
    auto it_props = kv.find("Properties");
    std::vector<ExtProperty> props;
    if (it_props != kv.end()) {
        props = parse_properties_list(it_props->second);
    }
    
    // Parse lattice if present
    if (auto it_lat = kv.find("Lattice"); it_lat != kv.end()) {
        std::istringstream iss(it_lat->second);
        double v[9];
        for (int i = 0; i < 9; ++i) iss >> v[i];
        xt::xarray<float> mat = xt::zeros<float>({3, 3});
        mat(0,0)=static_cast<float>(v[0]); mat(0,1)=static_cast<float>(v[1]); mat(0,2)=static_cast<float>(v[2]);
        mat(1,0)=static_cast<float>(v[3]); mat(1,1)=static_cast<float>(v[4]); mat(1,2)=static_cast<float>(v[5]);
        mat(2,0)=static_cast<float>(v[6]); mat(2,1)=static_cast<float>(v[7]); mat(2,2)=static_cast<float>(v[8]);
        molcpp::Block cell;
        cell.set("matrix", mat);
        frame.set_block("cell", cell);
    }
    
    // Set other metadata
    for (const auto& [k, v] : kv) {
        if (k == "Properties" || k == "Lattice") continue;
        set_typed_metadata(frame, k, v);
    }

    // Prepare arrays
    xt::xarray<float> coords = xt::zeros<float>({static_cast<size_t>(n_atoms), static_cast<size_t>(3)});
    xt::xarray<float> velocities; 
    bool has_velo = false;
    std::unordered_map<std::string, xt::xarray<float>> numeric_props;
    
    for (const auto& p : props) {
        if (p.kind == ExtProperty::VECTOR3 && (p.name == "velo" || p.name == "velocity")) {
            velocities = xt::zeros<float>({static_cast<size_t>(n_atoms), static_cast<size_t>(3)});
            has_velo = true;
        } else if (p.kind == ExtProperty::DOUBLE) {
            numeric_props.emplace(p.name, xt::zeros<float>({static_cast<size_t>(n_atoms)}));
        } else if (p.kind == ExtProperty::VECTOR3) {
            numeric_props.emplace(p.name + "_x", xt::zeros<float>({static_cast<size_t>(n_atoms)}));
            numeric_props.emplace(p.name + "_y", xt::zeros<float>({static_cast<size_t>(n_atoms)}));
            numeric_props.emplace(p.name + "_z", xt::zeros<float>({static_cast<size_t>(n_atoms)}));
        }
    }

    xt::xarray<float> atomic_numbers = xt::zeros<float>({static_cast<size_t>(n_atoms)});

    // Parse atoms
    for (int i = 0; i < n_atoms; ++i) {
        const std::string& line = lines[2 + i];
        std::istringstream iss(line);
        std::string species; 
        float x, y, z;
        if (!(iss >> species >> x >> y >> z)) {
            throw std::runtime_error("Invalid atom line in XYZ");
        }
        
        coords(i,0)=x; coords(i,1)=y; coords(i,2)=z;
        
        try {
            auto elem = Element::create(species);
            atomic_numbers(i) = static_cast<float>(elem.number);
        } catch (...) {
            atomic_numbers(i) = 0;
        }
        
        // Parse extended properties
        for (const auto& p : props) {
            if (!iss) break;
            if (p.kind == ExtProperty::DOUBLE) {
                double d; 
                if (iss >> d) {
                    auto it = numeric_props.find(p.name);
                    if (it != numeric_props.end()) it->second(i) = static_cast<float>(d);
                }
            } else if (p.kind == ExtProperty::VECTOR3) {
                double a,b,c; 
                if (iss >> a >> b >> c) {
                    if (has_velo && (p.name == "velo" || p.name == "velocity")) {
                        velocities(i,0)=static_cast<float>(a);
                        velocities(i,1)=static_cast<float>(b);
                        velocities(i,2)=static_cast<float>(c);
                    } else {
                        numeric_props[p.name + "_x"](i) = static_cast<float>(a);
                        numeric_props[p.name + "_y"](i) = static_cast<float>(b);
                        numeric_props[p.name + "_z"](i) = static_cast<float>(c);
                    }
                }
            }
        }
    }

    molcpp::Block atoms;
    atoms.set("coordinates", coords);
    atoms.set("atomic_numbers", atomic_numbers);
    if (has_velo) atoms.set("velocities", velocities);
    for (auto& [k, arr] : numeric_props) {
        atoms.set(k, arr);
    }
    frame.set_block("atoms", atoms);
    frame.set_metadata("n_atoms", static_cast<size_t>(n_atoms));

    return frame;
}

AtomData parse_atom_line(const std::string& line) {
    std::istringstream iss(line);
    AtomData data;
    
    float x, y, z;
    if (!(iss >> data.element >> x >> y >> z)) {
        throw std::runtime_error("Invalid atom line format");
    }
    
    data.coordinates = {x, y, z};
    
    // Get any remaining data as additional_data
    std::string remaining;
    std::getline(iss, remaining);
    data.additional_data = trim_copy(remaining);
    
    return data;
}

std::pair<int, std::string> parse_header(const std::string& atom_count_line, 
                                       const std::string& comment_line) {
    int n_atoms = 0;
    try { 
        n_atoms = std::stoi(atom_count_line); 
    } catch (...) {
        throw std::runtime_error("Invalid atom count line");
    }
    
    return {n_atoms, comment_line};
}

bool validate_xyz_frame(const std::vector<std::string>& lines) {
    if (lines.size() < 2) return false;
    
    try {
        int n_atoms = std::stoi(lines[0]);
        return static_cast<int>(lines.size()) == 2 + n_atoms;
    } catch (...) {
        return false;
    }
}

} // namespace molcpp::io::xyz_parser