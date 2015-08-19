#pragma once
#ifndef IDASTAR_SOLVE_H
#define IDASTAR_SOLVE_H

#include <vector>
#include <queue>
#include <limits>
#include <algorithm>

#include "solver.h"

template< typename State > 
struct IDAStar : public Solver< State >
{
	typedef typename State::Action Action;
	typedef Solver< State > Solver_t;
	using Solver_t::Solver;
	using Solver_t::GoalCostEstimate;
	using Solver_t::GoalTest;
	using Solver_t::PrintStatus;

std::vector< Action > Solve( const State & initial )
{

	struct StackFrame
	{
		struct Successor
		{
			Action* paction;
			State state;
			int g;
			int f;

			bool operator< ( const Successor & o ) const
			{
				//true = more first, false is more last
				if( paction == nullptr ) 
					return false; //nullest last
				if( f == o.f )
					return g < o.g; //shallowest first
				return f < o.f; //cheapest first
			}
		};
		std::array< Successor, Action::MaxBranch > successors;

		const State & state;
		int limit; //return
		int action_num;

		StackFrame( const State & s, const Action & prevAction, int g, 
				const IDAStar & solver )
			: state( s )
			, limit(std::numeric_limits<int>::max()) //inifinity
			, action_num( -1 )
		{
			auto actions = s.AvailableActions( prevAction );
			auto i = std::begin( successors );
			for( auto & paction : actions )
			{
				i->paction = paction;
				if( paction )
				{
					i->g      = g + paction->GetCost();
					i->state  = state.Apply( *paction );
					i->f      = i->g + solver.GoalCostEstimate( i->state );
				}
				++i;
			}
			std::sort( std::begin( successors ), std::end( successors ) );
		}
	};
	std::deque< StackFrame > Stack { { StackFrame{ initial, Action(), 0, *this }}  };

	int limit = GoalCostEstimate( initial ) ;
	int oldlimit = 0;

	unsigned int counter = 0;
	int deep_g = 0;

	while( !Stack.empty() && limit != oldlimit )
	{
		if( counter++ % 100 == 0 && PrintStatus() )
		{
			std::cerr << Stack.size() << " " << limit << " " << deep_g << std::endl;
		}
		StackFrame & top = Stack.back();
		auto & action_num = top.action_num;
		++action_num;

		//find the next non-null paction
		for( ; action_num < int(Action::MaxBranch) && 
				top.successors[action_num].paction == nullptr; ++action_num );
		if( action_num == int(Action::MaxBranch) )
		{
			if( Stack.size() == 1 )
			{
				//Update global limit
				oldlimit = limit;
				limit = top.limit;
				//Reset top
				top.action_num = 0;
				top.limit = std::numeric_limits<int>::max();
			}
			else
			{
				int lim = top.limit;
				Stack.pop_back();
				StackFrame & t = Stack.back();
				t.limit = std::min( t.limit, lim );
			}
			continue;
		}
		auto &successor = top.successors[ action_num ];

		if( GoalTest( successor.state ) ) 
		{
			//Done!
			std::vector< Action > ret;
			ret.reserve( Stack.size() + 1 );
			for( auto & SF : Stack )
				ret.push_back( *( SF.successors[ SF.action_num ].paction ) );
			return ret;
		}

		if( successor.f > limit )
			top.limit = std::min( top.limit, successor.f );
		else
		{
			Stack.emplace_back( successor.state, *successor.paction, successor.g, *this );
			if( successor.g >= deep_g )
				deep_g = successor.g + 1;
		}
	}


	return std::vector< Action >{};

}
};

#endif
