include ../conf

TARGET_ARCH= -mno-cygwin
CFLAGS= $(WARNINGS) $(DEFINES) $(INCLUDES)
DEFINES= -DWIN32_LEAN_AND_MEAN -DNOGDI
INCLUDES= $(LUAINC)
WARNINGS= -W -Wall

OBJS= ex.o spawn.o pusherror.o dirent.o
LIBS= $(LUALIB)
ex.dll: $(OBJS); $(CC) $(TARGET_ARCH) -shared -o $@ $(OBJS) $(LIBS)

clean:; rm -f ex.dll *.o
