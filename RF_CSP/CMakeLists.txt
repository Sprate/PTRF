cmake_minimum_required(VERSION 3.15)
project(RF_CSP)

set(CMAKE_CXX_STANDARD 14)

find_package(Threads REQUIRED)
find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)

find_package(emp-ot REQUIRED)
find_package(emp-tool REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${EMP-OT_INCLUDE_DIRS})

# Installation
install(FILES cmake/emp-ot-config.cmake DESTINATION cmake/)
install(FILES cmake/emp-sh2pc-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-ot DESTINATION include/)
install(DIRECTORY emp-sh2pc DESTINATION include/)

add_executable(RF_CSP main.cpp
        CSP.cpp
        CSP.h
        Paillier/Paillier.h
        Paillier/Paillier.cpp
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/bob_eva.h  emp-sh2pc/bob_eva.cpp)
add_executable(RF_CSP_2 main2.cpp
        CSP.cpp
        CSP.h
        Paillier/Paillier.h
        Paillier/Paillier.cpp
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/bob_eva.h  emp-sh2pc/bob_eva.cpp)
add_executable(RF_CSP_train PrivateTrain.cpp
        CSP.cpp
        CSP.h
        Paillier/Paillier.h
        Paillier/Paillier.cpp
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/bob_eva.h  emp-sh2pc/bob_eva.cpp)
add_executable(RF_CSP_test test.cpp
        CSP.cpp
        CSP.h
        Paillier/Paillier.h
        Paillier/Paillier.cpp
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/bob_eva.h  emp-sh2pc/bob_eva.cpp
        )
add_executable(RF_CSP_sub_test sub_test.cpp
        CSP.cpp
        CSP.h
        Paillier/Paillier.h
        Paillier/Paillier.cpp
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/bob_eva.h  emp-sh2pc/bob_eva.cpp)
target_link_libraries(RF_CSP Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_CSP_2 Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_CSP_train Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_CSP_test Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_CSP_sub_test Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
