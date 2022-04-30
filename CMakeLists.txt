cmake_minimum_required(VERSION 3.21)
set(VCPKG_TARGET_TRIPLET x64-windows)
set(VCPKG_MANIFEST_MODE ON)
project(schematic)

set(CMAKE_CXX_STANDARD 20)

add_executable(schematic AppMain.cpp AppMain.h FrameMain.cpp FrameMain.h id.h WindowGrid.cpp WindowGrid.h Grid.cpp Grid.h Item.cpp Item.h Resources.h Resources.cpp NewSchematicDialog.cpp NewSchematicDialog.h DotSizeDialog.cpp DotSizeDialog.h)
target_include_directories(schematic PRIVATE ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(WX_LIB_DIR ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/debug/lib)
    target_link_libraries(schematic ${WX_LIB_DIR}/wxbase31ud.lib ${WX_LIB_DIR}/wxmsw31ud_core.lib ${WX_LIB_DIR}/wxmsw31ud_propgrid.lib)
else()
    find_package(wxWidgets REQUIRED COMPONENTS core base propgrid) #this only works for release
    target_link_libraries(schematic ${wxWidgets_LIBRARIES})
endif()
target_link_options(schematic PRIVATE "/subsystem:WINDOWS")
target_sources(schematic PRIVATE schematic.manifest)
add_custom_command(TARGET schematic PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res)