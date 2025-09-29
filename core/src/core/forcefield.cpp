#include "molcpp/core/forcefield.hpp"
#include "molcpp/core/ecs/entity.hpp"
#include "molcpp/core/atom.hpp"
#include <sstream>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <unordered_set>

namespace molcpp {

// Static member initialization
std::unordered_map<std::string, std::unordered_map<std::string, std::function<void*()>>> 
    ForceField::kernel_registry_;

// ParamContainer implementation
ParamContainer::ParamContainer(const ParamList& parms, const ParamMap& kwparms)
    : parms_(parms), kwparms_(kwparms) {
}

const ParamValue& ParamContainer::operator[](size_t index) const {
    if (index >= parms_.size()) {
        throw std::out_of_range("Parameter index out of range");
    }
    return parms_[index];
}

const ParamValue& ParamContainer::operator[](const std::string& key) const {
    auto it = kwparms_.find(key);
    if (it == kwparms_.end()) {
        throw std::out_of_range("Parameter key not found: " + key);
    }
    return it->second;
}

void ParamContainer::set(const std::string& key, const ParamValue& value) {
    kwparms_[key] = value;
}

bool ParamContainer::has(const std::string& key) const {
    return kwparms_.find(key) != kwparms_.end();
}

void ParamContainer::update(const ParamContainer& other) {
    // Merge keyword parameters (other's values take precedence)
    for (const auto& [key, value] : other.kwparms_) {
        kwparms_[key] = value;
    }
    
    // Note: Positional parameters are not merged - they belong to the original container
}

// Type implementation
Type::Type(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : ParamContainer(parms, kwparms), name_(name) {
}

std::string Type::toString() const {
    std::ostringstream oss;
    oss << "<" << typeid(*this).name() << ": " << name_ << ">";
    return oss.str();
}

// TypeContainer template implementation
template<typename T>
const T& TypeContainer<T>::operator[](size_t index) const {
    if (index >= types_.size()) {
        throw std::out_of_range("Type index out of range");
    }
    return types_[index];
}

template<typename T>
T& TypeContainer<T>::operator[](size_t index) {
    if (index >= types_.size()) {
        throw std::out_of_range("Type index out of range");
    }
    return types_[index];
}

template<typename T>
const T& TypeContainer<T>::operator[](const std::string& name) const {
    auto it = std::find_if(types_.begin(), types_.end(),
        [&name](const T& type) { return type.getName() == name; });
    
    if (it == types_.end()) {
        throw std::out_of_range("Type not found: " + name);
    }
    return *it;
}

template<typename T>
void TypeContainer<T>::add(const T& type) {
    types_.push_back(type);
}

template<typename T>
void TypeContainer<T>::add(T&& type) {
    types_.push_back(std::move(type));
}

template<typename T>
T& TypeContainer<T>::emplace_back(const T& type) {
    types_.push_back(type);
    return types_.back();
}

template<typename T>
template<typename... Args>
T& TypeContainer<T>::emplace(Args&&... args) {
    types_.emplace_back(std::forward<Args>(args)...);
    return types_.back();
}

template<typename T>
const T* TypeContainer<T>::get(const std::string& name) const {
    auto it = std::find_if(types_.begin(), types_.end(),
        [&name](const T& type) { return type.getName() == name; });
    
    return (it != types_.end()) ? &(*it) : nullptr;
}

template<typename T>
T* TypeContainer<T>::get(const std::string& name) {
    auto it = std::find_if(types_.begin(), types_.end(),
        [&name](const T& type) { return type.getName() == name; });
    
    return (it != types_.end()) ? &(*it) : nullptr;
}

template<typename T>
std::vector<const T*> TypeContainer<T>::get_all_by(std::function<bool(const T&)> condition) const {
    std::vector<const T*> result;
    for (const auto& type : types_) {
        if (condition(type)) {
            result.push_back(&type);
        }
    }
    return result;
}

template<typename T>
void TypeContainer<T>::update(const TypeContainer<T>& other) {
    // Create a map of existing types by name for quick lookup
    std::unordered_map<std::string, size_t> name_to_index;
    for (size_t i = 0; i < types_.size(); ++i) {
        name_to_index[types_[i].getName()] = i;
    }
    
    // Process types from other container
    for (const auto& other_type : other.types_) {
        auto it = name_to_index.find(other_type.getName());
        if (it != name_to_index.end()) {
            // Type with this name exists, replace it
            types_[it->second] = other_type;
        } else {
            // New type, add it
            types_.push_back(other_type);
        }
    }
}

// Explicit template instantiations for commonly used types
template class TypeContainer<Type>;
template class TypeContainer<AtomType>;
template class TypeContainer<BondType>;
template class TypeContainer<AngleType>;
template class TypeContainer<DihedralType>;
template class TypeContainer<ImproperType>;
template class TypeContainer<PairType>;

// AtomType implementation
AtomType::AtomType(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Type(name, parms, kwparms) {
}

bool AtomType::match(const molcpp::ecs::Entity& entity) const {
    // This is a basic implementation - in practice, you might want to
    // check specific components or properties of the entity
    // For now, we'll implement a simple placeholder
    return false; // TODO: Implement proper matching logic
}

void AtomType::apply(molcpp::Atom& atom) const {
    // This would apply the type's properties to the atom
    // Implementation depends on how properties are stored in Atom
    // For now, we'll implement a placeholder
    // TODO: Implement proper application logic
}

std::string AtomType::toString() const {
    std::ostringstream oss;
    oss << "<AtomType: " << getName() << ">";
    return oss.str();
}

// BondType implementation
BondType::BondType(const AtomType& itype, const AtomType& jtype, const std::string& name,
                   const ParamList& parms, const ParamMap& kwparms)
    : Type(name.empty() ? (itype.getName() + "-" + jtype.getName()) : name, parms, kwparms),
      itype_(itype), jtype_(jtype) {
}

std::vector<const AtomType*> BondType::get_atom_types() const {
    return {&itype_, &jtype_};
}

std::string BondType::toString() const {
    std::ostringstream oss;
    oss << "<BondType: " << getName() << ">";
    return oss.str();
}

// AngleType implementation
AngleType::AngleType(const AtomType& itype, const AtomType& jtype, const AtomType& ktype,
                     const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Type(name.empty() ? (itype.getName() + "-" + jtype.getName() + "-" + ktype.getName()) : name, 
           parms, kwparms),
      itype_(itype), jtype_(jtype), ktype_(ktype) {
}

std::vector<const AtomType*> AngleType::get_atom_types() const {
    return {&itype_, &jtype_, &ktype_};
}

std::string AngleType::toString() const {
    std::ostringstream oss;
    oss << "<AngleType: " << getName() << ">";
    return oss.str();
}

// DihedralType implementation
DihedralType::DihedralType(const AtomType& itype, const AtomType& jtype, 
                           const AtomType& ktype, const AtomType& ltype,
                           const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Type(name.empty() ? (itype.getName() + "-" + jtype.getName() + "-" + 
                          ktype.getName() + "-" + ltype.getName()) : name, 
           parms, kwparms),
      itype_(itype), jtype_(jtype), ktype_(ktype), ltype_(ltype) {
}

std::vector<const AtomType*> DihedralType::get_atom_types() const {
    return {&itype_, &jtype_, &ktype_, &ltype_};
}

std::string DihedralType::toString() const {
    std::ostringstream oss;
    oss << "<DihedralType: " << getName() << ">";
    return oss.str();
}

// ImproperType implementation
ImproperType::ImproperType(const AtomType& itype, const AtomType& jtype, 
                           const AtomType& ktype, const AtomType& ltype,
                           const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Type(name.empty() ? (itype.getName() + "-" + jtype.getName() + "-" + 
                          ktype.getName() + "-" + ltype.getName()) : name, 
           parms, kwparms),
      itype_(itype), jtype_(jtype), ktype_(ktype), ltype_(ltype) {
}

std::vector<const AtomType*> ImproperType::get_atom_types() const {
    return {&itype_, &jtype_, &ktype_, &ltype_};
}

std::string ImproperType::toString() const {
    std::ostringstream oss;
    oss << "<ImproperType: " << getName() << ">";
    return oss.str();
}

// PairType implementation
PairType::PairType(const AtomType& itype, const AtomType& jtype, const std::string& name,
                   const ParamList& parms, const ParamMap& kwparms)
    : Type(name.empty() ? (itype.getName() + "-" + jtype.getName()) : name, parms, kwparms),
      itype_(itype), jtype_(jtype) {
}

std::vector<const AtomType*> PairType::get_atom_types() const {
    return {&itype_, &jtype_};
}

std::string PairType::toString() const {
    std::ostringstream oss;
    oss << "<PairType: " << getName() << ">";
    return oss.str();
}

// Style base class implementation
Style::Style(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : ParamContainer(parms, kwparms), name_(name) {
}

std::string Style::toString() const {
    std::ostringstream oss;
    oss << "<" << typeid(*this).name() << ": " << name_ << ">";
    return oss.str();
}

Style& Style::merge(const Style& other) {
    // Merge parameter containers
    this->update(other);
    return *this;
}

// AtomStyle implementation
AtomStyle::AtomStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

AtomType& AtomStyle::defType(const std::string& name, const std::string& class_name,
                                 const ParamList& parms, const ParamMap& kwparms) {
    AtomType atom_type(name, parms, kwparms);
    auto& added_type = types_.emplace_back(atom_type);
    
    if (!class_name.empty()) {
        classes_[class_name].insert(name);
    }
    
    return added_type;
}

std::vector<const AtomType*> AtomStyle::getClass(const std::string& class_name) const {
    std::vector<const AtomType*> result;
    
    auto it = classes_.find(class_name);
    if (it != classes_.end()) {
        for (const auto& type_name : it->second) {
            const auto* type = types_.get(type_name);
            if (type) {
                result.push_back(type);
            }
        }
    }
    
    return result;
}

// BondStyle implementation
BondStyle::BondStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

BondType& BondStyle::defType(const AtomType& itype, const AtomType& jtype,
                                 const std::string& name, const ParamList& parms, 
                                 const ParamMap& kwparms) {
    BondType bond_type(itype, jtype, name, parms, kwparms);
    return types_.emplace_back(bond_type);
}

// AngleStyle implementation
AngleStyle::AngleStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

AngleType& AngleStyle::defType(const AtomType& itype, const AtomType& jtype, 
                                   const AtomType& ktype, const std::string& name,
                                   const ParamList& parms, const ParamMap& kwparms) {
    AngleType angle_type(itype, jtype, ktype, name, parms, kwparms);
    return types_.emplace_back(angle_type);
}

// DihedralStyle implementation
DihedralStyle::DihedralStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

DihedralType& DihedralStyle::defType(const AtomType& itype, const AtomType& jtype,
                                         const AtomType& ktype, const AtomType& ltype,
                                         const std::string& name, const ParamList& parms, 
                                         const ParamMap& kwparms) {
    DihedralType dihedral_type(itype, jtype, ktype, ltype, name, parms, kwparms);
    return types_.emplace_back(dihedral_type);
}

// ImproperStyle implementation
ImproperStyle::ImproperStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

ImproperType& ImproperStyle::defType(const AtomType& itype, const AtomType& jtype,
                                         const AtomType& ktype, const AtomType& ltype,
                                         const std::string& name, const ParamList& parms, 
                                         const ParamMap& kwparms) {
    ImproperType improper_type(itype, jtype, ktype, ltype, name, parms, kwparms);
    return types_.emplace_back(improper_type);
}

// PairStyle implementation
PairStyle::PairStyle(const std::string& name, const ParamList& parms, const ParamMap& kwparms)
    : Style(name, parms, kwparms) {
}

PairType& PairStyle::defType(const AtomType& itype, const AtomType& jtype,
                                 const std::string& name, const ParamList& parms, 
                                 const ParamMap& kwparms) {
    PairType pair_type(itype, jtype, name, parms, kwparms);
    return types_.emplace_back(pair_type);
}

// ForceField implementation
ForceField::ForceField(const std::string& name, const std::string& units)
    : name_(name), units_(units) {
}

ForceField ForceField::fromForcefields(const std::string& name, 
                                       const std::vector<ForceField>& forcefields) {
    ForceField result(name);
    for (const auto& ff : forcefields) {
        result.merge(ff);
    }
    return result;
}

// Style definition methods
AtomStyle& ForceField::defAtomStyle(const std::string& style_name,
                                         const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = get_atom_style(style_name);
    if (existing) {
        return *existing;
    }
    
    atom_styles_.emplace_back(style_name, parms, kwparms);
    return atom_styles_.back();
}

BondStyle& ForceField::defBondStyle(const std::string& style_name,
                                         const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = get_bond_style(style_name);
    if (existing) {
        return *existing;
    }
    
    bond_styles_.emplace_back(style_name, parms, kwparms);
    return bond_styles_.back();
}

AngleStyle& ForceField::defAngleStyle(const std::string& style_name,
                                           const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = getAngleStyle(style_name);
    if (existing) {
        return *existing;
    }
    
    angle_styles_.emplace_back(style_name, parms, kwparms);
    return angle_styles_.back();
}

DihedralStyle& ForceField::defDihedralStyle(const std::string& style_name,
                                                 const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = getDihedralStyle(style_name);
    if (existing) {
        return *existing;
    }
    
    dihedral_styles_.emplace_back(style_name, parms, kwparms);
    return dihedral_styles_.back();
}

ImproperStyle& ForceField::defImproperStyle(const std::string& style_name,
                                                 const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = getImproperStyle(style_name);
    if (existing) {
        return *existing;
    }
    
    improper_styles_.emplace_back(style_name, parms, kwparms);
    return improper_styles_.back();
}

PairStyle& ForceField::defPairStyle(const std::string& style_name,
                                         const ParamList& parms, const ParamMap& kwparms) {
    auto* existing = getPairStyle(style_name);
    if (existing) {
        return *existing;
    }
    
    pair_styles_.emplace_back(style_name, parms, kwparms);
    return pair_styles_.back();
}

// Style retrieval methods (const)
const AtomStyle* ForceField::get_atom_style(const std::string& name) const {
    return find_style(atom_styles_, name);
}

const BondStyle* ForceField::get_bond_style(const std::string& name) const {
    return find_style(bond_styles_, name);
}

const AngleStyle* ForceField::getAngleStyle(const std::string& name) const {
    return find_style(angle_styles_, name);
}

const DihedralStyle* ForceField::getDihedralStyle(const std::string& name) const {
    return find_style(dihedral_styles_, name);
}

const ImproperStyle* ForceField::getImproperStyle(const std::string& name) const {
    return find_style(improper_styles_, name);
}

const PairStyle* ForceField::getPairStyle(const std::string& name) const {
    return find_style(pair_styles_, name);
}

// Style retrieval methods (non-const)
AtomStyle* ForceField::get_atom_style(const std::string& name) {
    return find_style(atom_styles_, name);
}

BondStyle* ForceField::get_bond_style(const std::string& name) {
    return find_style(bond_styles_, name);
}

AngleStyle* ForceField::getAngleStyle(const std::string& name) {
    return find_style(angle_styles_, name);
}

DihedralStyle* ForceField::getDihedralStyle(const std::string& name) {
    return find_style(dihedral_styles_, name);
}

ImproperStyle* ForceField::getImproperStyle(const std::string& name) {
    return find_style(improper_styles_, name);
}

PairStyle* ForceField::getPairStyle(const std::string& name) {
    return find_style(pair_styles_, name);
}

// Type collection methods
std::vector<const AtomType*> ForceField::get_atom_types() const {
    std::vector<const AtomType*> result;
    for (const auto& style : atom_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

std::vector<const BondType*> ForceField::get_bond_types() const {
    std::vector<const BondType*> result;
    for (const auto& style : bond_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

std::vector<const AngleType*> ForceField::getAngleTypes() const {
    std::vector<const AngleType*> result;
    for (const auto& style : angle_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

std::vector<const DihedralType*> ForceField::getDihedralTypes() const {
    std::vector<const DihedralType*> result;
    for (const auto& style : dihedral_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

std::vector<const ImproperType*> ForceField::getImproperTypes() const {
    std::vector<const ImproperType*> result;
    for (const auto& style : improper_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

std::vector<const PairType*> ForceField::getPairTypes() const {
    std::vector<const PairType*> result;
    for (const auto& style : pair_styles_) {
        const auto& types = style.getTypes();
        for (const auto& type : types) {
            result.push_back(&type);
        }
    }
    return result;
}

// Count methods
size_t ForceField::getNumAtomTypes() const {
    return std::accumulate(atom_styles_.begin(), atom_styles_.end(), size_t(0),
        [](size_t sum, const AtomStyle& style) { return sum + style.getNumTypes(); });
}

size_t ForceField::getNumBondTypes() const {
    return std::accumulate(bond_styles_.begin(), bond_styles_.end(), size_t(0),
        [](size_t sum, const BondStyle& style) { return sum + style.getNumTypes(); });
}

size_t ForceField::getNumAngleTypes() const {
    return std::accumulate(angle_styles_.begin(), angle_styles_.end(), size_t(0),
        [](size_t sum, const AngleStyle& style) { return sum + style.getNumTypes(); });
}

size_t ForceField::getNumDihedralTypes() const {
    return std::accumulate(dihedral_styles_.begin(), dihedral_styles_.end(), size_t(0),
        [](size_t sum, const DihedralStyle& style) { return sum + style.getNumTypes(); });
}

size_t ForceField::getNumImproperTypes() const {
    return std::accumulate(improper_styles_.begin(), improper_styles_.end(), size_t(0),
        [](size_t sum, const ImproperStyle& style) { return sum + style.getNumTypes(); });
}

size_t ForceField::getNumPairTypes() const {
    return std::accumulate(pair_styles_.begin(), pair_styles_.end(), size_t(0),
        [](size_t sum, const PairStyle& style) { return sum + style.getNumTypes(); });
}

// Utility methods
bool ForceField::contains(const std::string& name) const {
    return getStyle(name) != nullptr;
}

const Style* ForceField::getStyle(const std::string& name) const {
    // Check all style types
    if (auto* style = get_atom_style(name)) return style;
    if (auto* style = get_bond_style(name)) return style;
    if (auto* style = getAngleStyle(name)) return style;
    if (auto* style = getDihedralStyle(name)) return style;
    if (auto* style = getImproperStyle(name)) return style;
    if (auto* style = getPairStyle(name)) return style;
    
    return nullptr;
}

Style* ForceField::getStyle(const std::string& name) {
    // Check all style types
    if (auto* style = get_atom_style(name)) return style;
    if (auto* style = get_bond_style(name)) return style;
    if (auto* style = getAngleStyle(name)) return style;
    if (auto* style = getDihedralStyle(name)) return style;
    if (auto* style = getImproperStyle(name)) return style;
    if (auto* style = getPairStyle(name)) return style;
    
    return nullptr;
}

size_t ForceField::getTotalNumStyles() const {
    return atom_styles_.size() + bond_styles_.size() + angle_styles_.size() + 
           dihedral_styles_.size() + improper_styles_.size() + pair_styles_.size();
}

ForceField& ForceField::merge(const ForceField& other) {
    // Merge styles - only add if not already present
    for (const auto& style : other.atom_styles_) {
        if (!get_atom_style(style.getName())) {
            atom_styles_.push_back(style);
        }
    }
    
    for (const auto& style : other.bond_styles_) {
        if (!get_bond_style(style.getName())) {
            bond_styles_.push_back(style);
        }
    }
    
    for (const auto& style : other.angle_styles_) {
        if (!getAngleStyle(style.getName())) {
            angle_styles_.push_back(style);
        }
    }
    
    for (const auto& style : other.dihedral_styles_) {
        if (!getDihedralStyle(style.getName())) {
            dihedral_styles_.push_back(style);
        }
    }
    
    for (const auto& style : other.improper_styles_) {
        if (!getImproperStyle(style.getName())) {
            improper_styles_.push_back(style);
        }
    }
    
    for (const auto& style : other.pair_styles_) {
        if (!getPairStyle(style.getName())) {
            pair_styles_.push_back(style);
        }
    }
    
    return *this;
}

std::string ForceField::toString() const {
    std::ostringstream oss;
    oss << "<ForceField: " << name_;
    
    if (getNumAtomStyles() > 0) {
        oss << "\nn_atomstyles: " << getNumAtomStyles() 
            << ", n_atomtypes: " << getNumAtomTypes();
    }
    if (getNumBondStyles() > 0) {
        oss << "\nn_bondstyles: " << getNumBondStyles() 
            << ", n_bondtypes: " << getNumBondTypes();
    }
    if (getNumPairStyles() > 0) {
        oss << "\nn_pairstyles: " << getNumPairStyles() 
            << ", n_pairtypes: " << getNumPairTypes();
    }
    if (getNumAngleStyles() > 0) {
        oss << "\nn_anglestyles: " << getNumAngleStyles() 
            << ", n_angletypes: " << getNumAngleTypes();
    }
    if (getNumDihedralStyles() > 0) {
        oss << "\nn_dihedralstyles: " << getNumDihedralStyles() 
            << ", n_dihedraltypes: " << getNumDihedralTypes();
    }
    if (getNumImproperStyles() > 0) {
        oss << "\nn_improperstyles: " << getNumImproperStyles() 
            << ", n_impropertypes: " << getNumImproperTypes();
    }
    
    oss << ">";
    return oss.str();
}

std::unordered_map<std::string, ParamValue> ForceField::toDict() const {
    // Simplified serialization - in practice, this would be more complex
    std::unordered_map<std::string, ParamValue> result;
    result["name"] = name_;
    result["units"] = units_;
    result["num_atom_styles"] = static_cast<int>(getNumAtomStyles());
    result["num_bond_styles"] = static_cast<int>(getNumBondStyles());
    result["num_angle_styles"] = static_cast<int>(getNumAngleStyles());
    result["num_dihedral_styles"] = static_cast<int>(getNumDihedralStyles());
    result["num_improper_styles"] = static_cast<int>(getNumImproperStyles());
    result["num_pair_styles"] = static_cast<int>(getNumPairStyles());
    
    return result;
}

ForceField ForceField::fromDict(const std::unordered_map<std::string, ParamValue>& dict) {
    // Simplified deserialization - in practice, this would reconstruct the full object
    std::string name = "unnamed";
    std::string units = "real";
    
    auto name_it = dict.find("name");
    if (name_it != dict.end()) {
        name = std::get<std::string>(name_it->second);
    }
    
    auto units_it = dict.find("units");
    if (units_it != dict.end()) {
        units = std::get<std::string>(units_it->second);
    }
    
    return ForceField(name, units);
}

// Helper template methods
template<typename StyleType>
const StyleType* ForceField::find_style(const std::vector<StyleType>& styles, 
                                        const std::string& name) const {
    auto it = std::find_if(styles.begin(), styles.end(),
        [&name](const StyleType& style) { return style.getName() == name; });
    
    return (it != styles.end()) ? &(*it) : nullptr;
}

template<typename StyleType>
StyleType* ForceField::find_style(std::vector<StyleType>& styles, 
                                  const std::string& name) {
    auto it = std::find_if(styles.begin(), styles.end(),
        [&name](const StyleType& style) { return style.getName() == name; });
    
    return (it != styles.end()) ? &(*it) : nullptr;
}

// KernelRegistry implementation
void KernelRegistry::register_kernel(const std::string& type_name,
                                     const std::string& kernel_name,
                                     std::function<void*()> factory) {
    ForceField::kernel_registry_[type_name][kernel_name] = factory;
}

std::function<void*()> KernelRegistry::get_kernel_factory(const std::string& type_name,
                                                          const std::string& kernel_name) {
    auto type_it = ForceField::kernel_registry_.find(type_name);
    if (type_it != ForceField::kernel_registry_.end()) {
        auto kernel_it = type_it->second.find(kernel_name);
        if (kernel_it != type_it->second.end()) {
            return kernel_it->second;
        }
    }
    return nullptr;
}

bool KernelRegistry::has_kernel(const std::string& type_name,
                                const std::string& kernel_name) {
    return get_kernel_factory(type_name, kernel_name) != nullptr;
}

// Helper functions
ForceField createLjForcefield(const std::string& name) {
    ForceField ff(name, "real");
    
    // Create basic LJ atom style
    auto& atom_style = ff.defAtomStyle("lj");
    
    // Add some common atom types (basic examples)
    atom_style.defType("C", "sp3", {12.01}, {{"epsilon", 0.07}, {"sigma", 3.4}});
    atom_style.defType("H", "sp3", {1.008}, {{"epsilon", 0.03}, {"sigma", 2.5}});
    atom_style.defType("O", "sp3", {15.999}, {{"epsilon", 0.15}, {"sigma", 3.12}});
    atom_style.defType("N", "sp3", {14.007}, {{"epsilon", 0.17}, {"sigma", 3.25}});
    
    return ff;
}

ForceField createMolecularForcefield(const std::string& name) {
    ForceField ff = createLjForcefield(name);
    
    // Add bond style
    auto& bond_style = ff.defBondStyle("harmonic");
    
    // Add angle style  
    auto& angle_style = ff.defAngleStyle("harmonic");
    
    // Get atom types for defining bonds and angles
    auto atom_types = ff.get_atom_types();
    if (atom_types.size() >= 2) {
        // Define some basic bond types (example)
        const AtomType* c_type = nullptr;
        const AtomType* h_type = nullptr;
        
        for (const auto* type : atom_types) {
            if (type->getName() == "C") c_type = type;
            if (type->getName() == "H") h_type = type;
        }
        
        if (c_type && h_type) {
            bond_style.defType(*c_type, *h_type, "", {340.0, 1.09});
            angle_style.defType(*h_type, *c_type, *h_type, "", {33.0, 107.8});
        }
    }
    
    return ff;
}

} // namespace molcpp
