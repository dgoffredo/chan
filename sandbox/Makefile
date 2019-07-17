.PHONY: run
run: scratch
	./scratch

scratch: scratch.cpp ../build/libchan.a
	g++ -I ../build/src -o scratch -flto -O3 scratch.cpp -L ../build -lchan

../build/libchan.a:
	make -C ../build