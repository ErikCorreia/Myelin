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

# 1. Desativar o que não precisamos do llama.cpp para o build ser rápido
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)

# 2. Adicionar o subdiretório (Certifique-se que a pasta llama.cpp está aqui)
add_subdirectory(llama.cpp)

# 3. Nosso executável (ainda o Hello World por enquanto)
add_executable(
    Myelin 
    src/main.cpp
    ${IO_SOURCES}
    ${CORE_SOURCES}
)

# 4. Incluir os headers para que você possa usar o llama.h depois
target_include_directories(Myelin PRIVATE 
    llama.cpp/include
    llama.cpp/common
    "includes"
    "src/core"
    "src/io"
)

# 5. Linkar apenas a biblioteca principal
# O alvo 'llama' é bem definido. O 'common' é o que estava dando erro.
target_link_libraries(Myelin PRIVATE llama)

if(UNIX)
    target_link_libraries(Myelin PRIVATE m pthread dl)
endif()