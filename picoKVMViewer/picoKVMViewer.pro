TEMPLATE = app
TARGET = picoKVMViewer

QT += multimedia multimediawidgets serialport

HEADERS = \
    viewer.h \
    imagesettings.h \
    videosettings.h \
    ExtendedCameraViewfinder.h

SOURCES = \
    main.cpp \
    viewer.cpp \
    imagesettings.cpp \
    videosettings.cpp \
    ExtendedCameraViewfinder.cpp

FORMS += \
    viewer.ui \
    videosettings.ui \
    imagesettings.ui

RESOURCES += viewer.qrc

QT+=widgets
