#include <darmok/export.h>

#ifdef DARMOK_STATIC_DEFINE
#  define GLM_API
#else
#  ifdef darmok_EXPORTS
#    define GLM_API __declspec(dllexport)
#  else
#    define GLM_API __declspec(dllimport)
#  endif
#endif

#include <glm/glm.hpp>