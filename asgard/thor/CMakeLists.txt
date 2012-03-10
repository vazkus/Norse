FIND_PACKAGE(Qt4)

INCLUDE(${QT_USE_FILE})
# see http://cmake.org/cmake/help/cmake2.6docs.html#module:FindQt4
SET(CMAKE_BUILD_TYPE Debug)

SET(NORSE_ROOT    /home/vazgen/Projects/github/Norse)
SET(YGGDRASIL_DIR ${NORSE_ROOT}/Yggdrasil)
SET(RATATOSK_DIR  ${NORSE_ROOT}/Ratatosk)

INCLUDE_DIRECTORIES(${HELLO_SOURCE_DIR}/BUILD/linux/include) 
INCLUDE_DIRECTORIES($ENV{TCL_PATH}) 
INCLUDE_DIRECTORIES(${YGGDRASIL_DIR}) 
INCLUDE_DIRECTORIES(${RATATOSK_DIR}) 

SET(tho_posix_sources  thoPosixMain.cpp)

SET(tho_qt_sources     thoMainWindow.cpp
                       thoInputHandler.cpp
                       thoQtMain.cpp 
   )

SET(tho_qt_moc_headers thoMainWindow.hpp 
                       thoInputHandler.hpp 
   )

QT4_WRAP_CPP(tho_qt_moc_sources ${tho_qt_moc_headers})
SET(POSIX_LIBRARIES rt pthread)

ADD_EXECUTABLE(thorQt ${tho_qt_sources} ${tho_qt_moc_sources})
ADD_EXECUTABLE(thorPosix ${tho_posix_sources} )

# add libraries...
TARGET_LINK_LIBRARIES(thorQt ${QT_LIBRARIES})
TARGET_LINK_LIBRARIES(thorPosix ${POSIX_LIBRARIES})

