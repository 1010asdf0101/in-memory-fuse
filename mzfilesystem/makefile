# Copyright (C) JAEHYUK CHO
# All rights reserved.
# Code by JaeHyuk Cho <mailto:minzkn@minzkn.com>

CROSS_COMPILE                ?=#

CC                           := $(CROSS_COMPILE)gcc#
RM                           := rm -f#
STRIP                        := $(CROSS_COMPILE)strip#

THIS_NAME                    := mzfilesystem#

CFLAGS                       := -Os -pipe#
CFLAGS                       += -Wall -Werror#
CFLAGS                       += -fomit-frame-pointer#
#CFLAGS                       += -ansi#
CFLAGS                       += -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64#
CFLAGS                       += -I.#

LDFLAGS                      := -s -lfuse -lcrypt -lpthread#

TARGET                       := $(THIS_NAME)#
#OBJECTS                      := $(THIS_NAME).o#
OBJECTS                      := main.o mztree.o mzftpd.o#

.PHONY: all clean

all: $(TARGET) ; $(STRIP) --remove-section=.comment --remove-section=.note $(TARGET)
clean: ; $(RM) *.o $(TARGET)

$(TARGET): $(OBJECTS) ; $(CC) $(LDFLAGS) -o $(@) $(^)
$(OBJECTS): makefile
%.o: %.c ; $(CC) $(CFLAGS) -c -o $(@) $(<)

# End of Makefile
