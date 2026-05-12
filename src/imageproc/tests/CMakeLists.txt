INCLUDE_DIRECTORIES(BEFORE ..)

SET(
	sources
	main.cpp
	TestBinaryImage.cpp TestReduceThreshold.cpp
	TestSlicedHistogram.cpp
	TestConnCompEraser.cpp TestConnCompEraserExt.cpp
	TestGrayscale.cpp
	TestRasterOp.cpp TestShear.cpp
	TestOrthogonalRotation.cpp
	TestSkewFinder.cpp
	TestScale.cpp
	TestTransform.cpp
	TestMorphology.cpp
	TestDentFinder.cpp
	TestBinarize.cpp
	TestPolygonRasterizer.cpp
	TestSeedFill.cpp
	TestSEDM.cpp
	TestRastLineFinder.cpp
	Utils.cpp Utils.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_EXECUTABLE(imageproc_tests ${sources})
TARGET_LINK_LIBRARIES(
	imageproc_tests
	imageproc math foundation
	${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
	${Boost_PRG_EXECUTION_MONITOR_LIBRARY}
	Qt5::Widgets Qt5::Gui Qt5::Core
	${EXTRA_LIBS}
)
