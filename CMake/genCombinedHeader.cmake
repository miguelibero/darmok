
set(CONTENT "// automatically generated by darmok")
foreach(INCLUDE ${INCLUDES})
    set(CONTENT "${CONTENT}\n#include \"${INCLUDE}\"")
endforeach()
file(WRITE "${OUTPUT}" "${CONTENT}")