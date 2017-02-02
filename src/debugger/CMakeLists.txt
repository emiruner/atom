FILE(GLOB debuggerSources Main.cpp LineReader.cpp ImageSaver.cpp ImageLoader.cpp ../prims/Arithmetic.cpp ../prims/Display.cpp ../prims/Misc.cpp)
LINK_LIBRARIES(atomvm readline)
ADD_EXECUTABLE(zdb ${debuggerSources})
