#include <vector>

#include <csignal>
#include <cstring>
#include <unistd.h>

#include "sliding-puzzle.h"

#include "astar-solve.h"
#include "idastar-solve.h"
#include "rbfs-solve.h"

namespace
{
	  volatile std::sig_atomic_t gSignalStatus;
}

bool* gPrintStatus = nullptr;
 
void signal_handler(int signal)
{
	if( gPrintStatus ) *gPrintStatus = true;
}

template<typename State>
State GetRandomInitialState( State state, int max = 50  )
{
	state.init();
	typename State::Action lastAction; //default
	for( int i = 0 ; i < max; ++i )
	{
		auto actions = state.AvailableActions( lastAction );
		auto paction = actions[0];
		while( ( paction = actions[rand()%actions.size()] ) == nullptr );

		state = state.Apply( *paction );
		lastAction = *paction;
	}

	return state;
}


int main( int argc, char** argv )
{

	std::signal( SIGUSR1, signal_handler );

	typedef SlidingPuzzleState<5,5> State_t;

	auto initial = GetRandomInitialState( State_t(), 100 );

	//Solve that puzzle
	bool idast = ( argc > 1 && strcmp( argv[1], "idastar" ) == 0 );
	bool rbfs  = ( argc > 1 && strcmp( argv[1], "rbfs"    ) == 0 );
	std::vector< State_t::Action > Solution;
	if( idast )
	{
		auto Solver = IDAStar<State_t >{};
		gPrintStatus = &Solver.PrintStatus;
		Solution = Solver.Solve( initial );
	}
	else if( rbfs ) 
	{
		auto Solver = RBFS<State_t>{};
		gPrintStatus = &Solver.PrintStatus;
		Solution = Solver.Solve( initial );
	}
	else
	{
		auto Solver = AStar<State_t>{};
		gPrintStatus = &Solver.PrintStatus;
		Solution = Solver.Solve( initial );
	}

	std::cout << Solution.size() << std::endl;

	auto state = initial;
	std::cout << state;
	for( auto action : Solution )
	{
		state = state.Apply( action );
		std::cout << action << "->\n" << state;
	}
	return 0;
}

