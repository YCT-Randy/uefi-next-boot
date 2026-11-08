cmake_minimum_required(VERSION 3.20)
project(BootManager VERSION 0.1.1 LANGUAGES CXX)

if(WIN32)
    enable_language(RC)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets Concurrent)

add_library(boot_core
    src/core/boot_backend.cpp
    src/core/boot_flow.cpp
    src/core/boot_parser.cpp
    src/utils/command_runner.cpp
    src/platform/windows/windows_backend.cpp
    src/platform/linux/linux_backend.cpp
)
target_include_directories(boot_core PUBLIC include)
target_link_libraries(boot_core PUBLIC Qt6::Core)

add_executable(boot-manager WIN32 src/main.cpp src/gui/main_window.cpp include/boot_manager/gui/main_window.h)
target_compile_definitions(boot-manager PRIVATE BOOT_MANAGER_VERSION="${PROJECT_VERSION}")
target_link_libraries(boot-manager PRIVATE boot_core Qt6::Widgets Qt6::Concurrent)

if(WIN32)
    configure_file(packaging/windows/app.rc.in ${CMAKE_CURRENT_BINARY_DIR}/app.rc @ONLY)
    target_sources(boot-manager PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
endif()

include(GNUInstallDirs)
install(TARGETS boot-manager RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(UNIX AND NOT APPLE)
    install(PROGRAMS packaging/linux/boot-manager-helper DESTINATION lib/boot-manager)
    install(FILES packaging/linux/org.bootmanager.helper.policy DESTINATION share/polkit-1/actions)
    install(FILES packaging/linux/boot-manager.desktop DESTINATION share/applications)
    set(CPACK_GENERATOR DEB)
    set(CPACK_PACKAGE_NAME boot-manager)
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Boot Manager")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "efibootmgr, policykit-1 | polkitd")
    set(CPACK_PACKAGING_INSTALL_PREFIX /usr)
    include(CPack)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(boot_parser_tests tests/boot_parser_tests.cpp)
    target_link_libraries(boot_parser_tests PRIVATE boot_core)
    add_test(NAME boot_parser_tests COMMAND boot_parser_tests)
endif()
