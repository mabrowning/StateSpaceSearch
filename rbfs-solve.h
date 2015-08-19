#pragma once
#ifndef RBFS_SOLVE_H
#define RBFS_SOLVE_H

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>

#include "solver.h"

template< typename State > 
struct RBFS : public Solver< State >
{
	typedef typename State::Action Action;
	typedef Solver< State > Solver_t;
	using Solver_t::Solver;
	using Solver_t::GoalCostEstimate;
	using Solver_t::GoalTest;
	using Solver_t::PrintStatus;

	std::vector< Action > Solution;

	struct StateAndMeta
	{
		Action* paction;
		State state;

		int g;
		int f;
		int F;

		bool operator< ( const StateAndMeta & o ) const
		{
			//true = more first, false is more last
			if( paction == nullptr ) 
				return false; //nullest last
			if( F == o.F )
				return g > o.g; //deepest first
			return F < o.F; //cheapest first
		}
	};

	unsigned int counter = 0;
	unsigned int depth = 0;

	int RBFS_step( const StateAndMeta & n, int B )
	{
		if( GoalTest( n.state ) )
		{
			Solution.resize(depth);
			return -1;
		}

		++depth;

		//if( counter++ % 100 == 0 && PrintStatus() )
		if( counter++ % 100 == 0 )
		{
			std::cerr << depth << " " << n.F << std::endl;
		}

		std::array< StateAndMeta, Action::MaxBranch > child;

		auto actions = n.state.AvailableActions();
		auto i = std::begin( child );
		for( auto & paction : actions )
		{
			i->paction = paction;
			if( paction )
			{
				i->g      = n.g + paction->GetCost();
				i->state  = n.state.Apply( *paction );
				i->f      = i->g + GoalCostEstimate( i->state );
				if( n.f < n.F )
					i->F  = std::max( n.F, i->f );
				else
					i->F  = i->f;
			}
			++i;
		}

		std::sort( std::begin( child ), std::end( child ) );
		while( child[0].F <= B )
		{
			auto & top = child[0];

			top.F = RBFS_step( top, std::min( B, child[1].F ) );
			if( top.F == -1 )
			{
				Solution[depth-1] = *top.paction;
				return -1;
			}

			std::sort( std::begin( child ), std::end( child ) );
		} 

		--depth;
		return child[0].F;
	}

std::vector< Action > Solve( const State & initial )
{
	int f = GoalCostEstimate( initial );
	StateAndMeta n{ nullptr, initial, 0/*g*/, f/*f*/, f/*F*/ };
	int B = f;
	while( true )
	{
		B = RBFS_step( n, B );
		if( B == -1 ) break;
	}
	
	return Solution;

}
};

#endif
