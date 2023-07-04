add_library(async SHARED async.cpp)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_executable(async_cli main.cpp)
target_link_libraries(async_cli
        async
        ${CMAKE_THREAD_LIBS_INIT}
        )

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)


