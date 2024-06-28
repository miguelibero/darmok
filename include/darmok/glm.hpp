#include <darmok/export.h>

// this header adds dllexport to the glm classes
// it should be included before any glm include
// to guarantee that dynamic linking works

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