#pragma once
#ifndef SOLVER_H
#define SOLVER_H

//Base for State-space search solvers

#include <functional>
template< typename State > 
struct Solver
{
	std::function< int ( const State & ) > GoalCostEstimate;
	std::function< bool( const State & ) > GoalTest;
	std::function< bool(               ) > PrintStatus;

	Solver( 
		std::function< int ( const State & ) > && _GoalCostEstimate,
		std::function< bool( const State & ) > && _GoalTest,
		std::function< bool(               ) > && _PrintStatus )
		: GoalCostEstimate (std::move(_GoalCostEstimate))
		, GoalTest         (std::move(_GoalTest))       	
		, PrintStatus      (std::move(_PrintStatus))
	{}
};

#endif
