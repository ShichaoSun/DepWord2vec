set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)
include_directories(../../third_party/rapidjson/include)
set(SOURCE_FILES main.cpp Word.cpp Vocab.cpp SkgNeg.cpp)
set(EXECUTABLE_OUTPUT_PATH ../../bin)
LINK_LIBRARIES('m')
add_executable(Word2vec ${SOURCE_FILES})

target_link_libraries(Word2vec pthread)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O2 -Wall -funroll-loops")        ## Optimize
endif()