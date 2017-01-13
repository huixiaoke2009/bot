
CXX = g++
RM = rm -f
CP = cp -f
CXXFLAGS += -Wall -D_GNU_SOURCE  -g -DLINUX -O0 -fPIC -Wno-invalid-offsetof
INCS = -I. -I..
LIBS =  -lz -lcurl /usr/local/lib/libjson.a /usr/local/mysql/lib/libmysqlclient.a -lpthread -ldl

TARGETS = bot
all:$(TARGETS)

$(TARGETS):main.o mysql_wrap.o bot.o
	$(CXX) $(INCS) $^ $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) $< -o $@

clean:
	$(RM) $(TARGETS) *.o *.exe *.pb.h *.pb.cc
