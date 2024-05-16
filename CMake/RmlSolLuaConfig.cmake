include(FetchContent)
FetchContent_Declare(RmlSolLua
    GIT_REPOSITORY https://github.com/miguelibero/RmlSolLua.git
    GIT_TAG fix/cmake-fetchcontent
)

FetchContent_MakeAvailable(RmlSolLua)

set(EXPORT_NAME "RmlSolLuaTargets")
install(TARGETS RmlSolLua EXPORT ${EXPORT_NAME})
install(EXPORT ${EXPORT_NAME}
  NAMESPACE Rml::SolLua
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RmlSolLua"
)