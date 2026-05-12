PROJECT("Page Split Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

FILE(GLOB ui_files "ui/*.ui")
qt5_wrap_ui(ui_sources ${ui_files})
SET_SOURCE_FILES_PROPERTIES(${ui_sources} PROPERTIES GENERATED TRUE)
SOURCE_GROUP("UI Files" FILES ${ui_files})
SOURCE_GROUP("Generated" FILES ${ui_sources})

SET(
    sources
    SplitLineObject.h
    ImageView.cpp ImageView.h
    Thumbnail.cpp Thumbnail.h
    Params.cpp Params.h
    Dependencies.cpp Dependencies.h
    PageLayout.cpp PageLayout.h
    PageLayoutEstimator.cpp PageLayoutEstimator.h
    VertLineFinder.cpp VertLineFinder.h
    Filter.cpp Filter.h
    OptionsWidget.cpp OptionsWidget.h
    SplitModeDialog.cpp SplitModeDialog.h
    Settings.cpp Settings.h
    Task.cpp Task.h
    CacheDrivenTask.cpp CacheDrivenTask.h
    LayoutType.cpp LayoutType.h
    UnremoveButton.cpp UnremoveButton.h
    OrderBySplitTypeProvider.cpp OrderBySplitTypeProvider.h
)
SOURCE_GROUP("Sources" FILES ${sources})
ADD_LIBRARY(page_split STATIC ${sources} ${ui_sources})
target_link_libraries(page_split PUBLIC Qt5::Widgets Qt5::Xml Qt5::Gui Qt5::Core)

TRANSLATION_SOURCES(scantailor ${sources} ${ui_files})