
CXXSTD=c++11
CXX=g++ -std=$(CXXSTD)
CFLAGS= -O0 -g -ggdb -Wall -Wextra
LDFLAGS= $(SYSLDFLAGS) $(MYLDFLAGS)
LUA_ROOT= /usr/local
ATFRAME_UTILS= .
LIBS= -lm -llua

RM= rm -f

SAMPLE_T= sample
SAMPLE_C= sample.cpp

LUA_NATIVE_SRC= $(wildcard ../lua/*/*.cpp)

ALL_T= $(SAMPLE_T)
ALL_C= $(SAMPLE_C) $(LUA_NATIVE_SRC)

default: $(ALL_T)

all:	$(ALL_T)

$(ALL_T):
	$(CXX) -I$(LUA_ROOT)/include -L$(LUA_ROOT)/lib -I$(ATFRAME_UTILS)/include -I.. -o $@ $(LDFLAGS) $(ALL_C) $(LIBS)

clean:
	$(RM) $(ALL_T)

echo:
	@echo "CXX= $(CXX)"
	@echo "CFLAGS= $(CFLAGS)"
	@echo "LDFLAGS= $(SYSLDFLAGS)"
	@echo "LIBS= $(LIBS)"
	@echo "LUA_ROOT= $(LUA_ROOT)"
	@echo "ATFRAME_UTILS= $(ATFRAME_UTILS)"
	@echo "LUA_NATIVE_SRC= $(LUA_NATIVE_SRC)"


ALL= all
