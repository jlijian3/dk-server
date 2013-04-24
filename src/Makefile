OBJECTS=donkey_server.o donkey_base_connection.o donkey_http.o donkey_util.o \
				donkey_base_thread.o donkey_ev_thread.o donkey_worker.o thread_pool.o

CXXFLAGS=-g -Wall -D_DONKEY_DEBUG

LIB_NAME=libdonkey_server.a

$(LIB_NAME):$(OBJECTS)
		ar cru  $(LIB_NAME) $(OBJECTS)
		ranlib $(LIB_NAME)


SOURCES=donkey_server.cpp donkey_base_connection.cpp donkey_http.cpp donkey_util.cpp \
				donkey_base_thread.cpp donkey_ev_thread.cpp donkey_worker.cpp thread_pool.cpp

DEPS=.depend

$(DEPS):$(SOURCES)
	makedepend -f- -I./ -Y $(SOURCES) 2> /dev/null > $(DEPS)

include $(DEPS)

.PHONY:clean
clean:
	rm $(LIB_NAME) $(OBJECTS)
	rm $(DEPS)
