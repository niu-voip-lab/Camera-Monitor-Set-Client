add_library(recordLib recordLib.cpp)

find_package(ALSA REQUIRED)                                    

if (ALSA_FOUND)                                                
    include_directories(${ALSA_INCLUDE_DIRS})                  
    target_link_libraries (recordLib ${ALSA_LIBRARIES})      
endif(ALSA_FOUND)  
