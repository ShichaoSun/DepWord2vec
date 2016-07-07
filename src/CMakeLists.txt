set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Word.cpp Vocab.cpp SkgNeg.cpp)
LINK_LIBRARIES('m')
add_executable(DepWord2vec ${SOURCE_FILES})

target_link_libraries(DepWord2vec pthread)

## Compiler flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O2 -funroll-loops")        ## Optimize
endif()