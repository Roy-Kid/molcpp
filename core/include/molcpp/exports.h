
#ifndef MOLCPP_EXPORT_H
#define MOLCPP_EXPORT_H

#ifdef MOLCPP_STATIC_DEFINE
#  define MOLCPP_EXPORT
#  define MOLCPP_NO_EXPORT
#else
#  ifndef MOLCPP_EXPORT
#    ifdef molcpp_EXPORTS
        /* We are building this library */
#      define MOLCPP_EXPORT 
#    else
        /* We are using this library */
#      define MOLCPP_EXPORT 
#    endif
#  endif

#  ifndef MOLCPP_NO_EXPORT
#    define MOLCPP_NO_EXPORT 
#  endif
#endif

#ifndef MOLCPP_DEPRECATED
#  define MOLCPP_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef MOLCPP_DEPRECATED_EXPORT
#  define MOLCPP_DEPRECATED_EXPORT MOLCPP_EXPORT MOLCPP_DEPRECATED
#endif

#ifndef MOLCPP_DEPRECATED_NO_EXPORT
#  define MOLCPP_DEPRECATED_NO_EXPORT MOLCPP_NO_EXPORT MOLCPP_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef MOLCPP_NO_DEPRECATED
#    define MOLCPP_NO_DEPRECATED
#  endif
#endif

#endif /* MOLCPP_EXPORT_H */
