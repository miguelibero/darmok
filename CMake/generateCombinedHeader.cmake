
set(CONTENT "// automatically generated by darmok")
foreach(HEADER_INCLUDE ${HEADER_INCLUDES})
    set(CONTENT "${CONTENT}\n#include \"${HEADER_INCLUDE}\"")
endforeach()
file(WRITE "${OUTPUT}" "${CONTENT}")