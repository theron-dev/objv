
CORE = ../../include
CORE_PRIVATE = ../../private
CORE_SRC = ../../source
CLOUD = ../../cloud
HTTP = ../../http
SERVER = ../../server
DB = ../../db
TASKS = ../../tasks
DBOBJECTS = ../../dbobjects
SERVICES = ../../services

LIBRARY = -lc -lz -lpthread -lmysqlclient -lobjv -L../../libs -L/usr/lib64 -L/usr/lib64/mysql
INCLUDE = -I/usr/include -I/usr/include/mysql -I$(CORE) -I$(CORE_PRIVATE) -I$(CLOUD) -I$(HTTP) -I$(SERVER) -I$(DB) -I$(TASKS) -I$(DBOBJECTS) -I$(SERVICES)
CFLAGS = -std=gnu99 -DTARGET_OS_LINUX

EXECNAME = objvcloud
OUTPUT = ./bin
TARGET = $(OUTPUT)/$(EXECNAME)

SOURCE      = main.c CLAccept.c CLSRVProcess.c CLLoader.c

$(TARGET):
	cc $(CFLAGS) $(INCLUDE) $(SOURCE) -o $(TARGET) $(LIBRARY)

install:
	cp $(TARGET) /usr/bin/$(EXECNAME)

uninstall:
	rm -f /usr/bin/$(EXECNAME)

clean:
	rm -rf $(TARGET)

