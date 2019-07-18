.PHONY: run clean

BUILD_TYPE ?= Release

CXXFLAGS += -Wall -Wextra -pedantic -Werror \
            -Wno-unused-function -Wno-variadic-macros

ifeq (BUILD_TYPE, Release)
    CXXFLAGS += -flto -O3
else
    CXXFLAGS += -g -O0 # -Og
endif

scratch: scratch.cpp ../build/libchan.a
	g++ -I ../build/src $(CPPFLAGS) $(CXXFLAGS) -o scratch scratch.cpp \
	    -L ../build -lchan -lpthread

../build/libchan.a:
	make -C ../build

clean:
	-rm scratch
