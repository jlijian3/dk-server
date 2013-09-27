OBJECTS=dk_base_server.o dk_base_connection.o dk_http.o dk_util.o \
				dk_base_thread.o dk_ev_thread.o dk_worker.o thread_pool.o

CXXFLAGS=-g -Wall -D_DONKEY_DEBUG

LIB_NAME=libdk_base_server.a

$(LIB_NAME):$(OBJECTS)
		ar cru  $(LIB_NAME) $(OBJECTS)
		ranlib $(LIB_NAME)


SOURCES=dk_base_server.cpp dk_base_connection.cpp dk_http.cpp dk_util.cpp \
				dk_base_thread.cpp dk_ev_thread.cpp dk_worker.cpp thread_pool.cpp

DEPS=.depend

$(DEPS):$(SOURCES)
	makedepend -f- -I./ -Y $(SOURCES) 2> /dev/null > $(DEPS)

include $(DEPS)

.PHONY:clean
clean:
	rm $(LIB_NAME) $(OBJECTS)
	rm $(DEPS)
