cmake_minimum_required(VERSION 3.22)
project(WapMap)

##----------------------------------------
## Edit this to match your configuration
##----------------------------------------

##----------------------------------------
## End of manual configuration section
## try to minimize tampering with code below unless you know how CMake works

ADD_CUSTOM_TARGET(debug
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
  )

ADD_CUSTOM_TARGET(release
  COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
  COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
  COMMENT "Switch CMAKE_BUILD_TYPE to Release"
  )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WAPMAP_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin/x64/$<CONFIG>")
else()
    set(WAPMAP_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${WAPMAP_OUTPUT_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${WAPMAP_OUTPUT_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${WAPMAP_OUTPUT_DIR}")

SET(LUA_DIR "${PROJECT_SOURCE_DIR}/../lua" CACHE PATH "Lua source and build directory")
SET(LUA_LIBRARY_DIR "${LUA_DIR}" CACHE PATH "Lua library build directory")
SET(HGE_COMPAT_DIR "${PROJECT_SOURCE_DIR}/WapMap/hgecompat")
SET(SFML_INSTALL_DIR "${PROJECT_SOURCE_DIR}/../SFML-3.0.2-install" CACHE PATH "SFML 3 install prefix")
SET(SFML_DIR "${SFML_INSTALL_DIR}/lib/cmake/SFML")
SET(CURL_DIR "${PROJECT_SOURCE_DIR}/../curl" CACHE PATH "curl source and build directory")
SET(CURL_BUILD_DIR "${CURL_DIR}/build" CACHE PATH "curl build directory")
option(WAPMAP_ENABLE_IMGUI "Enable migrated detachable ImGui panels" OFF)

set(SFML_STATIC_LIBRARIES ON)
list(PREPEND CMAKE_PREFIX_PATH "${SFML_INSTALL_DIR}")
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
find_package(ZLIB REQUIRED)

if(WAPMAP_ENABLE_IMGUI)
    set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/../imgui" CACHE PATH "Dear ImGui source directory")
    set(IMGUI_SFML_DIR "${PROJECT_SOURCE_DIR}/../imgui-sfml" CACHE PATH "imgui-sfml source directory")
    if(NOT EXISTS "${IMGUI_DIR}/imgui.h" OR NOT EXISTS "${IMGUI_SFML_DIR}/imgui-SFML.cpp")
        message(FATAL_ERROR "WAPMAP_ENABLE_IMGUI requires imgui and imgui-sfml checkouts at IMGUI_DIR and IMGUI_SFML_DIR.")
    endif()
    set(IMGUI_SFML_ENABLE_VIEWPORTS ON)
    add_subdirectory("${IMGUI_SFML_DIR}" "${CMAKE_BINARY_DIR}/imgui-sfml")
endif()

SET(LUA_LIB lua.lib)
SET(SFML_LIBS SFML::Graphics SFML::Window SFML::System SFML::Audio)
SET(CURL_LIB optimized libcurl.lib debug libcurl-d.lib)
SET(OTHER_LIBS
	Version.lib Winmm.lib Ws2_32.lib Dwmapi.lib Crypt32.lib Secur32.lib
	Wldap32.lib Iphlpapi.lib Bcrypt.lib Ole32.lib Shell32.lib opengl32.lib
)

add_compile_definitions(NOMINMAX CURL_STATICLIB WAP_MAP WAP_TOOLS SFML_STATIC)
add_compile_definitions($<$<CONFIG:Debug>:BUILD_DEBUG> $<$<CONFIG:Debug>:SHOWMEMUSAGE>)

include_directories(
    "${HGE_COMPAT_DIR}/include"
    "${PROJECT_SOURCE_DIR}/incl"
    "${PROJECT_SOURCE_DIR}/shared"
    "${LUA_DIR}"
    "${CURL_DIR}/include"
    "${CURL_BUILD_DIR}/include"
)

add_executable(WapMap
	${HGE_COMPAT_DIR}/src/hge_sfml.cpp
	${HGE_COMPAT_DIR}/src/audio.cpp
	${HGE_COMPAT_DIR}/src/graphics.cpp
	${HGE_COMPAT_DIR}/src/system.cpp
	${HGE_COMPAT_DIR}/src/helpers/hgecolor.cpp
	${HGE_COMPAT_DIR}/src/helpers/hgefont.cpp
	${HGE_COMPAT_DIR}/src/helpers/hgerect.cpp
	${HGE_COMPAT_DIR}/src/helpers/hgesprite.cpp
	${HGE_COMPAT_DIR}/src/helpers/hgevector.cpp
	${HGE_COMPAT_DIR}/src/win32.cpp
	${PROJECT_SOURCE_DIR}/shared/cANI.cpp
	${PROJECT_SOURCE_DIR}/shared/cConsole.cpp
	${PROJECT_SOURCE_DIR}/shared/cInterface.cpp
	${PROJECT_SOURCE_DIR}/shared/cLang.cpp
	${PROJECT_SOURCE_DIR}/shared/commonFunc.cpp
	${PROJECT_SOURCE_DIR}/shared/cPID.cpp
	${PROJECT_SOURCE_DIR}/shared/cPieChart.cpp
	${PROJECT_SOURCE_DIR}/shared/cProgressInfo.cpp
	${PROJECT_SOURCE_DIR}/shared/cREZ.cpp
	${PROJECT_SOURCE_DIR}/shared/cSFS.cpp
	${PROJECT_SOURCE_DIR}/shared/cStateMgr.cpp
	${PROJECT_SOURCE_DIR}/shared/cWMD.cpp
	${PROJECT_SOURCE_DIR}/shared/cWWD.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/detail/EndOfCentralDirectoryBlock.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/detail/ZipCentralDirectoryFileHeader.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/detail/ZipGenericExtraField.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/detail/ZipLocalFileHeader.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/ZipArchive.cpp
	${PROJECT_SOURCE_DIR}/shared/ZipLib/ZipArchiveEntry.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/cTooltip.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wButton.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wCheckbox.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wComboButton.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wContainer.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wContext.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wDropDown.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnWidgets/wIconListbox.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wInventory.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wLabel.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wLink.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wListbox.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wProgressBar.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wRadioBut.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wScrollArea.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wSlider.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTab.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTabbedArea.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTextBox.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTextDropDown.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTextField.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wTree.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wWin.cpp
	${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wWinMgr.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/actionevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/basiccontainer.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/cliprectangle.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/color.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/defaultfont.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/event.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/exception.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/focusevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/focushandler.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/font.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/genericinput.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/graphics.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/gui.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/guichan.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/hge/hgegraphics.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/hge/hgeimage.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/hge/hgeimagefont.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/hge/hgeimageloader.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/hge/hgeinput.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/image.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/imagefont.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/inputevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/key.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/keyevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/keyinput.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/mouseevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/mouseinput.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/rectangle.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/selectionevent.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widget.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/button.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/checkbox.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/container.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/dropdown.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/icon.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/imagebutton.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/label.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/listbox.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/radiobutton.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/scrollarea.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/slider.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/tab.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/tabbedarea.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/textbox.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/textfield.cpp
	${PROJECT_SOURCE_DIR}/shared/guichan/widgets/window.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_md5.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_md5wrapper.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha1.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha1wrapper.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha256.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha256wrapper.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha2ext.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha384wrapper.cpp
	${PROJECT_SOURCE_DIR}/shared/hashlib/hl_sha512wrapper.cpp
	${PROJECT_SOURCE_DIR}/WapMap/bodyQuadTree.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cAppMenu.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cAppMenuNative.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cAutoUpdater.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cBrush.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cColorPicker.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cControllerIPC.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cDataController.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cFileSystem.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cInventoryController.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cMDI.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cModeMenuBar.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cMRUList.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cNativeController.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cObjectUserData.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cParallelLoop.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cPhysics.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cRulers.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cSoundPlayer.cpp
	${PROJECT_SOURCE_DIR}/WapMap/cTextureAtlas.cpp
	${PROJECT_SOURCE_DIR}/WapMap/databanks/anims.cpp
	${PROJECT_SOURCE_DIR}/WapMap/databanks/imageSets.cpp
	${PROJECT_SOURCE_DIR}/WapMap/databanks/logics.cpp
	${PROJECT_SOURCE_DIR}/WapMap/databanks/sounds.cpp
	${PROJECT_SOURCE_DIR}/WapMap/databanks/tiles.cpp
	${PROJECT_SOURCE_DIR}/WapMap/funcConsole.cpp
	${PROJECT_SOURCE_DIR}/WapMap/globals.cpp
	${PROJECT_SOURCE_DIR}/WapMap/globlua.cpp
	${PROJECT_SOURCE_DIR}/WapMap/io/cWWDx.cpp
	${PROJECT_SOURCE_DIR}/WapMap/main.cpp
	${PROJECT_SOURCE_DIR}/WapMap/newsReader.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editAmbient.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editBreakPlank.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCandy.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCandyImGui.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCheckpoint.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrabNest.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrate.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrumblingPeg.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editCurse.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editElevator.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editElevPath.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editEnemy.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objEdit/editFloorSpike.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editHealth.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editLaser.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editProjectile.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editRope.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editShake.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editSoundTrigger.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editSpecialPowerup.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editSpringboard.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editStalactite.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editStatue.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editText.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editTogglePeg.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editTreasure.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editWallCannon.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editWarp.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/editLavaHand.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/procPickRect.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objEdit/procPickXYLockable.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objedit/winInventoryPickbox.cpp
	${PROJECT_SOURCE_DIR}/WapMap/objQuadTree.cpp
	${PROJECT_SOURCE_DIR}/WapMap/rendering/cRenderableItem.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/alActionListener.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/cLayerRenderBuffer.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/clipboards.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/lmListModel.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/modeObject.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/modeTile.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/navigation.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/nativeChrome.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/procHotkeysHandle.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/rendering.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/vpCallback.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/vPort.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winDatabase.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winFirstRun.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winPlaneProperties.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/dialog.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/imageDetails.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/imageImport.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/imageImportNative.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/loadmap.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/mapshot.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/objpropImGui.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/objedit.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/objprop.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/stats.cpp
	${PROJECT_SOURCE_DIR}/WapMap/states/statsNative.cpp
	${PROJECT_SOURCE_DIR}/WapMap/vpFocusWidg.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/about.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/imgsetBrowser.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/goToCoords.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/mapProperties.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/planeProperties.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/tileProperties.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/measureOptions.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/objectToolOptions.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/objectSearchWindow.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/logicBrowser.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/mapShotOptions.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/tilePicker.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/mapShotImageWriter.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/mediaBrowser.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/goToLocation.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/newMap.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/options.cpp
	${PROJECT_SOURCE_DIR}/WapMap/windows/tileBrowser.cpp
	${PROJECT_SOURCE_DIR}/WapMap/wViewport.cpp
	${PROJECT_SOURCE_DIR}/WapMap/res.rc
	${PROJECT_SOURCE_DIR}/WapMap/app.manifest
		WapMap/databanks/meta.cpp
		WapMap/databanks/meta.h
		WapMap/databanks/palettes.cpp
		WapMap/databanks/palettes.h
		WapMap/windows/paletteBrowser.cpp
		WapMap/windows/paletteBrowser.h)

if(WAPMAP_ENABLE_IMGUI)
    target_compile_definitions(WapMap PRIVATE WAPMAP_ENABLE_IMGUI)
endif()

target_link_libraries(WapMap
    ${LUA_LIB}
    ${SFML_LIBS}
    ${CURL_LIB}
    ZLIB::ZLIB
    ${OTHER_LIBS}
)
if(WAPMAP_ENABLE_IMGUI)
    target_link_libraries(WapMap ImGui-SFML::ImGui-SFML)
endif()

target_link_directories(WapMap PRIVATE
    "${CURL_BUILD_DIR}/lib/$<CONFIG>"
    "${LUA_LIBRARY_DIR}/$<CONFIG>"
)

add_custom_command(TARGET WapMap POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/res/copy"
        "$<TARGET_FILE_DIR:WapMap>"
)
