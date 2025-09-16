#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <memory>

namespace molcpp {

// Dalton unit constant
constexpr double daltons = 1.0;

/**
 * @brief Data class representing the properties of a chemical element.
 */
struct ElementData {
    int number;           // Atomic number
    std::string name;     // Element name
    std::string symbol;   // Element symbol
    double mass;          // Atomic mass in daltons
    
    ElementData(int num, const std::string& n, const std::string& sym, double m)
        : number(num), name(n), symbol(sym), mass(m) {}
    
    // Default constructor
    ElementData() : number(0), name(""), symbol(""), mass(0.0) {}
    
    // Copy constructor
    ElementData(const ElementData& other) = default;
    
    // Move constructor
    ElementData(ElementData&& other) noexcept = default;
    
    // Assignment operators
    ElementData& operator=(const ElementData& other) = default;
    ElementData& operator=(ElementData&& other) noexcept = default;
    
    // Destructor
    ~ElementData() = default;
};

/**
 * @brief The Element class represents chemical elements and provides functionality 
 * to retrieve element information by name, symbol, or atomic number.
 * 
 * This class acts as a factory that returns ElementData instances based on the input
 * identifier (name, symbol, or atomic number).
 */
class Element {
public:
    // Static element databases
    static std::map<std::string, ElementData> _elements;
    static std::map<std::string, ElementData> _symbol_to_element;
    static std::map<int, ElementData> _number_to_element;
    
    // Factory method - returns ElementData based on identifier
    static ElementData create(const std::string& name_or_symbol);
    static ElementData create(int atomic_number);
    
    // Utility methods
    static std::vector<std::string> get_symbols(const std::vector<std::string>& identifiers);
    static std::vector<std::string> get_symbols(const std::vector<int>& atomic_numbers);
    static int get_atomic_number(const std::string& symbol);
    
    // Initialization
    static void initialize();
    
    // Check if element exists
    static bool exists(const std::string& identifier);
    static bool exists(int atomic_number);
    
    // Get all available elements
    static std::vector<ElementData> get_all_elements();
    
    // Get element count
    static size_t count();

private:
    // Helper methods
    static std::string to_lower(const std::string& str);
    static void populate_dictionaries();
};

} // namespace molcpp
