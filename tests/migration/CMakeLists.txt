cmake_minimum_required(VERSION 3.22)
project(WapMapMigrationRegression LANGUAGES CXX)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio)
find_package(ZLIB REQUIRED)
set(ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
add_executable(migration_regression
    regression.cpp
    ${ROOT}/shared/cPID.cpp
    ${ROOT}/WapMap/windows/mapShotImageWriter.cpp
    ${ROOT}/WapMap/hgecompat/src/graphics.cpp
    ${ROOT}/shared/guichan/hge/hgeinput.cpp
    ${ROOT}/shared/guichan/key.cpp
    ${ROOT}/shared/guichan/keyinput.cpp
    ${ROOT}/shared/guichan/mouseinput.cpp
    ${ROOT}/shared/guichan/exception.cpp)
target_compile_features(migration_regression PRIVATE cxx_std_20)
target_compile_definitions(migration_regression PRIVATE NOMINMAX)
target_include_directories(migration_regression PRIVATE
    ${ROOT}/WapMap/hgecompat/include ${ROOT}/WapMap/hgecompat/src ${ROOT}/incl)
target_link_libraries(migration_regression PRIVATE SFML::Graphics SFML::Audio ZLIB::ZLIB)
enable_testing()
add_test(NAME migration_regression COMMAND migration_regression)

set(IMGUI_DIR "${ROOT}/../imgui" CACHE PATH "Dear ImGui docking checkout for font regression")
if(EXISTS "${IMGUI_DIR}/imgui.cpp")
    add_executable(bitmap_font_regression
        fontSkin.cpp
        ${ROOT}/WapMap/hgecompat/src/graphics.cpp
        ${ROOT}/WapMap/hgecompat/src/helpers/hgefont.cpp
        ${ROOT}/WapMap/hgecompat/src/helpers/hgesprite.cpp
        ${ROOT}/WapMap/hgecompat/src/helpers/hgerect.cpp
        ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp ${IMGUI_DIR}/imgui_tables.cpp)
    target_compile_features(bitmap_font_regression PRIVATE cxx_std_20)
    target_compile_definitions(bitmap_font_regression PRIVATE NOMINMAX WAPMAP_ENABLE_IMGUI)
    if(MSVC)
        target_compile_options(bitmap_font_regression PRIVATE /UNDEBUG)
    else()
        target_compile_options(bitmap_font_regression PRIVATE -UNDEBUG)
    endif()
    target_include_directories(bitmap_font_regression PRIVATE ${IMGUI_DIR}
        ${ROOT}/WapMap/hgecompat/include ${ROOT}/WapMap/hgecompat/src)
    target_link_libraries(bitmap_font_regression PRIVATE SFML::Graphics SFML::Audio)
    add_test(NAME bitmap_font_regression COMMAND bitmap_font_regression)
endif()
