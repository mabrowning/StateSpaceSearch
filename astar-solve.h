#pragma once
#ifndef ASTAR_SOLVE_H
#define ASTAR_SOLVE_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <functional>

template< typename State > 
struct AStar
{ 
	bool PrintStatus = false;

	typedef typename State::Action Action;

	//forward declares
	struct MetaData;

	typedef std::pair< const State, MetaData > StateAndMeta;

	struct MetaData
	{
		//Default to "infinite" distance
		int cost_so_far = std::numeric_limits<int>::max() ;

		int num_references = 0;

		Action parent_action;
		StateAndMeta * parent_entry = nullptr;
	};

	struct QueueEntry
	{
		std::reference_wrapper< StateAndMeta > state_and_meta;
		int Priority;

		bool operator<( const QueueEntry & o ) const { return Priority > o.Priority; }//Invert logic, as priority_queue returns max element according to "<"
	};


	struct StatesHashTable
	{
		typedef std::unordered_map< State, MetaData > HashTable;
		//Stores 1 set of metadata about each state
		HashTable hash_table; //Known states and their data
		typename HashTable::reference get( const State & state )
		{
			//unordered_map.insert returns std::pair< iterator, bool >
			//iterator dereferences to type HashTable::reference
			auto it_bool = hash_table.insert( { state, MetaData{} } );
			typename HashTable::reference ret = *it_bool.first;
			auto it = it_bool.first;
			if( it_bool.second )
			{
				//Freshly inserted
				//ret.actions = state.AvailableActions();
			}

			return ret;
		}
	};

	template< typename Bucket, bool prefer_min = true >
	struct PriorityQueue1LevelBucket
	{
		std::vector< Bucket > queue;

		//Either min or max
		std::size_t next_priority = std::numeric_limits< std::size_t >::max();

		bool empty() const
		{
			return next_priority == std::numeric_limits< std::size_t >::max();
		}
		
		auto front() -> decltype( queue[ next_priority ].front() )
		{
			return queue[ next_priority ].front();
		}

		void pop_front()
		{
			queue[ next_priority ].pop_front();

			if( prefer_min )
			{
				//search forward for the next non-empty bucket
				auto ref = queue.begin();
				auto end = queue.end();
				auto it  = ref + next_priority;
				while( it != end && it->empty() ) ++it;
				if( it == end ) next_priority = std::numeric_limits< std::size_t >::max();
				else            next_priority = it - ref;
			}
			else
			{
				//search backward
				auto ref = queue.rend() - 1;
				auto end = queue.rend();
				auto it  = ref - next_priority;

				while( it != end && it->empty() ) ++it;
				if( it == end ) next_priority = std::numeric_limits< std::size_t >::max();
				else            next_priority = ref - it;

			}
		}

		Bucket & get_bucket( std::size_t priority )
		{
			if( priority >= queue.size() )
				queue.resize( priority + 1 );

			if( prefer_min )
			{
				if( priority < next_priority )
					next_priority = priority;
			}
			else
			{
				if( priority > next_priority || empty() )
					next_priority = priority;
			}
			return queue[ priority ];

		}
	};

	struct PriorityQueue : public 
		PriorityQueue1LevelBucket< 
			PriorityQueue1LevelBucket< 
				std::deque< std::reference_wrapper< StateAndMeta > >, true   //Inner queue prefers max
			> >
	{
		std::size_t m_size = 0;
		std::size_t size() const { return m_size; }

		void pop()
		{
			m_size--;
			this->pop_front();
		}

		void insert( StateAndMeta & value, std::size_t f, std::size_t g )
		{
			m_size++;
			this->get_bucket( f ).get_bucket( g ).push_back( value );
		}

	};


std::vector< Action > Solve( const State & initial )
{

	StatesHashTable                         States;
	PriorityQueue Frontier;


	//Add the initial state at 0 cost, returning iterator
	StateAndMeta & initial_state_and_meta = States.get( initial );
	initial_state_and_meta.second.cost_so_far = 0;

	//Create priority queue

	//Add initial state
	Frontier.insert( initial_state_and_meta, 0, 0 );

	StateAndMeta * Final = &initial_state_and_meta;

	std::size_t numChecks = 0;

	while( !Frontier.empty() )
	{
		StateAndMeta & state_and_meta = Frontier.front();
		Frontier.pop();//Ok to keep state_and_meta since Frontier only stores a reference_wrapper

		const State & state = state_and_meta.first;
		MetaData    & meta  = state_and_meta.second;

		meta.num_references--;

		if( state.IsGoal() )
		{
			Final = &state_and_meta;
			break;
		}

		//if( meta.num_references > 0 ) continue;

		for( auto &paction : state.AvailableActions() )
		{
			if( !paction ) continue;
			auto & action = *paction;

			//See what the new state is after applying the action
			State new_state   = state.Apply( action );

			//inserts if doesn't exist and returns reference
			StateAndMeta & new_state_and_meta = States.get( new_state );

			MetaData    & new_meta  = new_state_and_meta.second;

			int new_cost = meta.cost_so_far + action.GetCost(); //g
			if( new_cost < new_meta.cost_so_far )
			{
				new_meta.cost_so_far = new_cost;
				new_meta.parent_action = action;
				new_meta.parent_entry  = &state_and_meta;
				new_meta.num_references++;

				int new_priority = new_cost + new_state.EstGoalDist();
				Frontier.insert( new_state_and_meta, new_priority, new_cost );
			}

		}

		if( ++numChecks % 100 == 0 && PrintStatus ) 
		{
			PrintStatus = false;
			std::cerr << " Node evaluations: " << numChecks  
				 	  << " Queue size: "       << Frontier.size()
				 	  << " Nodes size: "       << States.hash_table.size()
					  << std::endl;
		}
	}

	//Now, walk backwards via parent_entry, adding the parent_action each time
	std::vector< Action > ret;
	if( Final ) ret.reserve( Final->second.cost_so_far + 1 );
	while( Final )
	{
		if( Final->second.parent_entry )
			ret.push_back( Final->second.parent_action );
		Final = Final->second.parent_entry;
	}
	std::reverse( ret.begin(), ret.end() );

	return ret;
}
};

#endif
