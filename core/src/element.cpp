#include "molcpp/core/element.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace molcpp {

// =============================================================================
// Static member initialization
// =============================================================================

std::map<std::string, ElementData> Element::_elements;
std::map<std::string, ElementData> Element::_symbol_to_element;
std::map<int, ElementData> Element::_number_to_element;

// =============================================================================
// Helper methods
// =============================================================================

std::string Element::to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void Element::populate_dictionaries() {
    // Define all elements data (same as Python version)
    std::vector<ElementData> elements_data = {
        // Special element for unknown atoms
        ElementData(0, "unknown", "X", 0.0 * daltons),
        ElementData(1, "hydrogen", "H", 1.007947 * daltons),
        ElementData(2, "helium", "He", 4.003 * daltons),
        ElementData(3, "lithium", "Li", 6.9412 * daltons),
        ElementData(4, "beryllium", "Be", 9.0121823 * daltons),
        ElementData(5, "boron", "B", 10.8117 * daltons),
        ElementData(6, "carbon", "C", 12.01078 * daltons),
        ElementData(7, "nitrogen", "N", 14.00672 * daltons),
        ElementData(8, "oxygen", "O", 15.99943 * daltons),
        ElementData(9, "fluorine", "F", 18.99840325 * daltons),
        ElementData(10, "neon", "Ne", 20.17976 * daltons),
        ElementData(11, "sodium", "Na", 22.989769282 * daltons),
        ElementData(12, "magnesium", "Mg", 24.30506 * daltons),
        ElementData(13, "aluminum", "Al", 26.98153868 * daltons),
        ElementData(14, "silicon", "Si", 28.08553 * daltons),
        ElementData(15, "phosphorus", "P", 30.9737622 * daltons),
        ElementData(16, "sulfur", "S", 32.0655 * daltons),
        ElementData(17, "chlorine", "Cl", 35.4532 * daltons),
        ElementData(18, "argon", "Ar", 39.9481 * daltons),
        ElementData(19, "potassium", "K", 39.09831 * daltons),
        ElementData(20, "calcium", "Ca", 40.0784 * daltons),
        ElementData(21, "scandium", "Sc", 44.9559126 * daltons),
        ElementData(22, "titanium", "Ti", 47.8671 * daltons),
        ElementData(23, "vanadium", "V", 50.94151 * daltons),
        ElementData(24, "chromium", "Cr", 51.99616 * daltons),
        ElementData(25, "manganese", "Mn", 54.9380455 * daltons),
        ElementData(26, "iron", "Fe", 55.8452 * daltons),
        ElementData(27, "cobalt", "Co", 58.9331955 * daltons),
        ElementData(28, "nickel", "Ni", 58.69342 * daltons),
        ElementData(29, "copper", "Cu", 63.5463 * daltons),
        ElementData(30, "zinc", "Zn", 65.4094 * daltons),
        ElementData(31, "gallium", "Ga", 69.7231 * daltons),
        ElementData(32, "germanium", "Ge", 72.641 * daltons),
        ElementData(33, "arsenic", "As", 74.921602 * daltons),
        ElementData(34, "selenium", "Se", 78.963 * daltons),
        ElementData(35, "bromine", "Br", 79.9041 * daltons),
        ElementData(36, "krypton", "Kr", 83.7982 * daltons),
        ElementData(37, "rubidium", "Rb", 85.46783 * daltons),
        ElementData(38, "strontium", "Sr", 87.621 * daltons),
        ElementData(39, "yttrium", "Y", 88.905852 * daltons),
        ElementData(40, "zirconium", "Zr", 91.2242 * daltons),
        ElementData(41, "niobium", "Nb", 92.906382 * daltons),
        ElementData(42, "molybdenum", "Mo", 95.942 * daltons),
        ElementData(43, "technetium", "Tc", 98 * daltons),
        ElementData(44, "ruthenium", "Ru", 101.072 * daltons),
        ElementData(45, "rhodium", "Rh", 102.905502 * daltons),
        ElementData(46, "palladium", "Pd", 106.421 * daltons),
        ElementData(47, "silver", "Ag", 107.86822 * daltons),
        ElementData(48, "cadmium", "Cd", 112.4118 * daltons),
        ElementData(49, "indium", "In", 114.8183 * daltons),
        ElementData(50, "tin", "Sn", 118.7107 * daltons),
        ElementData(51, "antimony", "Sb", 121.7601 * daltons),
        ElementData(52, "tellurium", "Te", 127.603 * daltons),
        ElementData(53, "iodine", "I", 126.904473 * daltons),
        ElementData(54, "xenon", "Xe", 131.2936 * daltons),
        ElementData(55, "cesium", "Cs", 132.90545192 * daltons),
        ElementData(56, "barium", "Ba", 137.3277 * daltons),
        ElementData(57, "lanthanum", "La", 138.905477 * daltons),
        ElementData(58, "cerium", "Ce", 140.1161 * daltons),
        ElementData(59, "praseodymium", "Pr", 140.907652 * daltons),
        ElementData(60, "neodymium", "Nd", 144.2423 * daltons),
        ElementData(61, "promethium", "Pm", 145 * daltons),
        ElementData(62, "samarium", "Sm", 150.362 * daltons),
        ElementData(63, "europium", "Eu", 151.9641 * daltons),
        ElementData(64, "gadolinium", "Gd", 157.253 * daltons),
        ElementData(65, "terbium", "Tb", 158.925352 * daltons),
        ElementData(66, "dysprosium", "Dy", 162.5001 * daltons),
        ElementData(67, "holmium", "Ho", 164.930322 * daltons),
        ElementData(68, "erbium", "Er", 167.2593 * daltons),
        ElementData(69, "thulium", "Tm", 168.934212 * daltons),
        ElementData(70, "ytterbium", "Yb", 173.043 * daltons),
        ElementData(71, "lutetium", "Lu", 174.9671 * daltons),
        ElementData(72, "hafnium", "Hf", 178.492 * daltons),
        ElementData(73, "tantalum", "Ta", 180.947882 * daltons),
        ElementData(74, "tungsten", "W", 183.841 * daltons),
        ElementData(75, "rhenium", "Re", 186.2071 * daltons),
        ElementData(76, "osmium", "Os", 190.233 * daltons),
        ElementData(77, "iridium", "Ir", 192.2173 * daltons),
        ElementData(78, "platinum", "Pt", 195.0849 * daltons),
        ElementData(79, "gold", "Au", 196.9665694 * daltons),
        ElementData(80, "mercury", "Hg", 200.592 * daltons),
        ElementData(81, "thallium", "Tl", 204.38332 * daltons),
        ElementData(82, "lead", "Pb", 207.21 * daltons),
        ElementData(83, "bismuth", "Bi", 208.980401 * daltons),
        ElementData(84, "polonium", "Po", 209 * daltons),
        ElementData(85, "astatine", "At", 210 * daltons),
        ElementData(86, "radon", "Rn", 222.018 * daltons),
        ElementData(87, "francium", "Fr", 223 * daltons),
        ElementData(88, "radium", "Ra", 226 * daltons),
        ElementData(89, "actinium", "Ac", 227 * daltons),
        ElementData(90, "thorium", "Th", 232.038062 * daltons),
        ElementData(91, "protactinium", "Pa", 231.035882 * daltons),
        ElementData(92, "uranium", "U", 238.028913 * daltons),
        ElementData(93, "neptunium", "Np", 237 * daltons),
        ElementData(94, "plutonium", "Pu", 244 * daltons),
        ElementData(95, "americium", "Am", 243 * daltons),
        ElementData(96, "curium", "Cm", 247 * daltons),
        ElementData(97, "berkelium", "Bk", 247 * daltons),
        ElementData(98, "californium", "Cf", 251 * daltons),
        ElementData(99, "einsteinium", "Es", 252 * daltons),
        ElementData(100, "fermium", "Fm", 257 * daltons),
        ElementData(101, "mendelevium", "Md", 258 * daltons),
        ElementData(102, "nobelium", "No", 259 * daltons),
        ElementData(103, "lawrencium", "Lr", 262 * daltons),
        ElementData(104, "rutherfordium", "Rf", 261 * daltons),
        ElementData(105, "dubnium", "Db", 262 * daltons),
        ElementData(106, "seaborgium", "Sg", 266 * daltons),
        ElementData(107, "bohrium", "Bh", 264 * daltons),
        ElementData(108, "hassium", "Hs", 269 * daltons),
        ElementData(109, "meitnerium", "Mt", 268 * daltons),
        ElementData(110, "darmstadtium", "Ds", 281 * daltons),
        ElementData(111, "roentgenium", "Rg", 272 * daltons),
        ElementData(112, "ununbium", "Uub", 285 * daltons),
        ElementData(113, "ununtrium", "Uut", 284 * daltons),
        ElementData(114, "ununquadium", "Uuq", 289 * daltons),
        ElementData(115, "ununpentium", "Uup", 288 * daltons),
        ElementData(116, "ununhexium", "Uuh", 292 * daltons),
    };
    
    // Clear existing dictionaries
    _elements.clear();
    _symbol_to_element.clear();
    _number_to_element.clear();
    
    // Populate dictionaries
    for (const auto& element : elements_data) {
        _elements[element.name] = element;
        _symbol_to_element[element.symbol] = element;
        _number_to_element[element.number] = element;
    }
}

// =============================================================================
// Public methods
// =============================================================================

void Element::initialize() {
    populate_dictionaries();
}

ElementData Element::create(const std::string& name_or_symbol) {
    // Handle atomic number 0 for unknown elements
    if (name_or_symbol == "0" || name_or_symbol == "X") {
        return ElementData(0, "unknown", "X", 0.0);
    }
    
    // Try name first (case-insensitive)
    std::string name_lower = to_lower(name_or_symbol);
    for (const auto& [element_name, element_data] : _elements) {
        if (to_lower(element_name) == name_lower) {
            return element_data;
        }
    }
    
    // Try symbol (case-insensitive)
    for (const auto& [symbol, element_data] : _symbol_to_element) {
        if (to_lower(symbol) == name_lower) {
            return element_data;
        }
    }
    
    throw std::runtime_error("Element not found: " + name_or_symbol);
}

ElementData Element::create(int atomic_number) {
    // Handle atomic number 0 for unknown elements
    if (atomic_number == 0) {
        return ElementData(0, "unknown", "X", 0.0);
    }
    
    auto it = _number_to_element.find(atomic_number);
    if (it != _number_to_element.end()) {
        return it->second;
    }
    
    throw std::runtime_error("Element not found: " + std::to_string(atomic_number));
}

std::vector<std::string> Element::get_symbols(const std::vector<std::string>& identifiers) {
    std::vector<std::string> symbols;
    symbols.reserve(identifiers.size());
    
    for (const auto& identifier : identifiers) {
        try {
            auto element = create(identifier);
            symbols.push_back(element.symbol);
        } catch (const std::exception&) {
            symbols.push_back("X"); // Unknown element
        }
    }
    
    return symbols;
}

std::vector<std::string> Element::get_symbols(const std::vector<int>& atomic_numbers) {
    std::vector<std::string> symbols;
    symbols.reserve(atomic_numbers.size());
    
    for (int atomic_number : atomic_numbers) {
        try {
            auto element = create(atomic_number);
            symbols.push_back(element.symbol);
        } catch (const std::exception&) {
            symbols.push_back("X"); // Unknown element
        }
    }
    
    return symbols;
}

int Element::get_atomic_number(const std::string& symbol) {
    auto it = _symbol_to_element.find(symbol);
    if (it != _symbol_to_element.end()) {
        return it->second.number;
    }
    
    // Try case-insensitive search
    std::string symbol_lower = to_lower(symbol);
    for (const auto& [sym, element_data] : _symbol_to_element) {
        if (to_lower(sym) == symbol_lower) {
            return element_data.number;
        }
    }
    
    throw std::runtime_error("Element with symbol '" + symbol + "' not found");
}

bool Element::exists(const std::string& identifier) {
    try {
        create(identifier);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool Element::exists(int atomic_number) {
    try {
        create(atomic_number);
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<ElementData> Element::get_all_elements() {
    std::vector<ElementData> elements;
    elements.reserve(_elements.size());
    
    for (const auto& [name, element_data] : _elements) {
        elements.push_back(element_data);
    }
    
    return elements;
}

size_t Element::count() {
    return _elements.size();
}

} // namespace molcpp::core
