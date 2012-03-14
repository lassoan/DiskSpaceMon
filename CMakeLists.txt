CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (DiskSpaceMon CXX C)

INCLUDE(InstallRequiredSystemLibraries) 

SET(VERSION_MAJOR 1)
SET(VERSION_MINOR 0)
SET(VERSION_PATCH 0) 

#Add QT support
FIND_PACKAGE(Qt4 REQUIRED) 
INCLUDE(${QT_USE_FILE})

#Include folders 
INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
  )

#Here we add our source files
SET (DiskSpaceMon_SRC
	main.cpp	
  MyMainWindow.cpp
  )

#Define QT resource file.this file may define icons,images.
SET(DiskSpaceMon_RCCS
  resources/myresources.qrc
  )

#QT headers needed to be processed with QT MOC compiler
SET(DiskSpaceMon_MOC_HDRS
  MyMainWindow.h
  )

#QT UI files needed to be processed
SET(DiskSpaceMon_UIS
  MyMainWindow.ui)

#Generate CPP files from MOC headers
QT4_WRAP_CPP(DiskSpaceMon_MOC_SRC ${DiskSpaceMon_MOC_HDRS})

#Generate resource header files 
QT4_ADD_RESOURCES(DiskSpaceMon_RCCS_SRC ${DiskSpaceMon_RCCS})

#Generate headers using QT UI files
QT4_WRAP_UI(DiskSpaceMon_UI_HDR ${DiskSpaceMon_UIS})

ADD_EXECUTABLE(DiskSpaceMon WIN32 
  ${DiskSpaceMon_SRC} 
  ${DiskSpaceMon_UI_HDR} 
  ${DiskSpaceMon_MOC_SRC} 
  ${DiskSpaceMon_RCCS_SRC} 
  )
TARGET_LINK_LIBRARIES(DiskSpaceMon  
  ${QT_LIBRARIES}
  ${QT_QTMAIN_LIBRARY} 
  )

SET(CPACK_PACKAGE_NAME "DiskSpaceMon")
SET(CPACK_PACKAGE_VENDOR "")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Display an alert if free disk space gets below a threshold value.")
SET(CPACK_PACKAGE_CONTACT "")
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_GENERATOR NSIS)
SET(CPACK_NSIS_CONTACT "")

if(WIN32 AND NOT UNIX)
	SET(CPACK_PACKAGE_EXECUTABLES "DiskSpaceMon;Disk space monitor")
endif()

# Install the Qt library files
INSTALL(FILES ${QT_BINARY_DIR}/QtCore4.dll DESTINATION bin)
INSTALL(FILES ${QT_BINARY_DIR}/QtGui4.dll DESTINATION bin)
INSTALL(FILES ${PROJECT_BINARY_DIR}/Release/DiskSpaceMon.exe DESTINATION bin)


# This should always be the last line
INCLUDE(CPack)
