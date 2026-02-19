#pragma once


namespace state {
	struct layers_stack;
}

namespace widgets {
	namespace modal {
		void pop_query(state::layers_stack& layers);
	}
}