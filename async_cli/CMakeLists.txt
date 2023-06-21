
add_executable(async_cli main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/async)
set_target_properties(libasync  PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(async_cli libasync)

install(TARGETS async_cli)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT https://github.com/friendpostum/Otus_hw9)
include(CPack)


