#pragma once
#ifndef SLIDING_PUZZLE_H
#define SLIDING_PUZZLE_H
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

#include "hash.h"
	

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
	SlidingPuzzleAction( HoleDirection d = NUM_HoleDirection ) : dir(d){}
	bool operator==( const SlidingPuzzleAction & o ) const { return dir == o.dir; };
	bool operator!=( const SlidingPuzzleAction & o ) const { return dir != o.dir; };

	static SlidingPuzzleAction UP    ;
	static SlidingPuzzleAction DOWN  ;
	static SlidingPuzzleAction LEFT  ;
	static SlidingPuzzleAction RIGHT ;
};

SlidingPuzzleAction SlidingPuzzleAction::UP    = HOLE_UP;
SlidingPuzzleAction SlidingPuzzleAction::DOWN  = HOLE_DOWN;
SlidingPuzzleAction SlidingPuzzleAction::LEFT  = HOLE_LEFT;
SlidingPuzzleAction SlidingPuzzleAction::RIGHT = HOLE_RIGHT;

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

constexpr std::size_t factorial(std::size_t n) { 
	    return n == 0 ? 1  :  n * factorial(n-1); 
}

//N rows by M columns
template< unsigned int N, unsigned int M>
struct SlidingPuzzleState
{
	typedef SlidingPuzzleAction Action;

	typedef unsigned char index_t;
	constexpr static std::size_t NumStates = factorial( N * M );
	//API for AStarSolve
	Action::Actions AvailableActions( Action PrevAction = Action{}  ) const
	{
		Action::Actions ret = { nullptr, nullptr, nullptr, nullptr };
		auto act = ret.data();
		if( n > 0   && PrevAction != Action::DOWN  ) *(act++) = &Action::UP   ;
		if( n < N-1 && PrevAction != Action::UP    ) *(act++) = &Action::DOWN ;
		if( m > 0   && PrevAction != Action::RIGHT ) *(act++) = &Action::LEFT ;
		if( m < M-1 && PrevAction != Action::LEFT  ) *(act++) = &Action::RIGHT;

		return ret;
	}


	SlidingPuzzleState Apply( const Action & a ) const
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

	SlidingPuzzleState( const SlidingPuzzleState & o, Action::HoleDirection dir )
		: arr ( o.arr )
		, n   ( o.n )
		, m   ( o.m )
		, GoalDist( o.GoalDist )

	{
		//New location of hole
		switch( dir )
		{
			case Action::HOLE_UP:
				n -= 1;
				break;
			case Action::HOLE_DOWN:
				n += 1;
				break;
			case Action::HOLE_LEFT:
				m -= 1;
				break;
			case Action::HOLE_RIGHT:
				m += 1;
				break;
			default:
				break;
		}

		unsigned char oldval = 0;//arr[o.n][o.m];
		unsigned char newval = arr[  n][  m];


		//Apply to tile arrangement
		std::swap( arr[n][m], arr[o.n][o.m] );

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

#endif
