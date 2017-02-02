FILE(GLOB_RECURSE testSources vm/*.cpp os/*.cpp assembler/*.cpp ../src/assembler/Lexer.cpp ../src/assembler/Parser.cpp ../src/assembler/Assembler.cpp)
LINK_LIBRARIES(gtest gtest_main atomvm pthread)
ADD_EXECUTABLE(runTests ${testSources})
