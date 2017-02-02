FILE(GLOB assemblerSources Main.cpp Lexer.cpp Assembler.cpp Parser.cpp)
LINK_LIBRARIES(atomvm)
ADD_EXECUTABLE(zas ${assemblerSources})
