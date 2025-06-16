#ifndef MOLCPP_EXPORT_HPP
#define MOLCPP_EXPORT_HPP

// Export macros for molcpp library
#ifdef molcpp_EXPORTS
    #define MOLCPP_EXPORT __attribute__((visibility("default")))
#else
    #define MOLCPP_EXPORT
#endif

#endif // MOLCPP_EXPORT_HPP
