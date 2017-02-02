FILE(GLOB primsSources Display.cpp Arithmetic.cpp Misc.cpp)
ADD_LIBRARY(atomprims SHARED ${primsSources})
