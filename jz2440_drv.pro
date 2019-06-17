DEFINES += KERNEL
DEFINES += MODULE

#TARGET = jz2440_drv
TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += led_drv.c \
           buttons.c


SRC_PROJECT_PATH = /work/platform-2440/js2440_dtb/kernel/linux-4.19-rc3
LINUX_HEADERS_PATH = /work/platform-2440/js2440_dtb/kernel/linux-4.19-rc3


ARCH = arm
SOURCES += system(find −L $$SRC_PROJECT_PATH -type f -name “*.c” -o -name “*.S” )
HEADERS += system(find −L $$SRC_PROJECT_PATH -type f -name “*.h” )
OTHER_FILES += system(find −L $$SRC_PROJECT_PATH -type f -not -name “*.h” -not -name “*.c” -not -name “*.S” )
INCLUDEPATH += system(find −L $$SRC_PROJECT_PATH -type d)
INCLUDEPATH += system(find −L $$LINUX_HEADERS_PATH/include/ -type d)
INCLUDEPATH += system(find −L $$LINUX_HEADERS_PATH/include/linux/ -type d)
INCLUDEPATH += system(find −L $$LINUX_HEADERS_PATH/arch/arm/include/ -type d)
INCLUDEPATH += system(find −L $$LINUX_HEADERS_PATH/arch/arm/mach-s3c24xx/include/ -type d)
INCLUDEPATH += system(find −L $$LINUX_HEADERS_PATH/arch/arm/plat-samsung/include/ -type d)
