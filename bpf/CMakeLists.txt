project(bpfexample)



if (NOT CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(FATAL_ERROR "ONLY support in linux")
    
endif()


add_executable(example example.c)
