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
		bool Visited = false;

		Action parent_action;
		StateAndMeta * parent_entry = nullptr;
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
			return *hash_table.insert( { state, MetaData{} } ).first;
		}
	};

	struct QueueEntry
	{
		std::reference_wrapper< StateAndMeta > state_and_meta;
		int Priority;

		bool operator<( const QueueEntry & o ) const { return Priority > o.Priority; }//Invert logic, as priority_queue returns max element according to "<"
	};

static std::vector< Action > Solve( 
		const State & initial, 
		std::function< int( const State &) > Evaluator, 
		std::function< bool( const State & ) > GoalTest )
{

	StatesHashTable                         States;
	std::priority_queue< QueueEntry >     Frontier; //States to explore next

	States.hash_table.reserve( /*State::NumStates*/ 1000000 );

	//Add the initial state at 0 cost, returning iterator
	StateAndMeta & initial_state_and_meta = States.get( initial );
	initial_state_and_meta.second.cost_so_far = 0;

	//Create priority queue

	//Add initial state
	Frontier.push( { initial_state_and_meta, 0 } );

	StateAndMeta * Final = &initial_state_and_meta;

	while( !Frontier.empty() )
	{
		QueueEntry top = Frontier.top();
		Frontier.pop();
		StateAndMeta & state_and_meta = top.state_and_meta;

		const State & state = state_and_meta.first;
		MetaData    & meta  = state_and_meta.second;

		if( GoalTest(state) )
		{
			Final = &state_and_meta;
			break;
		}
		if( meta.Visited ) continue;
		meta.Visited = true;

		for( auto action : state.AvailableActions() )
		{
			//See what the new state is after applying the action
			State new_state   = state.Apply( action );

			//inserts if doesn't exist and returns reference
			StateAndMeta & new_state_and_meta = States.get( new_state );

			MetaData    & new_meta  = new_state_and_meta.second;

			if( !new_meta.Visited ) //don't backtrack
			{
				int new_cost = meta.cost_so_far + action.GetCost();
				if( new_cost < new_meta.cost_so_far )
				{
					new_meta.cost_so_far = new_cost;
					new_meta.parent_action = action;
					new_meta.parent_entry  = &state_and_meta;

					int new_priority = new_cost + Evaluator( new_state );
					Frontier.push( { new_state_and_meta, new_priority } );
				}
			}

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
