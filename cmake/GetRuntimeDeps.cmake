message (STATUS "$CMAKE_ARGV0 = ${CMAKE_ARGV0}")
message (STATUS "$CMAKE_ARGV1 = ${CMAKE_ARGV1}")
message (STATUS "$CMAKE_ARGV2 = ${CMAKE_ARGV2}")
message (STATUS "$CMAKE_ARGV3 = ${CMAKE_ARGV3}")

file (GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
    UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
    EXECUTABLES "${CMAKE_ARGV3}"
)
list (SORT RESOLVED_DEPENDENCIES)
list (SORT UNRESOLVED_DEPENDENCIES)
foreach (DEP IN LISTS RESOLVED_DEPENDENCIES UNRESOLVED_DEPENDENCIES)
    message (STATUS "${CMAKE_ARGV3} -> ${DEP}")
endforeach ()
