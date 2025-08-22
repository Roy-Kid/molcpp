#include <iostream>
#include <iomanip>
#include "molcpp/types.hpp"
#include "molcpp/pack/packer.hpp"

int main() {
    std::cout << "=== MolCpp Type System Demo ===" << std::endl;
    
    // Show current precision setting
    #ifdef MOLCPP_USE_DOUBLE
        std::cout << "Using double precision" << std::endl;
    #else
        std::cout << "Using single precision (float)" << std::endl;
    #endif
    
    // Demonstrate template types with default precision
    std::cout << "\n--- Default Precision Types ---" << std::endl;
    
    // Create a 3D vector using default precision
    Vec3<> default_vec{1.0, 2.0, 3.0};
    std::cout << "Default Vec3: [" << default_vec(0) << ", " << default_vec(1) << ", " << default_vec(2) << "]" << std::endl;
    std::cout << "Type: " << typeid(default_vec).name() << std::endl;
    
    // Create coordinate array using default precision
    CoordArray<> coords = xt::zeros<double>({5, 3});
    coords(0, 0) = 1.0; coords(0, 1) = 2.0; coords(0, 2) = 3.0;
    std::cout << "Default CoordArray shape: [" << coords.shape(0) << ", " << coords.shape(1) << "]" << std::endl;
    std::cout << "Type: " << typeid(coords).name() << std::endl;
    
    // Demonstrate explicit precision types
    std::cout << "\n--- Explicit Precision Types ---" << std::endl;
    
    // Single precision
    Vec3<float> float_vec{1.0f, 2.0f, 3.0f};
    CoordArray<float> float_coords = xt::zeros<float>({3, 3});
    std::cout << "Float Vec3: [" << float_vec(0) << ", " << float_vec(1) << ", " << float_vec(2) << "]" << std::endl;
    std::cout << "Float CoordArray shape: [" << float_coords.shape(0) << ", " << float_coords.shape(1) << "]" << std::endl;
    
    // Double precision
    Vec3<double> double_vec{1.0, 2.0, 3.0};
    CoordArray<double> double_coords = xt::zeros<double>({3, 3});
    std::cout << "Double Vec3: [" << double_vec(0) << ", " << double_vec(1) << ", " << double_vec(2) << "]" << std::endl;
    std::cout << "Double CoordArray shape: [" << double_coords.shape(0) << ", " << double_coords.shape(1) << "]" << std::endl;
    
    // Demonstrate type conversion capabilities
    std::cout << "\n--- Type Conversion Demo ---" << std::endl;
    
    // Convert float to double
    CoordArray<double> converted_coords = xt::cast<double>(float_coords);
    std::cout << "Converted float->double: [" << converted_coords.shape(0) << ", " << converted_coords.shape(1) << "]" << std::endl;
    
    // Convert double to float
    CoordArray<float> converted_back = xt::cast<float>(double_coords);
    std::cout << "Converted double->float: [" << converted_back.shape(0) << ", " << converted_back.shape(1) << "]" << std::endl;
    
    // Demonstrate packer with different precisions
    std::cout << "\n--- Packer Type Demo ---" << std::endl;
    
    // Default precision packer
    molcpp::pack::MolPacker<> default_packer;
    std::cout << "Default MolPacker created successfully" << std::endl;
    
    // Explicit precision packers
    molcpp::pack::MolPacker<float> float_packer;
    molcpp::pack::MolPacker<double> double_packer;
    std::cout << "Float and double MolPackers created successfully" << std::endl;
    
    // Show type aliases
    std::cout << "\n--- Type Aliases ---" << std::endl;
    std::cout << "MolPackerf (float): " << typeid(molcpp::pack::MolPackerf).name() << std::endl;
    std::cout << "MolPackerd (double): " << typeid(molcpp::pack::MolPackerd).name() << std::endl;
    std::cout << "DefaultMolPacker: " << typeid(molcpp::pack::DefaultMolPacker).name() << std::endl;
    
    std::cout << "\n=== Demo Complete ===" << std::endl;
    return 0;
}
