cmake_minimum_required(VERSION 3.10)

project(WapMap)

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

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

SET(HGE_DIR "${PROJECT_SOURCE_DIR}/../hge")
SET(LUA_DIR "${PROJECT_SOURCE_DIR}/../lua")
SET(SFML_DIR "${PROJECT_SOURCE_DIR}/../SFML-2.5.1-2017")
SET(CURL_DIR "${PROJECT_SOURCE_DIR}/../curl")
SET(ZLIB_DIR "${HGE_DIR}/src/zlib-1.2.5")


SET(HGE_LIBS hge.lib hgehelpers.lib)
SET(LUA_LIB lua.lib)
SET(SFML_LIBS
	optimized sfml-graphics-s.lib debug sfml-graphics-s-d.lib
	optimized sfml-window-s.lib sfml-window-s-d.lib
	optimized sfml-system-s.lib sfml-system-s-d.lib
	)
SET(CURL_LIB optimized libcurl_imp.lib debug libcurl-d_imp.lib)
SET(ZLIB_LIB optimized zlib.lib debug zlibd.lib)
SET(OTHER_LIBS Version.lib opengl32.lib freetype.lib)

add_definitions(-DNOMINMAX -DWAP_MAP -DWAP_TOOLS -DSFML_STATIC)
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_definitions(-DBUILD_DEBUG -DSHOWMEMUSAGE)
endif ()

include_directories(
    "${PROJECT_SOURCE_DIR}/incl"
    "${PROJECT_SOURCE_DIR}/shared"
    "${HGE_DIR}/include"
    "${LUA_DIR}"
    "${SFML_DIR}/include"
    "${CURL_DIR}/include"
    "${ZLIB_DIR}"
)

link_directories(
    "${SFML_DIR}/lib"
    "${HGE_DIR}/linklib/${CMAKE_BUILD_TYPE}"
    "${CURL_DIR}/build/lib/${CMAKE_BUILD_TYPE}"
    "${LUA_DIR}/${CMAKE_BUILD_TYPE}"
)


add_executable(WapMap
    ${PROJECT_SOURCE_DIR}/shared/cANI.cpp
    ${PROJECT_SOURCE_DIR}/shared/cConsole.cpp
    ${PROJECT_SOURCE_DIR}/shared/cGUIparts.cpp
    ${PROJECT_SOURCE_DIR}/shared/cInterface.cpp
    ${PROJECT_SOURCE_DIR}/shared/cLang.cpp
    ${PROJECT_SOURCE_DIR}/shared/commonFunc.cpp
    ${PROJECT_SOURCE_DIR}/shared/cPID.cpp
    ${PROJECT_SOURCE_DIR}/shared/cPieChart.cpp
    ${PROJECT_SOURCE_DIR}/shared/cProgressInfo.cpp
    ${PROJECT_SOURCE_DIR}/shared/cREZ.cpp
    ${PROJECT_SOURCE_DIR}/shared/cSFS.cpp
    ${PROJECT_SOURCE_DIR}/shared/cStateMgr.cpp
    ${PROJECT_SOURCE_DIR}/shared/cUpdater.cpp
    ${PROJECT_SOURCE_DIR}/shared/cWWD.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/cTooltip.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wButton.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wCheckbox.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wComboButton.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wContainer.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wContext.cpp
    ${PROJECT_SOURCE_DIR}/shared/gcnwidgets/wDropDown.cpp
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
    ${PROJECT_SOURCE_DIR}/WapMap/cAutoUpdater.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cBrush.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cColorPicker.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cControllerIPC.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cDataController.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cFileSystem.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cInventoryController.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cLogicsList.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cMDI.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cModeMenuBar.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cMRUList.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cNativeController.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cObjectUserData.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cParallelLoop.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cPhysics.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cRulers.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/cSerialManager.cpp
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
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editCheckpoint.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrabNest.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrate.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editCrumblingPeg.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editCurse.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editDialog.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editElevator.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editElevPath.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editEnemy.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editHealth.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editLaser.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editProjectile.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editRope.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editShake.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editSpecialPowerup.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editSpringboard.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editStalactite.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editStatue.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editText.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editTits.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editTogglePeg.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editTreasure.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editWallCannon.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/editWarp.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/procPickRect.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objedit/winInventoryPickbox.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/objQuadTree.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/rendering/cRenderableItem.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/returncodes.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/codeEditor.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/alActionListener.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/cLayerRenderBuffer.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/cObjectRenderBuffer.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/lmListModel.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/modeObject.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/modeTile.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/procHotkeysHandle.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/rendering.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/vpCallback.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/vPort.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winDatabase.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winFirstRun.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winNewMap.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/editing_ww/winPlaneProperties.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/error.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/imageDetails.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/imageImport.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/loadmap.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/mapshot.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/objedit.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/objprop.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/stats.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/states/updater.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/tools/brushObject.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/tools/brushTile.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/tools/noTool.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/tools/objectCreator.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/tools/pencil.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/vpFocusWidg.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/about.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/imgsetBrowser.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/options.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/tileBrowser.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/tileProperties.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/windows/worldProperties.cpp
    ${PROJECT_SOURCE_DIR}/WapMap/wViewport.cpp
)

target_link_libraries(WapMap
    ${HGE_LIBS}
    ${LUA_LIB}
    ${SFML_LIBS}
    ${CURL_LIB}
    ${ZLIB_LIB}
    ${OTHER_LIBS}
)

file(COPY ${CURL_DIR}/build/lib/${CMAKE_BUILD_TYPE}/libcurl-d.dll DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${HGE_DIR}/bin/hge.dll DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/bass.dll DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/SciLexer.dll DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/Scintilla.dll DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/brush DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/lang DESTINATION ${CMAKE_BINARY_DIR})
file(COPY res/res DESTINATION ${CMAKE_BINARY_DIR})