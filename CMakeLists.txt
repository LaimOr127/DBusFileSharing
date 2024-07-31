cmake_minimum_required(VERSION 3.10)

project(SharingService LANGUAGES C CXX)

find_package(sdbus-c++ REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(INIH REQUIRED inih)

link_directories(/usr/lib/)
add_executable(sharing main.cpp sharing.cpp)
target_link_libraries(sharing PRIVATE SDBusCpp::sdbus-c++ systemd ${INIH_LIBRARIES})
target_include_directories(sharing PRIVATE ${INIH_INCLUDE_DIRS})
set_target_properties(sharing PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

include(GNUInstallDirs)
install(TARGETS sharing
    DESTINATION ${CMAKE_INSTALL_BINDIR}/
)
file(REAL_PATH ~/.config user_config_dir EXPAND_TILDE)
install(FILES data/org.rt.sharing.service
    DESTINATION ${user_config_dir}/systemd/user
)
install(FILES data/dbus-sharing.conf
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/sharing/
)
