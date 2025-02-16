# Find all source files in a single current directory
# Save the name to DIR_examples_SRCS
aux_source_directory(. DIR_picoDisplay_SRCS)

include_directories(../lib/Config)
include_directories(../lib/GUI)
include_directories(../lib/e-Paper)

# Generate the link library
add_library(picoDisplay ${DIR_picoDisplay_SRCS})
target_link_libraries(picoDisplay PUBLIC Config hardware_timer)