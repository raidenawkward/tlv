MODULE_NAME := tlv

RM := rm
CP := cp
STRIP := strip
AR := ar

CFLAGS := -Wall -fPIC

SRC := $(shell ls *.c)
OBJS := $(SRC:.c=.o)
LIB_OBJS := $(SRC:main.o=)

TARGET_EXEC := $(MODULE_NAME)
TARGET_LIB := lib$(MODULE_NAME).so
TARGET_LIB_STRIPPED := lib$(MODULE_NAME)-stripped.so
TARGET_ALIB := lib$(MODULE_NAME).a

default:
	##########################################################
	#                      TLV Makefile                      #
	#                                                        #
	#    targets:                                            #
	#        exec           -   build executable with main.c #
	#        lib            -   build .so without strip      #
	#        striplib       -   build striped .so            #
	#        ar             -   build .a                     #
	#        all            -   build all targets above      #
	#        clean          -   clean all products above     #
	#        default        -   show this message            #
	#                                                        #
	#                                                        #
	##########################################################
exec:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET_EXEC)
lib:$(LIB_OBJS)
	$(CC) $(CFLAGS) $(LIB_OBJS) -shared -o $(TARGET_LIB)
striplib:$(LIB_OBJS)
	$(CC) $(CFLAGS) $(LIB_OBJS) -shared -o $(TARGET_LIB_STRIPPED)
	$(STRIP) $(TARGET_LIB_STRIPPED)
ar:$(LIB_OBJS)
	$(AR) -r $(TARGET_ALIB) $(LIB_OBJS)
all:
	$(MAKE) exec
	$(MAKE) ar
	$(MAKE) lib
	$(MAKE) striplib
clean:
	$(RM) -f $(OBJS) $(TARGET_EXEC) $(TARGET_LIB) $(TARGET_LIB_STRIPPED) $(TARGET_ALIB)

