#pragma once
#ifndef SOLVER_H
#define SOLVER_H

//Base for State-space search solvers

#include <functional>
template< typename State > 
struct Solver
{
	const std::function< int ( const State & ) > & GoalCostEstimate;
	const std::function< bool( const State & ) > & GoalTest;
	const std::function< bool(               ) > & PrintStatus;

	Solver( 
		std::function< int ( const State & ) > && _GoalCostEstimate,
		std::function< bool( const State & ) > && _GoalTest,
		std::function< bool(               ) > && _PrintStatus )
		: GoalCostEstimate (_GoalCostEstimate)
		, GoalTest         (_GoalTest)        	
		, PrintStatus      (_PrintStatus)
	{}
};

#endif
