
LIBS=../libdk_base_server.a
CXXFLAGS=-g -I.. -D_DONKEY_DEBUG
LDFLAGS=-lrt -levent -levent_pthreads

PROGS=echo_srv http_client tcp_client
all:$(PROGS)

.PHONY:all

echo_srv:echo_srv.o $(LIBS)
	g++ $(CXXFLAGS) -o echo_srv echo_srv.o $(LIBS) $(LDFLAGS)

http_client:http_client.o $(LIBS)
	g++ $(CXXFLAGS) -o http_client http_client.o $(LIBS) $(LDFLAGS)

tcp_client:tcp_client.o $(LIBS)
	g++ $(CXXFLAGS) -o tcp_client tcp_client.o $(LIBS) $(LDFLAGS)

.PHONY:clean
clean:
	rm *.o $(PROGS)
