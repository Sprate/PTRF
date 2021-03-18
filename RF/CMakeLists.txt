cmake_minimum_required(VERSION 3.15)
project(RF)

set(CMAKE_CXX_STANDARD 14)

add_executable(RF_SP main.cpp
        RandomForest/Node.cpp
        RandomForest/Tree.h
        RandomForest/Tree.cpp
        RandomForest/RandomForest.h
        RandomForest/RandomForest.cpp
        RandomForest/Node.h
        Paillier/Paillier.h Paillier/Paillier.cpp
        SP.cpp SP.h CSP.cpp CSP.h
        test/test.h
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/alice_gen.h RandomForest/ReadForest.h util.h RandomForest/ReadTestSample.h   UserBehavior/touchPoint.cpp UserBehavior/touchPoint.h emp-sh2pc/alice_sort.h emp-sh2pc/alice_gen.cpp)
add_executable(RF_SP_2 main2.cpp
        RandomForest/Node.cpp
        RandomForest/Tree.h
        RandomForest/Tree.cpp
        RandomForest/RandomForest.h
        RandomForest/RandomForest.cpp
        RandomForest/Node.h
        Paillier/Paillier.h Paillier/Paillier.cpp
        SP.cpp SP.h CSP.cpp CSP.h
        test/test.h
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/alice_gen.h RandomForest/ReadForest.h util.h RandomForest/ReadTestSample.h   UserBehavior/touchPoint.cpp UserBehavior/touchPoint.h emp-sh2pc/alice_sort.h emp-sh2pc/alice_gen.cpp)
add_executable(RF_SP_train PrivateTrain.cpp
        PrivateRandomForest/PrivateNode.cpp
        PrivateRandomForest/PrivateTree.h
        PrivateRandomForest/PrivateTree.cpp
        PrivateRandomForest/PrivateRandomForest.h
        PrivateRandomForest/PrivateRandomForest.cpp
        PrivateRandomForest/PrivateNode.h
        Paillier/Paillier.h Paillier/Paillier.cpp
        SP.cpp SP.h CSP.cpp CSP.h
        test/test.h
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/alice_gen.h  util.h    UserBehavior/touchPoint.cpp UserBehavior/touchPoint.h emp-sh2pc/alice_sort.h emp-sh2pc/alice_gen.cpp)
add_executable(RF_SP_sub_test sub_protocol_test.cpp
        Paillier/Paillier.h Paillier/Paillier.cpp
        SP.cpp SP.h CSP.cpp CSP.h
        emp-ot/.
        emp-sh2pc/. emp-sh2pc/alice_gen.h  util.h    UserBehavior/touchPoint.cpp UserBehavior/touchPoint.h emp-sh2pc/alice_sort.h emp-sh2pc/alice_gen.cpp)
find_package(Threads REQUIRED)

find_path(CMAKE_FOLDER NAMES cmake/emp-tool-config.cmake)
include(${CMAKE_FOLDER}/cmake/common.cmake)
include(${CMAKE_FOLDER}/cmake/source_of_randomness.cmake)
include(${CMAKE_FOLDER}/cmake/threading.cmake)

find_package(emp-tool REQUIRED)
find_package(emp-ot REQUIRED)
include_directories(${EMP-TOOL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${EMP-OT_INCLUDE_DIRS})
install(FILES cmake/emp-ot-config.cmake DESTINATION cmake/)
install(FILES cmake/emp-sh2pc-config.cmake DESTINATION cmake/)
install(DIRECTORY emp-ot DESTINATION include/)
install(DIRECTORY emp-sh2pc DESTINATION include/)

target_link_libraries(RF_SP Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_SP_2 Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_SP_train Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})
target_link_libraries(RF_SP_sub_test Threads::Threads ntl gmp m ${EMP-TOOL_LIBRARIES} ${EMP-OT_LIBRARIES})