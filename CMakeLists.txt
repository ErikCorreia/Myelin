cmake_minimum_required(VERSION 3.18)
project(Myelin LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

set(IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/ConfigLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/InstructionLoader.cpp
)

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/LlamaEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ChatHistory.cpp
)

set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)

# 2. Adiciona o subdiretório (precisa clonar o llama.cpp) 
add_subdirectory(llama.cpp)

add_executable(
    Myelin 
    src/main.cpp
    ${IO_SOURCES}
    ${CORE_SOURCES}
)

target_include_directories(Myelin PRIVATE 
    llama.cpp/include
    llama.cpp/common
    "includes"
    "src/core"
    "src/io"
)

target_link_libraries(Myelin PRIVATE llama)

if(UNIX)
    target_link_libraries(Myelin PRIVATE m pthread dl)
endif()