set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)
include_directories(../../third_party/rapidjson/include)
set(SOURCE_FILES main.cpp Vocab.cpp DepSkgNeg.cpp DepTree.cpp)
LINK_LIBRARIES('m')
add_executable(Dep2vec ${SOURCE_FILES})

target_link_libraries(Dep2vec pthread)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O2 -Wall -funroll-loops")        ## Optimize
endif()