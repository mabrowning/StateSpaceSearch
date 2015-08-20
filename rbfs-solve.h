#pragma once
#ifndef RBFS_SOLVE_H
#define RBFS_SOLVE_H

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <stack>
#include "cpp-sort/sort.h"

template< typename State > 
struct RBFS
{
	bool PrintStatus = false;
	typedef typename State::Action Action;

	std::vector< Action > Solution;

	struct StateAndMeta
	{
		Action action;
		State state;

		int g;
		int f;
		int F;// = std::numeric_limits<int>::max();

		bool operator< ( const StateAndMeta & o ) const
		{
			return F < o.F; //cheapest first
		}
	};

	unsigned int counter = 0;
	unsigned int depth = 0;

	typedef std::array< StateAndMeta, Action::MaxBranch > Child_t;

	void InitChild( const StateAndMeta & n, Child_t & child )
	{
		auto actions = n.state.AvailableActions( n.action );
		auto i = std::begin( child );
		for( auto & paction : actions )
		{
			if( paction )
			{
				i->action = *paction;
				i->g      = n.g + paction->GetCost();
				i->state  = n.state.Apply( *paction );
				i->f      = i->g + i->state.EstGoalDist();
				if( n.f < n.F )
					i->F  = std::max( n.F, i->f );
				else
					i->F  = i->f;
				++i;
			}
		}
		while( i != std::end( child )) (i++)->F = std::numeric_limits<int>::max();
	}

	struct StackFrame
	{
		const StateAndMeta & n;
		int B;
		StackFrame( const StateAndMeta & _n, int _B ) : n( _n ), B( _B ) {}

		Child_t child;
	};

	void InitStackFrame( StackFrame & frame )
	{
		InitChild( frame.n, frame.child );
	}

std::vector< Action > Solve( const State & initial )
{
	if( initial.IsGoal() ) return Solution; //No actions to do

	int f = initial.EstGoalDist();
	StateAndMeta n{ Action{}, initial, 0/*g*/, f/*f*/, f/*F*/ };

	std::vector< StackFrame> Stack;
	Stack.reserve( 2*f );

	Stack.push_back( { n, f/*initial B*/ } );
	InitStackFrame( Stack.back() );

	while( true )
	{
		auto & frame = Stack.back();
		cppsort::sort( frame.child );

		auto & top = frame.child[0];

		if( ++counter % 100 == 0 && PrintStatus )
		{
			PrintStatus = false;
			std::cerr << counter << " " << Stack.size() << " " << top.F << " " << frame.B << std::endl;
		}

		if( top.F <= frame.B )
		{
			if( top.state.IsGoal() )
			{
				Solution.reserve( Stack.size() );
				for( auto it = Stack.begin(), end = Stack.end(); it != end; ++it  )
					Solution.push_back( it->child[0].action );
				break;
			}

			Stack.emplace_back( top, std::min( frame.B, frame.child[1].F ) );
			InitStackFrame( Stack.back() );
		}
		else
		{
			int ret = top.F;

			if( Stack.size() == 1 )
			{
				if( frame.B == ret )
					break;
				else
					frame.B = ret;
				continue;
			}

			Stack.pop_back();
			Stack.back().child[0].F = ret;
		}
	}
	return Solution;
}
};

#endif
