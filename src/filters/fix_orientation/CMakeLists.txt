PROJECT("Fix Orientation Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

FILE(GLOB ui_files "ui/*.ui")
qt5_wrap_ui(ui_sources ${ui_files})
SET_SOURCE_FILES_PROPERTIES(${ui_sources} PROPERTIES GENERATED TRUE)
SOURCE_GROUP("UI Files" FILES ${ui_files})
SOURCE_GROUP("Generated" FILES ${ui_sources})

SET(
    sources
    ImageView.cpp ImageView.h
    Filter.cpp Filter.h
    OptionsWidget.cpp OptionsWidget.h
    ApplyDialog.cpp ApplyDialog.h
    Settings.cpp Settings.h
    Task.cpp Task.h
    CacheDrivenTask.cpp CacheDrivenTask.h
)
SOURCE_GROUP("Sources" FILES ${sources})
ADD_LIBRARY(fix_orientation STATIC ${sources} ${ui_sources})
target_link_libraries(fix_orientation PUBLIC Qt5::Widgets Qt5::Xml Qt5::Gui Qt5::Core)

TRANSLATION_SOURCES(scantailor ${sources} ${ui_files})