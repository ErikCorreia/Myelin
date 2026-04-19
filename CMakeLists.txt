cmake_minimum_required(VERSION 3.18)
project(Myelin LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Lista de Dependencias
include(FetchContent)
FetchContent_declare(json GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FetchContent_declare(llama GIT_REPOSITORY https://github.com/ggml-org/llama.cpp.git GIT_TAG master)
FetchContent_declare(httplib GIT_REPOSITORY https://github.com/yhirose/cpp-httplib GIT_TAG master)

FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(llama)
FetchContent_MakeAvailable(httplib)

set(IO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/Logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/ConfigLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/InstructionLoader.cpp
)

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MyelinEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/LlamaProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MemoryArchivist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/ContextManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/MemoryEvaluator.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/EmbeddingEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/DatabaseManager.cpp
)

# set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# set(LLAMA_BUILD_SERVER OFF CACHE BOOL "" FORCE)

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
    "src/utils"
    "src/core"
    "src/io"
)

target_link_libraries(Myelin PRIVATE 
    llama
    httplib
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    OpenSSL::SSL 
    OpenSSL::Crypto
)

if(UNIX)
    target_link_libraries(Myelin PRIVATE m pthread dl)
endif()