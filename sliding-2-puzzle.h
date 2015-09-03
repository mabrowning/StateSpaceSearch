#pragma once
#ifndef SLIDING_2_PUZZLE_H
#define SLIDING_2_PUZZLE_H
#include <array>
#include <vector>
#include <iostream>
#include <iomanip>

#include "hash.h"
	

struct Sliding2PuzzleAction
{
	enum HoleDirection
	{
		HOLE_UP,
		HOLE_DOWN,
		HOLE_LEFT,
		HOLE_RIGHT,

		NUM_HoleDirection
		
	};

	constexpr static std::size_t MaxBranch   = 2*NUM_HoleDirection; //maximum out-degree of state graph
	constexpr static std::size_t MaxInBranch = 2*NUM_HoleDirection; //maximum  in-degree of state graph. This puzzle is reversible, so symmetric, so in=out

	typedef std::array< Sliding2PuzzleAction*, MaxBranch > Actions;

	int GetCost() const { return 1+hole; }

	HoleDirection dir;
	char hole;
	Sliding2PuzzleAction( HoleDirection d = NUM_HoleDirection, char h = 0 ) : dir(d), hole(h){}
	bool operator==( const Sliding2PuzzleAction & o ) const { return dir == o.dir && hole == o.hole; };
	bool operator!=( const Sliding2PuzzleAction & o ) const { return dir != o.dir || hole != o.hole; };

	static Sliding2PuzzleAction UP0   ;
	static Sliding2PuzzleAction DOWN0 ;
	static Sliding2PuzzleAction LEFT0 ;
	static Sliding2PuzzleAction RIGHT0;
	static Sliding2PuzzleAction UP1   ;
	static Sliding2PuzzleAction DOWN1 ;
	static Sliding2PuzzleAction LEFT1 ;
	static Sliding2PuzzleAction RIGHT1;
};

Sliding2PuzzleAction Sliding2PuzzleAction::UP0   = Sliding2PuzzleAction{ HOLE_UP,    0 };
Sliding2PuzzleAction Sliding2PuzzleAction::DOWN0 = Sliding2PuzzleAction{ HOLE_DOWN,  0 };
Sliding2PuzzleAction Sliding2PuzzleAction::LEFT0 = Sliding2PuzzleAction{ HOLE_LEFT,  0 };
Sliding2PuzzleAction Sliding2PuzzleAction::RIGHT0= Sliding2PuzzleAction{ HOLE_RIGHT, 0 };

Sliding2PuzzleAction Sliding2PuzzleAction::UP1   = Sliding2PuzzleAction{ HOLE_UP,    1 };
Sliding2PuzzleAction Sliding2PuzzleAction::DOWN1 = Sliding2PuzzleAction{ HOLE_DOWN,  1 };
Sliding2PuzzleAction Sliding2PuzzleAction::LEFT1 = Sliding2PuzzleAction{ HOLE_LEFT,  1 };
Sliding2PuzzleAction Sliding2PuzzleAction::RIGHT1= Sliding2PuzzleAction{ HOLE_RIGHT, 1 };

std::ostream & operator<<(std::ostream &os, const Sliding2PuzzleAction & a )
{
	switch( a.dir )
	{
		case Sliding2PuzzleAction::HOLE_UP:
			os << "UP" << int(a.hole);
			break;
		case Sliding2PuzzleAction::HOLE_DOWN:
			os << "DOWN"<<int(a.hole);
			break;
		case Sliding2PuzzleAction::HOLE_LEFT:
			os << "LEFT"<<int(a.hole);
			break;
		case Sliding2PuzzleAction::HOLE_RIGHT:
			os << "RIGHT"<<int(a.hole);
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
struct Sliding2PuzzleState
{
	typedef Sliding2PuzzleAction Action;

	typedef unsigned char index_t;
	constexpr static std::size_t NumStates = factorial( N * M );
	//API for AStarSolve
	Action::Actions AvailableActions( Action PrevAction = Action{}  ) const
	{
		Action::Actions ret = { nullptr, nullptr, nullptr, nullptr, 
								nullptr, nullptr, nullptr, nullptr };
		auto act = ret.data();
		bool up    = n0 + 1 == n1 && m0 == m1; // 0 is currently above 1
		bool down  = n0 - 1 == n1 && m0 == m1; // 0 is currently below 1
		bool left  = m0 + 1 == m1 && n0 == n1; // 0 is currently to the left  of 1
		bool right = m0 - 1 == m1 && n0 == n1; // 0 is currently to the right of 1

		if( n0 > 0   && PrevAction != Action::DOWN0  ) *(act++) = &Action::UP0   ;
		if( n0 < N-1 && PrevAction != Action::UP0    ) *(act++) = &Action::DOWN0 ;
		if( m0 > 0   && PrevAction != Action::RIGHT0 ) *(act++) = &Action::LEFT0 ;
		if( m0 < M-1 && PrevAction != Action::LEFT0  ) *(act++) = &Action::RIGHT0;

		if( !up    && n1 > 0   && PrevAction != Action::DOWN1  ) *(act++) = &Action::UP1   ;
		if( !down  && n1 < N-1 && PrevAction != Action::UP1    ) *(act++) = &Action::DOWN1 ;
		if( !left  && m1 > 0   && PrevAction != Action::RIGHT1 ) *(act++) = &Action::LEFT1 ;
		if( !right && m1 < M-1 && PrevAction != Action::LEFT1  ) *(act++) = &Action::RIGHT1;

		return ret;
	}


	Sliding2PuzzleState Apply( const Action & a ) const
	{
		//Use state-applying constructor
		return Sliding2PuzzleState( *this, a );
	}

	//Implementation details
	
	//N rows of M columns
	typedef std::array< std::array< unsigned char, M>, N > arr_t;
	arr_t arr;

	Sliding2PuzzleState( )
		: m0( 0 )
	    , n0( 0 )
		, m1( 1 )
	    , n1( 0 )
		, GoalDist( 0 )
	{
	}
	void init()
	{
		//Initial state, all in order
		unsigned char v=0;
		for( auto & row : arr )
			for( auto & val : row )
				val = v++;
	}


	//coords of 0,1
	index_t n0,m0,n1,m1;

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

	Sliding2PuzzleState( const Sliding2PuzzleState & o, const Action & act )
		: arr ( o.arr )
		, n0  ( o.n0 )
		, m0  ( o.m0 )
		, n1  ( o.n1 )
		, m1  ( o.m1 )
		, GoalDist( o.GoalDist )

	{
		auto &  n = act.hole ?   n1 :   n0;
		auto &  m = act.hole ?   m1 :   m0;
		auto & on = act.hole ? o.n1 : o.n0;
		auto & om = act.hole ? o.m1 : o.m0;

		//New location of hole
		switch( act.dir )
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

		unsigned char oldval = arr[on][om];
		unsigned char newval = arr[ n][ m];

		if( act.hole )
		{
			//moving 1
			if( newval == 0 )
			{
				//swapping with 0
				//update n0,m0 to onm
				n0 = on;
				m0 = om;
			}
		}
		else
		{
			//moving 0
			if( newval == 1 )
			{
				//swapping with 1
				//update n1,m1 to onm
				n1 = on;
				m1 = om;
			}
		}

		//Apply to tile arrangement
		std::swap( arr[n][m], arr[on][om] );

		unsigned char old_n = oldval / M;
		unsigned char old_m = oldval % M; 

		unsigned char new_n = newval / M;
		unsigned char new_m = newval % M; 

		unsigned char lessDist = ( abs( old_n - on ) + abs( old_m - om ) + abs( new_n -  n ) + abs( new_m -  m ) );
		unsigned char moreDist = ( abs( old_n -  n ) + abs( old_m -  m ) + abs( new_n - on ) + abs( new_m - om ) );

		GoalDist -= lessDist; 
		GoalDist += moreDist; 
	}
};

template<unsigned N, unsigned M> 
bool operator==( const Sliding2PuzzleState<N,M> & lhs, const Sliding2PuzzleState<N,M> & rhs )
{
	return lhs.n0 == rhs.n0 && lhs.m0 == rhs.m0 && 
		   lhs.n1 == rhs.n1 && lhs.m1 == rhs.m1 && 
		   lhs.arr == rhs.arr;
}

namespace std
{
	template<unsigned N, unsigned M> struct hash< Sliding2PuzzleState<N,M> >
	{
		size_t operator() ( const Sliding2PuzzleState<N,M> & state ) const
		{
			return hash< typename Sliding2PuzzleState<N,M>::arr_t >()( state.arr ) ;
		}
	};
}

template< unsigned int N, unsigned int M>
std::ostream & operator<<(std::ostream &os, const Sliding2PuzzleState<N,M> & t )
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
