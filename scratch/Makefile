CXX ?= g++
CXXFLAGS ?= -I. -g

.PHONY: run

scratch: scratch.o chai_lockguard.o chai_mutex.o chai_pipepair.o \
         chai_pipepool.o chai_error.o
	$(CXX) -o scratch $^

run: scratch
	./scratch