#include <functional>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits>
#include <algorithm>

template< typename State, typename Action > 
struct AStar
{
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

static std::vector< Action > Solve( 
		const State & initial, 
		std::function< int ( const State & ) > && GoalCostEstimate, 
		std::function< bool( const State & ) > && GoalTest,
		std::function< bool(               ) > && PrintStatus
		)
{

	StatesHashTable                         States;
	std::priority_queue< QueueEntry >     Frontier; //States to explore next


	//Add the initial state at 0 cost, returning iterator
	StateAndMeta & initial_state_and_meta = States.get( initial );
	initial_state_and_meta.second.cost_so_far = 0;

	//Create priority queue

	//Add initial state
	Frontier.push( { initial_state_and_meta, 0 } );

	StateAndMeta * Final = &initial_state_and_meta;

	std::size_t numChecks = 0;

	while( !Frontier.empty() )
	{
		QueueEntry top = Frontier.top();
		Frontier.pop();
		StateAndMeta & state_and_meta = top.state_and_meta;

		const State & state = state_and_meta.first;
		MetaData    & meta  = state_and_meta.second;

		meta.num_references--;

		if( GoalTest(state) )
		{
			Final = &state_and_meta;
			break;
		}

		//if( meta.num_references > 0 ) continue;

		for( auto &paction : state.AvailableActions() )
		{
			if( !paction ) continue;
			auto action = *paction;

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

				int new_priority = new_cost + GoalCostEstimate( new_state );
				Frontier.push( { new_state_and_meta, new_priority } );
			}

		}

		numChecks++;

		if( numChecks % 100 == 0 && PrintStatus() ) 
		{
			std::cout << " Node evaluations: " << numChecks  
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

	/*
	std::cout << " Node evaluations: " << numChecks  
			  << " Queue size: "       << Frontier.size()
			  << " Nodes size: "       << States.hash_table.size()
			  << std::endl;

			  */
	return ret;
}
};
