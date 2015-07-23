all: puzzle_test_dbg
opt: puzzle_test
run: puzzle_test
	./puzzle_test

trace: trace.svg

out.stacks: puzzle_test ustacks.d
	@-rm -f out.stacks
	sudo ./ustacks.d -c ./puzzle_test -o out.stacks

trace.svg: out.stacks
	@-rm trace.svg
	~/bin/FlameGraph/stackcollapse.pl out.stacks | ~/bin/FlameGraph/flamegraph.pl - > trace.svg



puzzle_test_dbg: main.cpp astar-solve.h hash.h
	 g++ main.cpp --std=c++11 -g -o puzzle_test_dbg
puzzle_test: main.cpp astar-solve.h hash.h
	 g++ main.cpp --std=c++11 -g -O3 -o puzzle_test

