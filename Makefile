TARGET = 2048
OBJS = main.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP

#软件名称
PSP_EBOOT_TITLE = 2048
#软件图标（144*80）
PSP_EBOOT_ICON = icon.png
#软件背景（480*272）
PSP_EBOOT_PIC1 = miku.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
