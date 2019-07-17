.PHONY: run clean

CXXFLAGS += -flto -O3 

run: scratch
	./scratch

clean:
	-rm scratch

scratch: scratch.cpp ../build/libchan.a
	g++ -I ../build/src $(CXXFLAGS) -o scratch scratch.cpp -L ../build -lchan

../build/libchan.a:
	make -C ../build