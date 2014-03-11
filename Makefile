
CORE = ./include
CORE_SRC = ./source
CLOUD = ./cloud
HTTP = ./http
SERVER = ./server
DB = ./db
TASKS = ./tasks
DBOBJECTS = ./dbobjects
SERVICES = ./services

INCLUDE = -I$(CORE) -I$(CLOUD) -I$(HTTP) -I$(SERVER) -I$(DB) -I$(TASKS) -I$(DBOBJECTS) -I$(SERVICES)
CFLAGS = -DTARGET_OS_LINUX

OUTPUT = ./libs
TARGET = $(OUTPUT)/libobjv.a
OBJOUTPUT = ./objs

CORE_C      = $(CORE_SRC)/objv.c $(CORE_SRC)/objv_zone.c $(CORE_SRC)/objv_types.c $(CORE_SRC)/objv_value.c $(CORE_SRC)/objv_data.c \
                 $(CORE_SRC)/objv_string.c $(CORE_SRC)/objv_array.c $(CORE_SRC)/objv_dispatch.c $(CORE_SRC)/objv_dispatch_queue.c $(CORE_SRC)/objv_map.c \
                 $(CORE_SRC)/objv_hash_map.c $(CORE_SRC)/objv_autorelease.c $(CORE_SRC)/objv_log.c $(CORE_SRC)/objv_dictionary.c $(CORE_SRC)/objv_mbuf.c \
                 $(CORE_SRC)/objv_json.c $(CORE_SRC)/objv_zombie.c $(CORE_SRC)/objv_vermin.c $(CORE_SRC)/objv_vermin_runtime.c $(CORE_SRC)/objv_exception.c \
                 $(CORE_SRC)/objv_iterator.c $(CORE_SRC)/objv_tokenizer.c $(CORE_SRC)/objv_vmcompiler.c $(CORE_SRC)/objv_vmcompiler_tokenizer.c $(CORE_SRC)/objv_vmcompiler_expression.c \
                 $(CORE_SRC)/objv_vmcompiler_invokes.c $(CORE_SRC)/objv_vmcompiler_class.c $(CORE_SRC)/objv_vmcompiler_meta.c $(CORE_SRC)/objv_vmcompiler_binary.c \
                 $(CORE_SRC)/objv_crypt.c $(CORE_SRC)/md5.c $(CORE_SRC)/objv_vm.c $(CORE_SRC)/objv_gc.c $(CORE_SRC)/objv_inifile.c $(CORE_SRC)/objv_channel.c \
                 $(CORE_SRC)/objv_channel_tcp.c $(CORE_SRC)/objv_actree.c $(CORE_SRC)/objv_url.c $(CORE_SRC)/objv_mail.c

CLOUD_C     = $(CLOUD)/objv_cloud.c $(CLOUD)/objv_clchannel.h $(CLOUD)/objv_clcontext.c $(CLOUD)/objv_clchannel_http.c
HTTP_C      = $(HTTP)/objv_http.c 
SERVER_C    = $(SERVER)/objv_server_posix.c
DB_C        = $(DB)/objv_db.c $(DB)/objv_db_sqlite.c $(DB)/objv_db_mysql.c
TASKS_C     = $(TASKS)/CLMessageTask.c
DBOBJECTS_C = $(DBOBJECTS)/CLMessageObject.c
SERVICES_C  = $(SERVICES)/CLMonitorService.c $(SERVICES)/CLMessageService.c

OBJECTS := $(OBJOUTPUT)/hconfig.o $(OBJOUTPUT)/htypes.o $(OBJOUTPUT)/hbase64.o $(OBJOUTPUT)/hbtree.o $(OBJOUTPUT)/hbuffer.o $(OBJOUTPUT)/hclass.o \
	 $(OBJOUTPUT)/hdata.o $(OBJOUTPUT)/hdata_alloc.o $(OBJOUTPUT)/hdata_binary.o $(OBJOUTPUT)/hdata_json.o $(OBJOUTPUT)/hdata_set.o $(OBJOUTPUT)/hdata_storage.o \
	 $(OBJOUTPUT)/hdata_xml.o $(OBJOUTPUT)/hdebug.o  $(OBJOUTPUT)/hext_obj.o $(OBJOUTPUT)/hfile.o $(OBJOUTPUT)/hinifile.o $(OBJOUTPUT)/hlibrary.o \
	 $(OBJOUTPUT)/hlist.o $(OBJOUTPUT)/hlog.o $(OBJOUTPUT)/hmap.o $(OBJOUTPUT)/hmem.o $(OBJOUTPUT)/hmutex.o $(OBJOUTPUT)/hqueue.o $(OBJOUTPUT)/hrecursive.o \
	 $(OBJOUTPUT)/hserial_list.o $(OBJOUTPUT)/hserial_map.o $(OBJOUTPUT)/hstack.o $(OBJOUTPUT)/hstr.o $(OBJOUTPUT)/hstruct_list.o $(OBJOUTPUT)/hthread.o \
	 $(OBJOUTPUT)/htime.o $(OBJOUTPUT)/huniquekey.o $(OBJOUTPUT)/hurl.o $(OBJOUTPUT)/hwaiter.o $(OBJOUTPUT)/md5.o $(OBJOUTPUT)/hrunloop.o $(OBJOUTPUT)/hform_data.o \
	 $(OBJOUTPUT)/hstream.o $(OBJOUTPUT)/hpeer.o $(OBJOUTPUT)/hpeer_remote.o $(OBJOUTPUT)/hpeer_stream.o $(OBJOUTPUT)/hpeer_remote_peer.o \
	 $(OBJOUTPUT)/hpeer_remote_runloop.o $(OBJOUTPUT)/hpeer_package.o $(OBJOUTPUT)/hprocess.o $(OBJOUTPUT)/hstream_websocket.o $(OBJOUTPUT)/sha1.o $(OBJOUTPUT)/hbytes.o \
	 $(OBJOUTPUT)/hstream_tcp.o $(OBJOUTPUT)/hstream_process.o $(OBJOUTPUT)/hhttp_request.o $(OBJOUTPUT)/hhttp_response.o


$(TARGET):$(OBJECTS)
	ar crv $(TARGET) $(OBJECTS)

$(OBJOUTPUT)/objv.o:$(CORE_SRC)/objv.c $(CORE)/objv.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv.c -o $(OBJOUTPUT)/objv.o

$(OBJOUTPUT)/objv_zone.o:$(CORE_SRC)/objv_zone.c $(CORE)/objv_zone.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_zone.c -o $(OBJOUTPUT)/objv_zone.o
    
$(OBJOUTPUT)/objv_types.o:$(CORE_SRC)/objv_types.c $(CORE)/objv_types.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_types.c -o $(OBJOUTPUT)/objv_types.o

$(OBJOUTPUT)/objv_value.o:$(CORE_SRC)/objv_value.c $(CORE)/objv_value.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_value.c -o $(OBJOUTPUT)/objv_value.o
   
$(OBJOUTPUT)/objv_data.o:$(CORE_SRC)/objv_data.c $(CORE)/objv_data.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_data.c -o $(OBJOUTPUT)/objv_data.o

$(OBJOUTPUT)/objv_string.o:$(CORE_SRC)/objv_string.c $(CORE)/objv_string.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_string.c -o $(OBJOUTPUT)/objv_string.o

$(OBJOUTPUT)/objv_array.o:$(CORE_SRC)/objv_array.c $(CORE)/objv_array.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_array.c -o $(OBJOUTPUT)/objv_array.o

$(OBJOUTPUT)/objv_dispatch.o:$(CORE_SRC)/objv_dispatch.c $(CORE)/objv_dispatch.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_dispatch.c -o $(OBJOUTPUT)/objv_dispatch.o

$(OBJOUTPUT)/objv_dispatch_queue.o:$(CORE_SRC)/objv_dispatch_queue.c $(CORE)/objv_dispatch_queue.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_dispatch_queue.c -o $(OBJOUTPUT)/objv_dispatch_queue.o

$(OBJOUTPUT)/objv_map.o:$(CORE_SRC)/objv_map.c $(CORE)/objv_map.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_map.c -o $(OBJOUTPUT)/objv_map.o

$(OBJOUTPUT)/objv_hash_map.o:$(CORE_SRC)/objv_hash_map.c $(CORE)/objv_hash_map.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_hash_map.c -o $(OBJOUTPUT)/objv_hash_map.o

$(OBJOUTPUT)/objv_autorelease.o:$(CORE_SRC)/objv_autorelease.c $(CORE)/objv_autorelease.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_autorelease.c -o $(OBJOUTPUT)/objv_autorelease.o

$(OBJOUTPUT)/objv_log.o:$(CORE_SRC)/objv_log.c $(CORE)/objv_log.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_log.c -o $(OBJOUTPUT)/objv_log.o
  
$(OBJOUTPUT)/objv_dictionary.o:$(CORE_SRC)/objv_dictionary.c $(CORE)/objv_dictionary.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_dictionary.c -o $(OBJOUTPUT)/objv_dictionary.o

$(OBJOUTPUT)/objv_mbuf.o:$(CORE_SRC)/objv_mbuf.c $(CORE)/objv_mbuf.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_mbuf.c -o $(OBJOUTPUT)/objv_mbuf.o

$(OBJOUTPUT)/objv_json.o:$(CORE_SRC)/objv_json.c $(CORE)/objv_json.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_json.c -o $(OBJOUTPUT)/objv_json.o

$(OBJOUTPUT)/objv_zombie.o:$(CORE_SRC)/objv_zombie.c $(CORE)/objv_zombie.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_zombie.c -o $(OBJOUTPUT)/objv_zombie.o

$(OBJOUTPUT)/objv_vermin.o:$(CORE_SRC)/objv_vermin.c $(CORE)/objv_vermin.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vermin.c -o $(OBJOUTPUT)/objv_vermin.o

$(OBJOUTPUT)/objv_vermin_runtime.o:$(CORE_SRC)/objv_vermin_runtime.c $(CORE)/objv_vermin_runtime.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vermin_runtime.c -o $(OBJOUTPUT)/objv_vermin_runtime.o
   
$(OBJOUTPUT)/objv_exception.o:$(CORE_SRC)/objv_exception.c $(CORE)/objv_exception.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_exception.c -o $(OBJOUTPUT)/objv_exception.o

$(OBJOUTPUT)/objv_iterator.o:$(CORE_SRC)/objv_iterator.c $(CORE)/objv_iterator.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_iterator.c -o $(OBJOUTPUT)/objv_iterator.o
   
$(OBJOUTPUT)/objv_tokenizer.o:$(CORE_SRC)/objv_tokenizer.c $(CORE)/objv_tokenizer.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_tokenizer.c -o $(OBJOUTPUT)/objv_tokenizer.o
    
$(OBJOUTPUT)/objv_vmcompiler.o:$(CORE_SRC)/objv_vmcompiler.c $(CORE)/objv_vmcompiler.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler.c -o $(OBJOUTPUT)/objv_vmcompiler.o

$(OBJOUTPUT)/objv_vmcompiler_tokenizer.o:$(CORE_SRC)/objv_vmcompiler_tokenizer.c $(CORE)/objv_vmcompiler_tokenizer.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_tokenizer.c -o $(OBJOUTPUT)/objv_vmcompiler_tokenizer.o

$(OBJOUTPUT)/objv_vmcompiler_expression.o:$(CORE_SRC)/objv_vmcompiler_expression.c $(CORE)/objv_vmcompiler_expression.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_expression.c -o $(OBJOUTPUT)/objv_vmcompiler_expression.o

$(OBJOUTPUT)/objv_vmcompiler_invokes.o:$(CORE_SRC)/objv_vmcompiler_invokes.c $(CORE)/objv_vmcompiler_invokes.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_invokes.c -o $(OBJOUTPUT)/objv_vmcompiler_invokes.o
    
$(OBJOUTPUT)/objv_vmcompiler_class.o:$(CORE_SRC)/objv_vmcompiler_class.c $(CORE)/objv_vmcompiler_class.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_class.c -o $(OBJOUTPUT)/objv_vmcompiler_class.o
 
$(OBJOUTPUT)/objv_vmcompiler_meta.o:$(CORE_SRC)/objv_vmcompiler_meta.c $(CORE)/objv_vmcompiler_meta.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_meta.c -o $(OBJOUTPUT)/objv_vmcompiler_meta.o

$(OBJOUTPUT)/objv_vmcompiler_binary.o:$(CORE_SRC)/objv_vmcompiler_binary.c $(CORE)/objv_vmcompiler_binary.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vmcompiler_binary.c -o $(OBJOUTPUT)/objv_vmcompiler_binary.o

$(OBJOUTPUT)/objv_crypt.o:$(CORE_SRC)/objv_crypt.c $(CORE)/objv_crypt.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_crypt.c -o $(OBJOUTPUT)/objv_crypt.o

$(OBJOUTPUT)/md5.o:$(CORE_SRC)/md5.c $(CORE)/md5.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/md5.c -o $(OBJOUTPUT)/md5.o

$(OBJOUTPUT)/objv_vm.o:$(CORE_SRC)/objv_vm.c $(CORE)/objv_vm.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_vm.c -o $(OBJOUTPUT)/objv_vm.o

$(OBJOUTPUT)/objv_gc.o:$(CORE_SRC)/objv_gc.c $(CORE)/objv_gc.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_gc.c -o $(OBJOUTPUT)/objv_gc.o
    
$(OBJOUTPUT)/objv_inifile.o:$(CORE_SRC)/objv_inifile.c $(CORE)/objv_inifile.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_inifile.c -o $(OBJOUTPUT)/objv_inifile.o

$(OBJOUTPUT)/objv_channel.o:$(CORE_SRC)/objv_channel.c $(CORE)/objv_channel.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_channel.c -o $(OBJOUTPUT)/objv_channel.o

$(OBJOUTPUT)/objv_channel_tcp.o:$(CORE_SRC)/objv_channel_tcp.c $(CORE)/objv_channel_tcp.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_channel_tcp.c -o $(OBJOUTPUT)/objv_channel_tcp.o

$(OBJOUTPUT)/objv_actree.o:$(CORE_SRC)/objv_actree.c $(CORE)/objv_actree.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_actree.c -o $(OBJOUTPUT)/objv_actree.o

$(OBJOUTPUT)/objv_url.o:$(CORE_SRC)/objv_url.c $(CORE)/objv_url.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_url.c -o $(OBJOUTPUT)/objv_url.o

$(OBJOUTPUT)/objv_mail.o:$(CORE_SRC)/objv_mail.c $(CORE)/objv_mail.h
	cc $(CFLAGS) $(INCLUDE) -c $(CORE_SRC)/objv_mail.c -o $(OBJOUTPUT)/objv_mail.o


$(OBJOUTPUT)/objv_cloud.o:$(CLOUD)/objv_cloud.c $(CLOUD)/objv_cloud.h
	cc $(CFLAGS) $(INCLUDE) -c $(CLOUD)/objv_cloud.c -o $(OBJOUTPUT)/objv_cloud.o

$(OBJOUTPUT)/objv_clchannel.o:$(CLOUD)/objv_clchannel.c $(CLOUD)/objv_clchannel.h
	cc $(CFLAGS) $(INCLUDE) -c $(CLOUD)/objv_clchannel.c -o $(OBJOUTPUT)/objv_clchannel.o
 
$(OBJOUTPUT)/objv_clcontext.o:$(CLOUD)/objv_clcontext.c $(CLOUD)/objv_clcontext.h
	cc $(CFLAGS) $(INCLUDE) -c $(CLOUD)/objv_clcontext.c -o $(OBJOUTPUT)/objv_clcontext.o

$(OBJOUTPUT)/objv_clchannel_http.o:$(CLOUD)/objv_clchannel_http.c $(CLOUD)/objv_clchannel_http.h
	cc $(CFLAGS) $(INCLUDE) -c $(CLOUD)/objv_clchannel_http.c -o $(OBJOUTPUT)/objv_clchannel_http.o

$(OBJOUTPUT)/objv_clchannel_http.o:$(CLOUD)/objv_clchannel_http.c $(CLOUD)/objv_clchannel_http.h
	cc $(CFLAGS) $(INCLUDE) -c $(CLOUD)/objv_clchannel_http.c -o $(OBJOUTPUT)/objv_clchannel_http.o

clean:
	rm -rf $(OBJOUTPUT)/*.o $(TARGET) 




