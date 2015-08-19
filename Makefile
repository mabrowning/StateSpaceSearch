#Find all "*.h" files included in main.cpp
HEADERS=$(shell grep "include \".*\.h\"" main.cpp | sed 's/^.*"\(.*\)".*$$/\1/' ) solver.h

all: puzzle_test_dbg
opt: puzzle_test
run: puzzle_test
	./puzzle_test

astar: astar.out
idastar: idastar.out
rbfs: rbfs.out

astar.out: puzzle_test
	time ./puzzle_test | tee -i $@
idastar.out: puzzle_test
	time ./puzzle_test 'idastar' | tee -i $@
rbfs.out: puzzle_test
	time ./puzzle_test 'rbfs' | tee -i $@
kill:
	killall puzzle_test
status:
	killall -SIGUSR1 puzzle_test

trace_astar:   astar.svg
trace_idastar: idastar.svg
trace_rbfs:    rbfs.svg

%.svg : %.stacks
	~/bin/FlameGraph/stackcollapse.pl $< | ~/bin/FlameGraph/flamegraph.pl - > $@

%.stacks: puzzle_test ustacks.d
	-rm -f $@
	sudo ./ustacks.d -c './puzzle_test $(basename $@)' -o $@


puzzle_test_dbg: main.cpp $(HEADERS)
	 g++ main.cpp --std=c++11 -g -o puzzle_test_dbg
puzzle_test: main.cpp $(HEADERS)
	 g++ main.cpp --std=c++11 -g -O3 -o puzzle_test

.PHONY: astar idastar rbfs kill trace_idastar trace_astar trace_rbfs
