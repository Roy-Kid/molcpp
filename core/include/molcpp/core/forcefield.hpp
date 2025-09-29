#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <any>
#include <unordered_set>

namespace molcpp {
    // Forward declarations
    class Atom;
    class Bond;
    class Angle;
    namespace ecs { class Entity; }
}

namespace molcpp {

// Forward declarations for Param system
class ValueRef;
class ParamList;
class ParamMap;
class ParamContainer;

/**
 * @brief A thin wrapper around std::any for parameter values.
 * 
 * Provides get<T>() for type-safe access and set(T) for assignment.
 * No error checking - assumes user knows correct type.
 */
class ParamValue {
private:
    std::any value_;

public:
    ParamValue() = default;
    
    template<typename T>
    ParamValue(T&& value) : value_(std::forward<T>(value)) {}
    
    template<typename T>
    ParamValue& operator=(T&& value) {
        value_ = std::forward<T>(value);
        return *this;
    }
    
    template<typename T>
    T& get() {
        return std::any_cast<T&>(value_);
    }
    
    template<typename T>
    const T& get() const {
        return std::any_cast<const T&>(value_);
    }
    
    template<typename T>
    void set(T&& value) {
        value_ = std::forward<T>(value);
    }
    
    bool has_value() const noexcept {
        return value_.has_value();
    }
};

/**
 * @brief Lightweight proxy for container operator[] access.
 * 
 * Supports both assignment and implicit type conversion.
 * Enables ergonomic syntax like `int a = plist[i];`
 */
class ValueRef {
private:
    ParamValue* value_ptr_;

public:
    explicit ValueRef(ParamValue* ptr) : value_ptr_(ptr) {}
    
    // Assignment operator - stores value as any
    template<typename T>
    ValueRef& operator=(T&& value) {
        if (!value_ptr_) {
            // This shouldn't happen in normal usage
            return *this;
        }
        value_ptr_->set(std::forward<T>(value));
        return *this;
    }
    
    // Implicit conversion operator - deduces type from target variable
    template<typename T>
    operator T() const {
        return value_ptr_->get<T>();
    }
    
    // Direct reference access
    template<typename T>
    T& ref() {
        return value_ptr_->get<T>();
    }
    
    template<typename T>
    const T& ref() const {
        return value_ptr_->get<T>();
    }
    
    // Check if value exists
    bool has_value() const {
        return value_ptr_ && value_ptr_->has_value();
    }
};

/**
 * @brief Container for positional parameters backed by std::vector<ParamValue>.
 * 
 * Auto-expands when writing to out-of-range indices.
 * Access via operator[] returns ValueRef proxy.
 */
class ParamList {
private:
    std::vector<ParamValue> values_;

public:
    using size_type = std::size_t;
    
    ParamList() = default;
    explicit ParamList(size_type size) : values_(size) {}
    
    // Access via operator[] - returns ValueRef proxy
    ValueRef operator[](size_type index) {
        if (index >= values_.size()) {
            values_.resize(index + 1);
        }
        return ValueRef(&values_[index]);
    }
    
    // Const access - returns copy of value
    template<typename T>
    T get(size_type index) const {
        return values_[index].get<T>();
    }
    
    // Non-const access - returns reference
    template<typename T>
    T& ref(size_type index) {
        if (index >= values_.size()) {
            values_.resize(index + 1);
        }
        return values_[index].get<T>();
    }
    
    // Const reference access
    template<typename T>
    const T& ref(size_type index) const {
        return values_[index].get<T>();
    }
    
    // Direct access to underlying vector
    std::vector<ParamValue>& data() { return values_; }
    const std::vector<ParamValue>& data() const { return values_; }
    
    size_type size() const noexcept { return values_.size(); }
    bool empty() const noexcept { return values_.empty(); }
    void resize(size_type size) { values_.resize(size); }
    void clear() { values_.clear(); }
};

/**
 * @brief Container for keyword parameters backed by std::unordered_map.
 * 
 * Access via operator[] returns ValueRef proxy.
 */
class ParamMap {
private:
    std::unordered_map<std::string, ParamValue> values_;

public:
    using key_type = std::string;
    using mapped_type = ParamValue;
    
    ParamMap() = default;
    
    // Access via operator[] - returns ValueRef proxy
    ValueRef operator[](const std::string& key) {
        return ValueRef(&values_[key]);
    }
    
    // Const access - returns copy of value
    template<typename T>
    T get(const std::string& key) const {
        return values_.at(key).get<T>();
    }
    
    // Non-const access - returns reference
    template<typename T>
    T& ref(const std::string& key) {
        return values_[key].get<T>();
    }
    
    // Const reference access
    template<typename T>
    const T& ref(const std::string& key) const {
        return values_.at(key).get<T>();
    }
    
    // Direct access to underlying map
    std::unordered_map<std::string, ParamValue>& data() { return values_; }
    const std::unordered_map<std::string, ParamValue>& data() const { return values_; }
    
    bool contains(const std::string& key) const {
        return values_.find(key) != values_.end();
    }
    
    size_t size() const noexcept { return values_.size(); }
    bool empty() const noexcept { return values_.empty(); }
    void clear() { values_.clear(); }
    
    // Iterator support
    auto begin() { return values_.begin(); }
    auto end() { return values_.end(); }
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }
};

/**
 * @brief Container that holds both ParamList and ParamMap.
 * 
 * Provides unified access to both positional and keyword parameters.
 * Keeps implementation simple while allowing ergonomic usage.
 */
class ParamContainer {
private:
    ParamList list_;
    ParamMap map_;

public:
    ParamContainer() = default;
    
    explicit ParamContainer(const ParamList& list) : list_(list) {}
    explicit ParamContainer(const ParamMap& map) : map_(map) {}
    ParamContainer(const ParamList& list, const ParamMap& map) 
        : list_(list), map_(map) {}
    
    // Access to underlying containers
    ParamList& list() { return list_; }
    const ParamList& list() const { return list_; }
    
    ParamMap& map() { return map_; }
    const ParamMap& map() const { return map_; }
    
    // Unified access - try list first, then map
    ValueRef operator[](size_t index) {
        return list_[index];
    }
    
    ValueRef operator[](const std::string& key) {
        return map_[key];
    }
    
    // Direct access methods
    template<typename T>
    T get(size_t index) const {
        return list_.get<T>(index);
    }
    
    template<typename T>
    T& ref(size_t index) {
        return list_.ref<T>(index);
    }
    
    template<typename T>
    const T& ref(size_t index) const {
        return list_.ref<T>(index);
    }
    
    template<typename T>
    T get(const std::string& key) const {
        return map_.get<T>(key);
    }
    
    template<typename T>
    T& ref(const std::string& key) {
        return map_.ref<T>(key);
    }
    
    template<typename T>
    const T& ref(const std::string& key) const {
        return map_.ref<T>(key);
    }
    
    // Utility methods
    bool has(const std::string& key) const {
        return map_.contains(key);
    }
    
    size_t list_size() const { return list_.size(); }
    size_t map_size() const { return map_.size(); }
    bool empty() const { return list_.empty() && map_.empty(); }
    
    void clear() {
        list_.clear();
        map_.clear();
    }

};

// Forward declarations
class Type;
class AtomType;
template<typename T> class TypeContainer;

/**
 * @brief Base class for all force field types.
 * 
 * A Type represents a category of interaction with associated parameters.
 * It combines positional and keyword parameters similar to the Python version.
 */
class Type : public ParamContainer {
public:
    /**
     * @brief Construct a new Type.
     * @param name The name of the type
     * @param parms Positional parameters (unnamed, order-dependent)
     * @param kwparms Keyword parameters (named, as a map)
     */
    explicit Type(const std::string& name, 
                  const ParamList& parms = {}, 
                  const ParamMap& kwparms = {})
        : ParamContainer(parms, kwparms), name_(name) {}

    /**
     * @brief Virtual destructor.
     */
    virtual ~Type() = default;

    /**
     * @brief Copy constructor.
     */
    Type(const Type& other) = default;

    /**
     * @brief Move constructor.
     */
    Type(Type&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    Type& operator=(const Type& other) = default;

    /**
     * @brief Move assignment operator.
     */
    Type& operator=(Type&& other) noexcept = default;

    /**
     * @brief Get the name of the type.
     * @return const std::string& Reference to the type name
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Set the name of the type.
     * @param name The new name for the type
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Compare types for equality based on name.
     * @param other The other type to compare with
     * @return true if names are equal
     */
    bool operator==(const Type& other) const { return name_ == other.name_; }

    /**
     * @brief Compare type with string name.
     * @param name The name to compare with
     * @return true if names are equal
     */
    bool operator==(const std::string& name) const { return name_ == name; }

    /**
     * @brief Hash function for use in containers.
     * @return size_t Hash value based on name
     */
    size_t hash() const { return std::hash<std::string>{}(name_); }

    /**
     * @brief String representation of the type.
     * @return std::string String representation
     */
    virtual std::string toString() const;

private:
    std::string name_;  ///< Name of the type
};

/**
 * @brief Container for managing a collection of Type objects.
 * @tparam T The specific Type subclass (e.g., AtomType, BondType)
 */
template<typename T = Type>
class TypeContainer {
public:
    static_assert(std::is_base_of_v<Type, T>, "T must derive from Type");

    /**
     * @brief Default constructor.
     */
    TypeContainer() = default;

    /**
     * @brief Get type by index.
     * @param index Index of the type
     * @return const T& Reference to the type
     * @throws std::out_of_range if index is invalid
     */
    const T& operator[](size_t index) const;

    /**
     * @brief Get type by index (non-const).
     * @param index Index of the type
     * @return T& Reference to the type
     * @throws std::out_of_range if index is invalid
     */
    T& operator[](size_t index);

    /**
     * @brief Get type by name.
     * @param name Name of the type
     * @return const T& Reference to the type
     * @throws std::out_of_range if type is not found
     */
    const T& operator[](const std::string& name) const;

    /**
     * @brief Add a type to the container.
     * @param type The type to add (copy)
     */
    void add(const T& type);

    /**
     * @brief Add a type to the container.
     * @param type The type to add (move)
     */
    void add(T&& type);

    /**
     * @brief Add a type to the container by constructing it in place.
     * @param type_args Arguments to construct the type
     * @return T& Reference to the created type
     */
    T& emplace_back(const T& type);

    /**
     * @brief Emplace a type by constructing it in place with given arguments.
     * @param args Arguments to construct the type
     * @return T& Reference to the created type
     */
    template<typename... Args>
    T& emplace(Args&&... args);

    /**
     * @brief Get type by name with default.
     * @param name Name of the type
     * @param default_value Default value if not found
     * @return const T* Pointer to the type, or nullptr if not found
     */
    const T* get(const std::string& name) const;

    /**
     * @brief Get type by name with default (non-const).
     * @param name Name of the type
     * @return T* Pointer to the type, or nullptr if not found
     */
    T* get(const std::string& name);

    /**
     * @brief Get all types that satisfy a condition.
     * @param condition Function that takes a type and returns bool
     * @return std::vector<const T*> Vector of pointers to matching types
     */
    std::vector<const T*> get_all_by(std::function<bool(const T&)> condition) const;

    /**
     * @brief Update container with types from another container.
     * @param other Another TypeContainer to merge from
     */
    void update(const TypeContainer<T>& other);

    /**
     * @brief Get number of types in container.
     * @return size_t Number of types
     */
    size_t size() const { return types_.size(); }

    /**
     * @brief Check if container is empty.
     * @return true if empty
     */
    bool empty() const { return types_.empty(); }

    /**
     * @brief Iterator support - begin.
     */
    typename std::vector<T>::const_iterator begin() const { return types_.begin(); }

    /**
     * @brief Iterator support - end.
     */
    typename std::vector<T>::const_iterator end() const { return types_.end(); }

    /**
     * @brief Iterator support - begin (non-const).
     */
    typename std::vector<T>::iterator begin() { return types_.begin(); }

    /**
     * @brief Iterator support - end (non-const).
     */
    typename std::vector<T>::iterator end() { return types_.end(); }

private:
    std::vector<T> types_;  ///< Vector of types
};

/**
 * @brief Represents an atom type in a molecular force field.
 * 
 * This class defines the properties and behavior of a specific type of atom
 * within the context of a molecular simulation.
 */
class AtomType : public Type {
public:
    /**
     * @brief Construct a new AtomType.
     * @param name The name of the atom type
     * @param parms Positional parameters for the atom type
     * @param kwparms Keyword parameters for the atom type
     */
    explicit AtomType(const std::string& name,
                      const ParamList& parms = {},
                      const ParamMap& kwparms = {});

    /**
     * @brief Virtual destructor.
     */
    virtual ~AtomType() = default;

    /**
     * @brief Check if this atom type matches the given entity.
     * @param entity The entity to check (should be an Atom)
     * @return true if the type matches
     */
    virtual bool match(const molcpp::ecs::Entity& entity) const;

    /**
     * @brief Apply this atom type properties to the given atom.
     * @param atom The atom to apply the type to
     */
    virtual void apply(molcpp::Atom& atom) const;

    /**
     * @brief String representation specific to AtomType.
     * @return std::string String representation
     */
    std::string toString() const override;
};

/**
 * @brief Represents a bond type between two atom types.
 */
class BondType : public Type {
public:
    /**
     * @brief Construct a new BondType.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param name Custom name (if empty, will be generated from atom types)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    BondType(const AtomType& itype,
             const AtomType& jtype,
             const std::string& name = "",
             const ParamList& parms = {},
             const ParamMap& kwparms = {});

    /**
     * @brief Get the atom types involved in this bond.
     * @return std::vector<const AtomType*> Vector of atom type pointers
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get the first atom type.
     * @return const AtomType& Reference to the first atom type
     */
    const AtomType& get_i_type() const { return itype_; }

    /**
     * @brief Get the second atom type.
     * @return const AtomType& Reference to the second atom type
     */
    const AtomType& get_j_type() const { return jtype_; }

    /**
     * @brief String representation specific to BondType.
     * @return std::string String representation
     */
    std::string toString() const override;

private:
    AtomType itype_;  ///< First atom type
    AtomType jtype_;  ///< Second atom type
};

/**
 * @brief Represents an angle type between three atom types.
 */
class AngleType : public Type {
public:
    /**
     * @brief Construct a new AngleType.
     * @param itype The first atom type
     * @param jtype The second atom type (center)
     * @param ktype The third atom type
     * @param name Custom name (if empty, will be generated from atom types)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    AngleType(const AtomType& itype,
              const AtomType& jtype,
              const AtomType& ktype,
              const std::string& name = "",
              const ParamList& parms = {},
              const ParamMap& kwparms = {});

    /**
     * @brief Get the atom types involved in this angle.
     * @return std::vector<const AtomType*> Vector of atom type pointers
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get the first atom type.
     * @return const AtomType& Reference to the first atom type
     */
    const AtomType& getIType() const { return itype_; }

    /**
     * @brief Get the second atom type (center).
     * @return const AtomType& Reference to the second atom type
     */
    const AtomType& getJType() const { return jtype_; }

    /**
     * @brief Get the third atom type.
     * @return const AtomType& Reference to the third atom type
     */
    const AtomType& getKType() const { return ktype_; }

    /**
     * @brief String representation specific to AngleType.
     * @return std::string String representation
     */
    std::string toString() const override;

private:
    AtomType itype_;  ///< First atom type
    AtomType jtype_;  ///< Second atom type (center)
    AtomType ktype_;  ///< Third atom type
};

/**
 * @brief Represents a dihedral type between four atom types.
 */
class DihedralType : public Type {
public:
    /**
     * @brief Construct a new DihedralType.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param ktype The third atom type
     * @param ltype The fourth atom type
     * @param name Custom name (if empty, will be generated from atom types)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    DihedralType(const AtomType& itype,
                 const AtomType& jtype,
                 const AtomType& ktype,
                 const AtomType& ltype,
                 const std::string& name = "",
                 const ParamList& parms = {},
                 const ParamMap& kwparms = {});

    /**
     * @brief Get the atom types involved in this dihedral.
     * @return std::vector<const AtomType*> Vector of atom type pointers
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get the first atom type.
     * @return const AtomType& Reference to the first atom type
     */
    const AtomType& getIType() const { return itype_; }

    /**
     * @brief Get the second atom type.
     * @return const AtomType& Reference to the second atom type
     */
    const AtomType& getJType() const { return jtype_; }

    /**
     * @brief Get the third atom type.
     * @return const AtomType& Reference to the third atom type
     */
    const AtomType& getKType() const { return ktype_; }

    /**
     * @brief Get the fourth atom type.
     * @return const AtomType& Reference to the fourth atom type
     */
    const AtomType& getLType() const { return ltype_; }

    /**
     * @brief String representation specific to DihedralType.
     * @return std::string String representation
     */
    std::string toString() const override;

private:
    AtomType itype_;  ///< First atom type
    AtomType jtype_;  ///< Second atom type
    AtomType ktype_;  ///< Third atom type
    AtomType ltype_;  ///< Fourth atom type
};

/**
 * @brief Represents an improper type between four atom types.
 */
class ImproperType : public Type {
public:
    /**
     * @brief Construct a new ImproperType.
     * @param itype The first atom type
     * @param jtype The second atom type (usually the center)
     * @param ktype The third atom type
     * @param ltype The fourth atom type
     * @param name Custom name (if empty, will be generated from atom types)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    ImproperType(const AtomType& itype,
                 const AtomType& jtype,
                 const AtomType& ktype,
                 const AtomType& ltype,
                 const std::string& name = "",
                 const ParamList& parms = {},
                 const ParamMap& kwparms = {});

    /**
     * @brief Get the atom types involved in this improper.
     * @return std::vector<const AtomType*> Vector of atom type pointers
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get the first atom type.
     * @return const AtomType& Reference to the first atom type
     */
    const AtomType& getIType() const { return itype_; }

    /**
     * @brief Get the second atom type (center).
     * @return const AtomType& Reference to the second atom type
     */
    const AtomType& getJType() const { return jtype_; }

    /**
     * @brief Get the third atom type.
     * @return const AtomType& Reference to the third atom type
     */
    const AtomType& getKType() const { return ktype_; }

    /**
     * @brief Get the fourth atom type.
     * @return const AtomType& Reference to the fourth atom type
     */
    const AtomType& getLType() const { return ltype_; }

    /**
     * @brief String representation specific to ImproperType.
     * @return std::string String representation
     */
    std::string toString() const override;

private:
    AtomType itype_;  ///< First atom type
    AtomType jtype_;  ///< Second atom type (center)
    AtomType ktype_;  ///< Third atom type
    AtomType ltype_;  ///< Fourth atom type
};

/**
 * @brief Represents a pair interaction type between two atom types.
 */
class PairType : public Type {
public:
    /**
     * @brief Construct a new PairType.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param name Custom name (if empty, will be generated from atom types)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    PairType(const AtomType& itype,
             const AtomType& jtype,
             const std::string& name = "",
             const ParamList& parms = {},
             const ParamMap& kwparms = {});

    /**
     * @brief Get the atom types involved in this pair interaction.
     * @return std::vector<const AtomType*> Vector of atom type pointers
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get the first atom type.
     * @return const AtomType& Reference to the first atom type
     */
    const AtomType& getIType() const { return itype_; }

    /**
     * @brief Get the second atom type.
     * @return const AtomType& Reference to the second atom type
     */
    const AtomType& getJType() const { return jtype_; }

    /**
     * @brief String representation specific to PairType.
     * @return std::string String representation
     */
    std::string toString() const override;

private:
    AtomType itype_;  ///< First atom type
    AtomType jtype_;  ///< Second atom type
};

// Type aliases for commonly used containers
using AtomTypeContainer = TypeContainer<AtomType>;
using BondTypeContainer = TypeContainer<BondType>;
using AngleTypeContainer = TypeContainer<AngleType>;
using DihedralTypeContainer = TypeContainer<DihedralType>;
using ImproperTypeContainer = TypeContainer<ImproperType>;
using PairTypeContainer = TypeContainer<PairType>;

/**
 * @brief Base class for force field styles.
 * 
 * A Style defines a specific method or algorithm for calculating
 * a particular type of interaction (e.g., bond, angle, etc.).
 */
class Style : public ParamContainer {
public:
    /**
     * @brief Construct a new Style.
     * @param name The name of the style
     * @param parms Positional parameters for global configuration
     * @param kwparms Keyword parameters for global configuration
     */
    explicit Style(const std::string& name,
                   const ParamList& parms = {},
                   const ParamMap& kwparms = {});

    /**
     * @brief Virtual destructor.
     */
    virtual ~Style() = default;

    /**
     * @brief Copy constructor.
     */
    Style(const Style& other) = default;

    /**
     * @brief Move constructor.
     */
    Style(Style&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    Style& operator=(const Style& other) = default;

    /**
     * @brief Move assignment operator.
     */
    Style& operator=(Style&& other) noexcept = default;

    /**
     * @brief Get the name of the style.
     * @return const std::string& Reference to the style name
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Set the name of the style.
     * @param name The new name for the style
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    virtual size_t getNumTypes() const = 0;

    /**
     * @brief Compare styles for equality based on name.
     * @param other The other style to compare with
     * @return true if names are equal
     */
    bool operator==(const Style& other) const { return name_ == other.name_; }

    /**
     * @brief Compare style with string name.
     * @param name The name to compare with
     * @return true if names are equal
     */
    bool operator==(const std::string& name) const { return name_ == name; }

    /**
     * @brief Hash function for use in containers.
     * @return size_t Hash value based on name
     */
    size_t hash() const { return std::hash<std::string>{}(name_); }

    /**
     * @brief String representation of the style.
     * @return std::string String representation
     */
    virtual std::string toString() const;

    /**
     * @brief Merge another style into this one.
     * @param other Another style to merge from
     * @return Style& Reference to this style
     */
    virtual Style& merge(const Style& other);

private:
    std::string name_;  ///< Name of the style
};

/**
 * @brief Style for atom interactions.
 */
class AtomStyle : public Style {
public:
    /**
     * @brief Construct a new AtomStyle.
     * @param name The name of the atom style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit AtomStyle(const std::string& name,
                       const ParamList& parms = {},
                       const ParamMap& kwparms = {});

    /**
     * @brief Define a new atom type within this style.
     * @param name Name of the atom type
     * @param class_name Optional class name for grouping
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return AtomType& Reference to the created atom type
     */
    AtomType& defType(const std::string& name,
                      const std::string& class_name = "",
                      const ParamList& parms = {},
                      const ParamMap& kwparms = {});

    /**
     * @brief Get atom types by class name.
     * @param class_name The class name to filter by
     * @return std::vector<const AtomType*> Vector of atom types in the class
     */
    std::vector<const AtomType*> getClass(const std::string& class_name) const;

    /**
     * @brief Get all atom types in this style.
     * @return const AtomTypeContainer& Reference to the atom type container
     */
    const AtomTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all atom types in this style (non-const).
     * @return AtomTypeContainer& Reference to the atom type container
     */
    AtomTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    AtomTypeContainer types_;                                        ///< Container of atom types
    std::unordered_map<std::string, std::unordered_set<std::string>> classes_;  ///< Map of class names to type names
};

/**
 * @brief Style for bond interactions.
 */
class BondStyle : public Style {
public:
    /**
     * @brief Construct a new BondStyle.
     * @param name The name of the bond style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit BondStyle(const std::string& name,
                       const ParamList& parms = {},
                       const ParamMap& kwparms = {});

    /**
     * @brief Define a new bond type within this style.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param name Custom name (if empty, will be generated)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return BondType& Reference to the created bond type
     */
    BondType& defType(const AtomType& itype,
                      const AtomType& jtype,
                      const std::string& name = "",
                      const ParamList& parms = {},
                      const ParamMap& kwparms = {});

    /**
     * @brief Get all bond types in this style.
     * @return const BondTypeContainer& Reference to the bond type container
     */
    const BondTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all bond types in this style (non-const).
     * @return BondTypeContainer& Reference to the bond type container
     */
    BondTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    BondTypeContainer types_;  ///< Container of bond types
};

/**
 * @brief Style for angle interactions.
 */
class AngleStyle : public Style {
public:
    /**
     * @brief Construct a new AngleStyle.
     * @param name The name of the angle style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit AngleStyle(const std::string& name,
                        const ParamList& parms = {},
                        const ParamMap& kwparms = {});

    /**
     * @brief Define a new angle type within this style.
     * @param itype The first atom type
     * @param jtype The second atom type (center)
     * @param ktype The third atom type
     * @param name Custom name (if empty, will be generated)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return AngleType& Reference to the created angle type
     */
    AngleType& defType(const AtomType& itype,
                       const AtomType& jtype,
                       const AtomType& ktype,
                       const std::string& name = "",
                       const ParamList& parms = {},
                       const ParamMap& kwparms = {});

    /**
     * @brief Get all angle types in this style.
     * @return const AngleTypeContainer& Reference to the angle type container
     */
    const AngleTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all angle types in this style (non-const).
     * @return AngleTypeContainer& Reference to the angle type container
     */
    AngleTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    AngleTypeContainer types_;  ///< Container of angle types
};

/**
 * @brief Style for dihedral interactions.
 */
class DihedralStyle : public Style {
public:
    /**
     * @brief Construct a new DihedralStyle.
     * @param name The name of the dihedral style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit DihedralStyle(const std::string& name,
                           const ParamList& parms = {},
                           const ParamMap& kwparms = {});

    /**
     * @brief Define a new dihedral type within this style.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param ktype The third atom type
     * @param ltype The fourth atom type
     * @param name Custom name (if empty, will be generated)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return DihedralType& Reference to the created dihedral type
     */
    DihedralType& defType(const AtomType& itype,
                              const AtomType& jtype,
                              const AtomType& ktype,
                              const AtomType& ltype,
                              const std::string& name = "",
                              const ParamList& parms = {},
                              const ParamMap& kwparms = {});

    /**
     * @brief Get all dihedral types in this style.
     * @return const DihedralTypeContainer& Reference to the dihedral type container
     */
    const DihedralTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all dihedral types in this style (non-const).
     * @return DihedralTypeContainer& Reference to the dihedral type container
     */
    DihedralTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    DihedralTypeContainer types_;  ///< Container of dihedral types
};

/**
 * @brief Style for improper interactions.
 */
class ImproperStyle : public Style {
public:
    /**
     * @brief Construct a new ImproperStyle.
     * @param name The name of the improper style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit ImproperStyle(const std::string& name,
                           const ParamList& parms = {},
                           const ParamMap& kwparms = {});

    /**
     * @brief Define a new improper type within this style.
     * @param itype The first atom type
     * @param jtype The second atom type (center)
     * @param ktype The third atom type
     * @param ltype The fourth atom type
     * @param name Custom name (if empty, will be generated)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return ImproperType& Reference to the created improper type
     */
    ImproperType& defType(const AtomType& itype,
                              const AtomType& jtype,
                              const AtomType& ktype,
                              const AtomType& ltype,
                              const std::string& name = "",
                              const ParamList& parms = {},
                              const ParamMap& kwparms = {});

    /**
     * @brief Get all improper types in this style.
     * @return const ImproperTypeContainer& Reference to the improper type container
     */
    const ImproperTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all improper types in this style (non-const).
     * @return ImproperTypeContainer& Reference to the improper type container
     */
    ImproperTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    ImproperTypeContainer types_;  ///< Container of improper types
};

/**
 * @brief Style for pair interactions.
 */
class PairStyle : public Style {
public:
    /**
     * @brief Construct a new PairStyle.
     * @param name The name of the pair style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     */
    explicit PairStyle(const std::string& name,
                       const ParamList& parms = {},
                       const ParamMap& kwparms = {});

    /**
     * @brief Define a new pair type within this style.
     * @param itype The first atom type
     * @param jtype The second atom type
     * @param name Custom name (if empty, will be generated)
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return PairType& Reference to the created pair type
     */
    PairType& defType(const AtomType& itype,
                          const AtomType& jtype,
                          const std::string& name = "",
                          const ParamList& parms = {},
                          const ParamMap& kwparms = {});

    /**
     * @brief Get all pair types in this style.
     * @return const PairTypeContainer& Reference to the pair type container
     */
    const PairTypeContainer& getTypes() const { return types_; }

    /**
     * @brief Get all pair types in this style (non-const).
     * @return PairTypeContainer& Reference to the pair type container
     */
    PairTypeContainer& getTypes() { return types_; }

    /**
     * @brief Get the number of types in this style.
     * @return size_t Number of types
     */
    size_t getNumTypes() const override { return types_.size(); }

private:
    PairTypeContainer types_;  ///< Container of pair types
};

/**
 * @brief Main ForceField class representing a molecular force field.
 * 
 * A ForceField defines the styles and types of interactions between atoms,
 * bonds, angles, dihedrals, and impropers in a molecular system.
 * It serves as a container and manager for all force field components.
 */
class ForceField {
public:
    /**
     * @brief Construct a new ForceField.
     * @param name The name of the force field
     * @param units The units system (e.g., "real", "metal", "si")
     */
    explicit ForceField(const std::string& name = "", const std::string& units = "real");

    /**
     * @brief Virtual destructor.
     */
    virtual ~ForceField() = default;

    /**
     * @brief Copy constructor.
     */
    ForceField(const ForceField& other) = default;

    /**
     * @brief Move constructor.
     */
    ForceField(ForceField&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    ForceField& operator=(const ForceField& other) = default;

    /**
     * @brief Move assignment operator.
     */
    ForceField& operator=(ForceField&& other) noexcept = default;

    /**
     * @brief Create a ForceField from multiple other ForceFields.
     * @param name Name for the new force field
     * @param forcefields Vector of force fields to merge
     * @return ForceField New force field containing merged data
     */
    static ForceField fromForcefields(const std::string& name, 
                                      const std::vector<ForceField>& forcefields);

    /**
     * @brief Get the name of the force field.
     * @return const std::string& Reference to the force field name
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Set the name of the force field.
     * @param name The new name
     */
    void setName(const std::string& name) { name_ = name; }

    /**
     * @brief Get the units system.
     * @return const std::string& Reference to the units string
     */
    const std::string& getUnits() const { return units_; }

    /**
     * @brief Set the units system.
     * @param units The new units system
     */
    void setUnits(const std::string& units) { units_ = units; }

    // Style management methods
    /**
     * @brief Define or retrieve an atom style by name.
     * @param style_name Name of the atom style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return AtomStyle& Reference to the atom style
     */
    AtomStyle& defAtomStyle(const std::string& style_name,
                                 const ParamList& parms = {},
                                 const ParamMap& kwparms = {});

    /**
     * @brief Define or retrieve a bond style by name.
     * @param style_name Name of the bond style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return BondStyle& Reference to the bond style
     */
    BondStyle& defBondStyle(const std::string& style_name,
                                 const ParamList& parms = {},
                                 const ParamMap& kwparms = {});

    /**
     * @brief Define or retrieve an angle style by name.
     * @param style_name Name of the angle style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return AngleStyle& Reference to the angle style
     */
    AngleStyle& defAngleStyle(const std::string& style_name,
                                   const ParamList& parms = {},
                                   const ParamMap& kwparms = {});

    /**
     * @brief Define or retrieve a dihedral style by name.
     * @param style_name Name of the dihedral style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return DihedralStyle& Reference to the dihedral style
     */
    DihedralStyle& defDihedralStyle(const std::string& style_name,
                                         const ParamList& parms = {},
                                         const ParamMap& kwparms = {});

    /**
     * @brief Define or retrieve an improper style by name.
     * @param style_name Name of the improper style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return ImproperStyle& Reference to the improper style
     */
    ImproperStyle& defImproperStyle(const std::string& style_name,
                                         const ParamList& parms = {},
                                         const ParamMap& kwparms = {});

    /**
     * @brief Define or retrieve a pair style by name.
     * @param style_name Name of the pair style
     * @param parms Positional parameters
     * @param kwparms Keyword parameters
     * @return PairStyle& Reference to the pair style
     */
    PairStyle& defPairStyle(const std::string& style_name,
                                 const ParamList& parms = {},
                                 const ParamMap& kwparms = {});

    // Style retrieval methods
    /**
     * @brief Get atom style by name.
     * @param name Name of the atom style
     * @return const AtomStyle* Pointer to the style, or nullptr if not found
     */
    const AtomStyle* get_atom_style(const std::string& name) const;

    /**
     * @brief Get atom style by name (non-const).
     * @param name Name of the atom style
     * @return AtomStyle* Pointer to the style, or nullptr if not found
     */
    AtomStyle* get_atom_style(const std::string& name);

    /**
     * @brief Get bond style by name.
     * @param name Name of the bond style
     * @return const BondStyle* Pointer to the style, or nullptr if not found
     */
    const BondStyle* get_bond_style(const std::string& name) const;

    /**
     * @brief Get bond style by name (non-const).
     * @param name Name of the bond style
     * @return BondStyle* Pointer to the style, or nullptr if not found
     */
    BondStyle* get_bond_style(const std::string& name);

    /**
     * @brief Get angle style by name.
     * @param name Name of the angle style
     * @return const AngleStyle* Pointer to the style, or nullptr if not found
     */
    const AngleStyle* getAngleStyle(const std::string& name) const;

    /**
     * @brief Get angle style by name (non-const).
     * @param name Name of the angle style
     * @return AngleStyle* Pointer to the style, or nullptr if not found
     */
    AngleStyle* getAngleStyle(const std::string& name);

    /**
     * @brief Get dihedral style by name.
     * @param name Name of the dihedral style
     * @return const DihedralStyle* Pointer to the style, or nullptr if not found
     */
    const DihedralStyle* getDihedralStyle(const std::string& name) const;

    /**
     * @brief Get dihedral style by name (non-const).
     * @param name Name of the dihedral style
     * @return DihedralStyle* Pointer to the style, or nullptr if not found
     */
    DihedralStyle* getDihedralStyle(const std::string& name);

    /**
     * @brief Get improper style by name.
     * @param name Name of the improper style
     * @return const ImproperStyle* Pointer to the style, or nullptr if not found
     */
    const ImproperStyle* getImproperStyle(const std::string& name) const;

    /**
     * @brief Get improper style by name (non-const).
     * @param name Name of the improper style
     * @return ImproperStyle* Pointer to the style, or nullptr if not found
     */
    ImproperStyle* getImproperStyle(const std::string& name);

    /**
     * @brief Get pair style by name.
     * @param name Name of the pair style
     * @return const PairStyle* Pointer to the style, or nullptr if not found
     */
    const PairStyle* getPairStyle(const std::string& name) const;

    /**
     * @brief Get pair style by name (non-const).
     * @param name Name of the pair style
     * @return PairStyle* Pointer to the style, or nullptr if not found
     */
    PairStyle* getPairStyle(const std::string& name);

    // Type collection methods
    /**
     * @brief Get all atom types from all atom styles.
     * @return std::vector<const AtomType*> Vector of pointers to all atom types
     */
    std::vector<const AtomType*> get_atom_types() const;

    /**
     * @brief Get all bond types from all bond styles.
     * @return std::vector<const BondType*> Vector of pointers to all bond types
     */
    std::vector<const BondType*> get_bond_types() const;

    /**
     * @brief Get all angle types from all angle styles.
     * @return std::vector<const AngleType*> Vector of pointers to all angle types
     */
    std::vector<const AngleType*> getAngleTypes() const;

    /**
     * @brief Get all dihedral types from all dihedral styles.
     * @return std::vector<const DihedralType*> Vector of pointers to all dihedral types
     */
    std::vector<const DihedralType*> getDihedralTypes() const;

    /**
     * @brief Get all improper types from all improper styles.
     * @return std::vector<const ImproperType*> Vector of pointers to all improper types
     */
    std::vector<const ImproperType*> getImproperTypes() const;

    /**
     * @brief Get all pair types from all pair styles.
     * @return std::vector<const PairType*> Vector of pointers to all pair types
     */
    std::vector<const PairType*> getPairTypes() const;

    // Count methods
    /**
     * @brief Get number of atom styles.
     * @return size_t Number of atom styles
     */
    size_t getNumAtomStyles() const { return atom_styles_.size(); }

    /**
     * @brief Get number of bond styles.
     * @return size_t Number of bond styles
     */
    size_t getNumBondStyles() const { return bond_styles_.size(); }

    /**
     * @brief Get number of angle styles.
     * @return size_t Number of angle styles
     */
    size_t getNumAngleStyles() const { return angle_styles_.size(); }

    /**
     * @brief Get number of dihedral styles.
     * @return size_t Number of dihedral styles
     */
    size_t getNumDihedralStyles() const { return dihedral_styles_.size(); }

    /**
     * @brief Get number of improper styles.
     * @return size_t Number of improper styles
     */
    size_t getNumImproperStyles() const { return improper_styles_.size(); }

    /**
     * @brief Get number of pair styles.
     * @return size_t Number of pair styles
     */
    size_t getNumPairStyles() const { return pair_styles_.size(); }

    /**
     * @brief Get total number of atom types.
     * @return size_t Number of atom types
     */
    size_t getNumAtomTypes() const;

    /**
     * @brief Get total number of bond types.
     * @return size_t Number of bond types
     */
    size_t getNumBondTypes() const;

    /**
     * @brief Get total number of angle types.
     * @return size_t Number of angle types
     */
    size_t getNumAngleTypes() const;

    /**
     * @brief Get total number of dihedral types.
     * @return size_t Number of dihedral types
     */
    size_t getNumDihedralTypes() const;

    /**
     * @brief Get total number of improper types.
     * @return size_t Number of improper types
     */
    size_t getNumImproperTypes() const;

    /**
     * @brief Get total number of pair types.
     * @return size_t Number of pair types
     */
    size_t getNumPairTypes() const;

    /**
     * @brief Check if a style exists by name.
     * @param name Style name to check
     * @return true if any style with the given name exists
     */
    bool contains(const std::string& name) const;

    /**
     * @brief Get a style by name (searches all style types).
     * @param name Style name
     * @return const Style* Pointer to the style, or nullptr if not found
     */
    const Style* getStyle(const std::string& name) const;

    /**
     * @brief Get a style by name (searches all style types, non-const).
     * @param name Style name
     * @return Style* Pointer to the style, or nullptr if not found
     */
    Style* getStyle(const std::string& name);

    /**
     * @brief Get total number of styles.
     * @return size_t Total number of styles across all categories
     */
    size_t getTotalNumStyles() const;

    /**
     * @brief Merge another ForceField into this one.
     * @param other The other force field to merge
     * @return ForceField& Reference to this force field
     */
    ForceField& merge(const ForceField& other);

    /**
     * @brief String representation of the force field.
     * @return std::string String representation with statistics
     */
    std::string toString() const;

    // Serialization support (similar to Python version's to_dict/from_dict)
    /**
     * @brief Convert ForceField to a map for serialization.
     * @return std::unordered_map<std::string, ParamValue> Map representation
     */
    std::unordered_map<std::string, ParamValue> toDict() const;

    /**
     * @brief Create ForceField from a map representation.
     * @param dict Map containing forcefield data
     * @return ForceField Reconstructed ForceField instance
     */
    static ForceField fromDict(const std::unordered_map<std::string, ParamValue>& dict);

public:
    /**
     * @brief Registry for potential kernel implementations.
     * Similar to _kernel_registry in Python version.
     */
    static std::unordered_map<std::string, 
                              std::unordered_map<std::string, 
                                                 std::function<void*()>>> kernel_registry_;

private:
    std::string name_;   ///< Name of the force field
    std::string units_;  ///< Units system

    // Style containers
    std::vector<AtomStyle> atom_styles_;         ///< Atom styles
    std::vector<BondStyle> bond_styles_;         ///< Bond styles
    std::vector<AngleStyle> angle_styles_;       ///< Angle styles
    std::vector<DihedralStyle> dihedral_styles_; ///< Dihedral styles
    std::vector<ImproperStyle> improper_styles_; ///< Improper styles
    std::vector<PairStyle> pair_styles_;         ///< Pair styles

    // Helper methods for style lookup
    template<typename StyleType>
    const StyleType* find_style(const std::vector<StyleType>& styles, 
                                 const std::string& name) const;

    template<typename StyleType>
    StyleType* find_style(std::vector<StyleType>& styles, 
                          const std::string& name);
};

/**
 * @brief Metaclass for kernel registration (similar to KernelMeta in Python).
 * 
 * This provides a way to register potential implementations with the ForceField
 * for later use in creating potential instances from force field definitions.
 */
class KernelRegistry {
public:
    /**
     * @brief Register a kernel implementation.
     * @param type_name Type of potential (e.g., "pair", "bond", "angle")
     * @param kernel_name Name of the kernel implementation
     * @param factory Factory function to create instances
     */
    static void register_kernel(const std::string& type_name,
                                const std::string& kernel_name,
                                std::function<void*()> factory);

    /**
     * @brief Get registered kernel factory.
     * @param type_name Type of potential
     * @param kernel_name Name of the kernel implementation
     * @return std::function<void*()> Factory function, or nullptr if not found
     */
    static std::function<void*()> get_kernel_factory(const std::string& type_name,
                                                     const std::string& kernel_name);

    /**
     * @brief Check if a kernel is registered.
     * @param type_name Type of potential
     * @param kernel_name Name of the kernel implementation
     * @return true if kernel is registered
     */
    static bool has_kernel(const std::string& type_name,
                           const std::string& kernel_name);
};

/**
 * @brief Version information for the force field system.
 */
constexpr const char* FORCEFIELD_VERSION = "1.0.0";

/**
 * @brief Supported unit systems.
 */
constexpr const char* SUPPORTED_UNITS[] = {
    "real",      // LAMMPS real units
    "metal",     // LAMMPS metal units  
    "si",        // SI units
    "cgs",       // CGS units
    "electron",  // LAMMPS electron units
    "micro",     // LAMMPS micro units
    "nano"       // LAMMPS nano units
};

/**
 * @brief Check if a units system is supported.
 * @param units Units system string to check
 * @return true if the units system is supported
 */
inline bool isSupportedUnits(const std::string& units) {
    for (const auto& supported : SUPPORTED_UNITS) {
        if (units == supported) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Helper function to create a simple Lennard-Jones force field.
 * @param name Name for the force field
 * @return ForceField A basic LJ force field setup
 */
ForceField createLjForcefield(const std::string& name = "LJ");

/**
 * @brief Helper function to create a basic molecular force field with bonds and angles.
 * @param name Name for the force field
 * @return ForceField A molecular force field setup
 */
ForceField createMolecularForcefield(const std::string& name = "Molecular");

} // namespace molcpp

// Hash specialization for Type
namespace std {
    template<>
    struct hash<molcpp::Type> {
        size_t operator()(const molcpp::Type& type) const {
            return type.hash();
        }
    };

    template<>
    struct hash<molcpp::Style> {
        size_t operator()(const molcpp::Style& style) const {
            return style.hash();
        }
    };
}
