cmake_minimum_required(VERSION 3.13)

project(codecrafters-shell)

file(GLOB_RECURSE SOURCE_FILES src/*.c src/*.h)

set(CMAKE_C_STANDARD 23) # Enable the C23 standard

# Emit compile_commands.json so clangd/editors can find the right headers.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(shell ${SOURCE_FILES})

# Link readline when it is available (e.g. on Linux/CodeCrafters). It is not
# used by the current source yet and is not present on a stock Windows setup,
# so treat it as optional to keep local Windows builds working.
find_library(READLINE_LIBRARY NAMES readline)
if(READLINE_LIBRARY)
  target_link_libraries(shell PRIVATE ${READLINE_LIBRARY})
endif()
