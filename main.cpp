
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

#include "hash.h"
#include "astar-solve.h"

struct SlidingPuzzleAction
{
	enum HoleDirection
	{
		HOLE_UP,
		HOLE_DOWN,
		HOLE_LEFT,
		HOLE_RIGHT
	};

	int GetCost() const { return 1; }

	HoleDirection dir;
	SlidingPuzzleAction( HoleDirection d = HOLE_UP ) : dir(d){}
};

constexpr std::size_t factorial(std::size_t n) { 
	    return n == 0 ? 1  :  n * factorial(n-1); 
}

//N rows by M columns
template< unsigned int N, unsigned int M>
struct SlidingPuzzleState
{
	constexpr static std::size_t NumStates = factorial( N * M );
	//API for AStarSolve
	std::vector< SlidingPuzzleAction > AvailableActions() const
	{
		std::vector< SlidingPuzzleAction > ret;
		if( n > 0   ) ret.push_back( SlidingPuzzleAction::HOLE_UP    );
		if( n < N-1 ) ret.push_back( SlidingPuzzleAction::HOLE_DOWN  );
		if( m > 0   ) ret.push_back( SlidingPuzzleAction::HOLE_LEFT  );
		if( m < M-1 ) ret.push_back( SlidingPuzzleAction::HOLE_RIGHT );

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
		unsigned int v=0;
		for( auto & row : arr )
			for( auto & val : row )
				val = v++;
	}


	//coords of 0
	int n,m;

	int GetGoalDist() const { return GoalDist; };

	bool IsGoal() const 
	{
		return GoalDist == 0;
	}

	private:
	int DoGetGoalDist() const 
	{
		//Esimate the "number of moves" needed to get to the goal state
		int dist = 0;
		for( int n = 0; n < N; ++n )
		{
			for( int m = 0; m < M; ++m )
			{
				//n,m are the actual coordinates
				//The value at this index
				auto val = arr[n][m];
				int n_ = val / M;
				int m_ = val % M;
				//n_,m_ are the coordinates of val

				//manhattan distance of moves to put this piece where it belongs
				dist += abs( n_ - n ) + abs( m_ - m );
			}
		}
		return dist;
	}

	int GoalDist;// = DoGetGoalDist();

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
		}

		unsigned char oldval = 0;//arr[o.n][o.m];
		unsigned char newval = arr[  n][  m];


		//Apply to tile arrangement
		std::swap( arr[n][m], arr[o.n][o.m] );

		int test = DoGetGoalDist();

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
	}
	return os;
}


int main( int argc, char** argv )
{

	typedef SlidingPuzzleState<4,4> State_t;

	//Generate an initial (valid) puzzle randomly
	State_t state;
	State_t maxState;
	int maxDist = 0;

	//Apply 100 random state transitions
	for( int i = 0 ; i < 10000; ++i )
	{
		auto actions = state.AvailableActions();
		auto action =  actions[rand()%actions.size()];

		state = state.Apply( action );
		int dist = state.GetGoalDist( ); 
		if( dist > maxDist )
		{
			maxDist  = dist;
			maxState = state;
		}

		/*
		std::cout << action << "->\n" << state;
		std::cout << dist << std::endl;
		*/
	}

	std:: cout << maxState << maxDist << std::endl;


	state = maxState;

	//Solve that puzzle
	auto Solution = AStar< State_t, SlidingPuzzleAction>::Solve(
			maxState, 
			std::mem_fn( &State_t::GetGoalDist ), //Evaluator
			std::mem_fn( &State_t::IsGoal )		  //GoalTest
			); 

	std::cout << "Solution has " << Solution.size() << " moves" << std::endl;

	for( auto action : Solution )
	{
		state = state.Apply( action );
		std::cout << action << "->\n" << state;
	}

	return 0;
}

