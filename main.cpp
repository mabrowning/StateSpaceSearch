#include <array>
#include <vector>
#include <iostream>
#include <iomanip>
#include <csignal>
#include <unistd.h>

#include "hash.h"
#include "astar-solve.h"
#include "idastar-solve.h"
#include "rbfs-solve.h"

namespace
{
	  volatile std::sig_atomic_t gSignalStatus;
}
 
void signal_handler(int signal)
{
	  gSignalStatus = signal;
}

bool PrintStatus() 
{
	if( gSignalStatus > 0 )
	{
		gSignalStatus = 0;
		return true;
	}
	return false;
}
	

struct SlidingPuzzleAction
{
	enum HoleDirection
	{
		HOLE_UP,
		HOLE_DOWN,
		HOLE_LEFT,
		HOLE_RIGHT,

		NUM_HoleDirection
		
	};

	constexpr static std::size_t MaxBranch   = NUM_HoleDirection; //maximum out-degree of state graph
	constexpr static std::size_t MaxInBranch = NUM_HoleDirection; //maximum in-degree of state graph. This puzzle is reversible, so symmetric, so in=out

	typedef std::array< SlidingPuzzleAction*, MaxBranch > Actions;

	int GetCost() const { return 1; }

	HoleDirection dir;
	SlidingPuzzleAction( HoleDirection d = HOLE_UP ) : dir(d){}
	bool operator==( const SlidingPuzzleAction & o ) const { return dir == o.dir; };

	static SlidingPuzzleAction UP    ;
	static SlidingPuzzleAction DOWN  ;
	static SlidingPuzzleAction LEFT  ;
	static SlidingPuzzleAction RIGHT ;
};

SlidingPuzzleAction SlidingPuzzleAction::UP    = HOLE_UP;
SlidingPuzzleAction SlidingPuzzleAction::DOWN  = HOLE_DOWN;
SlidingPuzzleAction SlidingPuzzleAction::LEFT  = HOLE_LEFT;
SlidingPuzzleAction SlidingPuzzleAction::RIGHT = HOLE_RIGHT;

constexpr std::size_t factorial(std::size_t n) { 
	    return n == 0 ? 1  :  n * factorial(n-1); 
}

//N rows by M columns
template< unsigned int N, unsigned int M>
struct SlidingPuzzleState
{
	typedef unsigned char index_t;
	constexpr static std::size_t NumStates = factorial( N * M );
	//API for AStarSolve
	SlidingPuzzleAction::Actions AvailableActions() const
	{
		SlidingPuzzleAction::Actions ret = { nullptr, nullptr, nullptr, nullptr };
		auto act = ret.data();
		if( n > 0   ) *(act++) = &SlidingPuzzleAction::UP   ;
		if( n < N-1 ) *(act++) = &SlidingPuzzleAction::DOWN ;
		if( m > 0   ) *(act++) = &SlidingPuzzleAction::LEFT ;
		if( m < M-1 ) *(act++) = &SlidingPuzzleAction::RIGHT;

		return ret;
	}


	SlidingPuzzleState Apply( const SlidingPuzzleAction & a ) const
	{
		//Use state-applying constructor
		return SlidingPuzzleState( *this, a.dir );
	}

	//Implementation details
	
	//N rows of M columns
	typedef std::array< std::array< unsigned char, M>, N > arr_t;
	arr_t arr;

	SlidingPuzzleState( )
		: m( 0 )
	    , n( 0 )
		, GoalDist( 0 )
	{
		//Initial state, all in order
		unsigned char v=0;
		for( auto & row : arr )
			for( auto & val : row )
				val = v++;
	}


	//coords of 0
	index_t n,m;

	int EstGoalDist() const { return GoalDist; };

	bool IsGoal() const 
	{
		return GoalDist == 0;
	}

	private:
	int DoEstGoalDist() const 
	{
		//Esimate the "number of moves" needed to get to the goal state
		int dist = 0;
		for( index_t n = 0; n < N; ++n )
		{
			for( index_t m = 0; m < M; ++m )
			{
				//n,m are the actual coordinates
				//The value at this index
				auto val = arr[n][m];
				index_t n_ = val / M;
				index_t m_ = val % M;
				//n_,m_ are the coordinates of val

				//manhattan distance of moves to put this piece where it belongs
				dist += abs( n_ - n ) + abs( m_ - m );
			}
		}
		return dist;
	}

	int GoalDist;// = DoEstGoalDist();

	SlidingPuzzleState( const SlidingPuzzleState & o, SlidingPuzzleAction::HoleDirection dir )
		: arr ( o.arr )
		, n   ( o.n )
		, m   ( o.m )
		, GoalDist( o.GoalDist )

	{
		//New location of hole
		switch( dir )
		{
			case SlidingPuzzleAction::HOLE_UP:
				n -= 1;
				break;
			case SlidingPuzzleAction::HOLE_DOWN:
				n += 1;
				break;
			case SlidingPuzzleAction::HOLE_LEFT:
				m -= 1;
				break;
			case SlidingPuzzleAction::HOLE_RIGHT:
				m += 1;
				break;
			default:
				break;
		}

		unsigned char oldval = 0;//arr[o.n][o.m];
		unsigned char newval = arr[  n][  m];


		//Apply to tile arrangement
		std::swap( arr[n][m], arr[o.n][o.m] );

		int test = DoEstGoalDist();

		unsigned char old_n = oldval / M;
		unsigned char old_m = oldval % M; 

		unsigned char new_n = newval / M;
		unsigned char new_m = newval % M; 

		unsigned char lessDist = ( abs( old_n - o.n ) + abs( old_m - o.m ) + abs( new_n -   n ) + abs( new_m -   m ) );
		unsigned char moreDist = ( abs( old_n -   n ) + abs( old_m -   m ) + abs( new_n - o.n ) + abs( new_m - o.m ) );

		GoalDist -= lessDist; 
		GoalDist += moreDist; 
	}
};

template<unsigned N, unsigned M> 
bool operator==( const SlidingPuzzleState<N,M> & lhs, const SlidingPuzzleState<N,M> & rhs )
{
	return lhs.n == rhs.n && lhs.m == rhs.m && lhs.arr == rhs.arr;
}

namespace std
{
	template<unsigned N, unsigned M> struct hash< SlidingPuzzleState<N,M> >
	{
		size_t operator() ( const SlidingPuzzleState<N,M> & state ) const
		{
			return hash< typename SlidingPuzzleState<N,M>::arr_t >()( state.arr ) ;
		}
	};
}

template< unsigned int N, unsigned int M>
std::ostream & operator<<(std::ostream &os, const SlidingPuzzleState<N,M> & t )
{
	for( auto & row : t.arr )
	{
		for( auto & val : row )
			os << std::setw(2) << (int)val << " ";
		os << "\n";
	}
	os << std::endl;

	return os;
}

std::ostream & operator<<(std::ostream &os, const SlidingPuzzleAction & a )
{
	switch( a.dir )
	{
		case SlidingPuzzleAction::HOLE_UP:
			os << "UP";
			break;
		case SlidingPuzzleAction::HOLE_DOWN:
			os << "DOWN";
			break;
		case SlidingPuzzleAction::HOLE_LEFT:
			os << "LEFT";
			break;
		case SlidingPuzzleAction::HOLE_RIGHT:
			os << "RIGHT";
			break;
		default:
			break;
	}
	return os;
}

template<typename State, typename Action>
State GetRandomInitialState( State state  )
{
	for( int i = 0 ; i < 10000; ++i )
	{
		auto actions = state.AvailableActions();
		auto action = actions[0];
		while( ( action = actions[rand()%actions.size()] ) == nullptr );

		state = state.Apply( *action );
	}

	return state;
}


int main( int argc, char** argv )
{

	std::signal( SIGINT, signal_handler );

	typedef SlidingPuzzleState<4,4> State_t;

	auto initial = GetRandomInitialState< State_t, SlidingPuzzleAction>( State_t() );

	//Solve that puzzle
	bool idast = !( argc == 2 && strcmp( argv[1], "ida*" ) == 0 );
	bool rbfs  = !( argc == 2 && strcmp( argv[1], "rbfs" ) == 0 );
	std::vector< SlidingPuzzleAction> Solution;
	if( idast )
	{
		auto Solver = IDAStar<State_t, SlidingPuzzleAction>{
				std::mem_fn( &State_t::EstGoalDist ), //heuristic
				std::mem_fn( &State_t::IsGoal ),	  //GoalTest
				PrintStatus };
		//Solution = Solver.Solve( initial );
	}
	else if( rbfs ) 
	{
		auto Solver = RBFS<State_t, SlidingPuzzleAction>{
				std::mem_fn( &State_t::EstGoalDist ), //heuristic
				std::mem_fn( &State_t::IsGoal ),	  //GoalTest
				PrintStatus };
		//Solution = Solver.Solve( initial );
	}
	else
	{
		auto Solver = AStar<State_t, SlidingPuzzleAction>{
				std::mem_fn( &State_t::EstGoalDist ), //heuristic
				std::mem_fn( &State_t::IsGoal ),	  //GoalTest
				PrintStatus };
		//Solution = Solver.Solve( initial );
	}

	std::cout << Solution.size() << std::endl;
	return 0;

	auto state = initial;
	std::cout << state;
	for( auto action : Solution )
	{
		state = state.Apply( action );
		std::cout << action << "->\n" << state;
	}
	return 0;
}

