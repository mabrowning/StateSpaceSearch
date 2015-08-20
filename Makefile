#Find all "*.h" files included in main.cpp
HEADERS=$(shell grep "include \".*\.h\"" main.cpp | sed 's/^.*"\(.*\)".*$$/\1/' )

all: puzzle_test_dbg
opt: puzzle_test
run: puzzle_test
	./puzzle_test
clean:
	@-rm puzzle_test puzzle_test_dbg *.svg *.out callgrind.*

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

CC=g++-mp-5
CFLAGS=-I cpp-sort/include --std=c++14 -g 

puzzle_test_dbg: main.cpp $(HEADERS)
	 $(CC) $(CFLAGS) $< -o $@
puzzle_test: main.cpp $(HEADERS)
	 $(CC) $(CFLAGS) -O3 $< -o $@

.PHONY: astar idastar rbfs kill trace_idastar trace_astar trace_rbfs
