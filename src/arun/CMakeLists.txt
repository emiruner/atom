FILE(GLOB arunSources Main.cpp)
ADD_EXECUTABLE(arun ${arunSources})
TARGET_LINK_LIBRARIES(arun atomvm)
