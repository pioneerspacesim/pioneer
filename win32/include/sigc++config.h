/* sigc++config.h.  Generated from sigc++config.h.in by configure.  */

/* Major version number of sigc++. */
#define SIGCXX_MAJOR_VERSION 2

/* Micro version number of sigc++. */
#define SIGCXX_MICRO_VERSION 8

/* Minor version number of sigc++. */
#define SIGCXX_MINOR_VERSION 2

/* Detect Win32 platform */
#ifdef _WIN32
# if defined(_MSC_VER)
#  define SIGC_MSC 1
#  define SIGC_WIN32 1
#  define SIGC_DLL 1
# elif defined(__CYGWIN__)
#  define SIGC_CONFIGURE 1
# elif defined(__MINGW32__)
#  define SIGC_WIN32 1
#  define SIGC_CONFIGURE 1
# else
#  error "libsigc++ config: Unknown win32 architecture (send me gcc --dumpspecs or equiv)"
# endif
#else /* !_WIN32 */
# define SIGC_CONFIGURE 1
#endif /* !_WIN32 */

#ifdef SIGC_MSC
/*
 * MS VC7 Warning 4251 says that the classes to any member objects in an
 * exported class must be also be exported.  Some of the libsigc++
 * template classes contain std::list members.  MS KB article 168958 says
 * that it's not possible to export a std::list instantiation due to some
 * wacky class nesting issues, so our only options are to ignore the
 * warning or to modify libsigc++ to remove the std::list dependency.
 * AFAICT, the std::list members are used internally by the library code
 * and don't need to be used from the outside, and ignoring the warning
 * seems to have no adverse effects, so that seems like a good enough
 * solution for now.
 */
# pragma warning(disable:4251)

# define SIGC_MSVC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD 1
# define SIGC_NEW_DELETE_IN_LIBRARY_ONLY 1 /* To keep ABI compatibility */
# define SIGC_HAVE_NAMESPACE_STD 1

#else /* SIGC_MSC */

/* does the C++ compiler support the use of a particular specialization when
   calling operator() template methods. */
# define SIGC_GCC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD 1

/* Defined when the libstdc++ declares the std-namespace */
# define SIGC_HAVE_NAMESPACE_STD 1

/* Define if the non-standard Sun reverse_iterator must be used. */
/* # undef SIGC_HAVE_SUN_REVERSE_ITERATOR */

/* does the C++ compiler support the use of a particular specialization when
   calling operator() template methods omitting the template keyword. */
# define SIGC_MSVC_TEMPLATE_SPECIALIZATION_OPERATOR_OVERLOAD 1

/* does c++ compiler allows usage of member function in initialization of
   static member field. */
# define SIGC_SELF_REFERENCE_IN_MEMBER_INITIALIZATION 1

#endif /* !SIGC_MSC */

#ifdef SIGC_HAVE_NAMESPACE_STD
# define SIGC_USING_STD(Symbol) /* empty */
#else
# define SIGC_USING_STD(Symbol) namespace std { using ::Symbol; }
#endif

#ifdef SIGC_DLL
# if defined(SIGC_BUILD) && defined(_WINDLL)
#  define SIGC_API __declspec(dllexport)
# elif !defined(SIGC_BUILD)
#  define SIGC_API __declspec(dllimport)
# else
#  define SIGC_API
# endif
#else /* !SIGC_DLL */
# define SIGC_API
#endif /* !SIGC_DLL */
