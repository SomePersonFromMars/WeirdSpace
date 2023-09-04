#include "expiration_queue.hpp"

expiration_queue_t::expiration_queue_t(
		std::size_t elements_buffer_size,
		std::size_t max_active_elements_cnt)
	:elements_buffer(elements_buffer_size)
	,lists_states {
		list_state_t(elements_buffer_size - max_active_elements_cnt),
		list_state_t(max_active_elements_cnt),
	}
{
	assert(get_all_elements_cnt() >= 2);
	assert(max_active_elements_cnt <= get_all_elements_cnt());

	queue_list_state.tail_element_id = 0;
	queue_list_state.head_element_id = get_all_elements_cnt() - 1;
	elements_buffer.front().next_el_id = 1;
	elements_buffer.back().prev_el_id = get_all_elements_cnt() - 2;
	for (std::size_t i = 1; i < get_all_elements_cnt() - 1; ++i) {
		auto &element = elements_buffer[i];
		element.prev_el_id = i - 1;
		element.next_el_id = i + 1;
	}
}

void expiration_queue_t::cut_element_from_list_with_type(
		std::size_t element_id) {
	assert(is_correct_element_id(element_id));
	const list_type_t list_type = elements_buffer[element_id].list_type;
	assert(is_correct_list_type(list_type));
	list_state_t &list_state = lists_states[list_type];

	const auto [left_list_last_element_id, right_list_first_element_id]
		= cut_element_from_simple_list(element_id);
	if (list_state.tail_element_id == element_id)
		list_state.tail_element_id = right_list_first_element_id;
	if (list_state.head_element_id == element_id)
		list_state.head_element_id = left_list_last_element_id;
	assert(list_state.elements_cnt >= 1);
	--list_state.elements_cnt;
}

std::pair<std::size_t, std::size_t>
		expiration_queue_t::cut_element_from_simple_list(
				std::size_t element_id) {
	assert(is_correct_element_id(element_id));
	const std::size_t left_list_last_element_id
		= elements_buffer[element_id].prev_el_id;
	const std::size_t right_list_first_element_id
		= elements_buffer[element_id].next_el_id;
	connect_two_simple_lists(
		left_list_last_element_id,
		right_list_first_element_id);
	elements_buffer[element_id].prev_el_id = INVALID_ID;
	elements_buffer[element_id].next_el_id = INVALID_ID;
	return { left_list_last_element_id, right_list_first_element_id };
}

void expiration_queue_t::connect_two_simple_lists(
		std::size_t left_list_last_element_id,
		std::size_t right_list_first_element_id) {
	if (left_list_last_element_id != INVALID_ID) {
		assert(is_correct_element_id(left_list_last_element_id));
		elements_buffer[left_list_last_element_id].next_el_id
			= right_list_first_element_id;
	}
	if (right_list_first_element_id != INVALID_ID) {
		assert(is_correct_element_id(right_list_first_element_id));
		elements_buffer[right_list_first_element_id].next_el_id
			= left_list_last_element_id;
	}
}

void expiration_queue_t::push_back_element_to_another_list(
		std::size_t element_id) {
	const list_type_t first_list_type = elements_buffer[element_id].list_type;
	const list_type_t another_list_type
		= get_opposite_list_type(first_list_type);
	list_state_t &another_list_state = lists_states[another_list_type];
	assert(another_list_state.elements_cnt
			< another_list_state.max_elements_cnt);

	cut_element_from_list_with_type(element_id);
	connect_two_simple_lists(another_list_state.head_element_id, element_id);
	another_list_state.head_element_id = element_id;
	++another_list_state.elements_cnt;
	elements_buffer[element_id].list_type = another_list_type;
}

void expiration_queue_t::push_back_all_active_elements_to_queue() {
	while (not is_active_list_empty()) {
		assert(elements_buffer[active_list_state.head_element_id].list_type
				== ACTIVE_LIST);
		push_back_element_to_another_list(active_list_state.head_element_id);
		assert(elements_buffer[active_list_state.head_element_id].list_type
				== QUEUE_LIST);
	}
}

void expiration_queue_t::for_each_active_element_id(
		const std::function<void(std::size_t)> f) const {
	std::size_t i = active_list_state.tail_element_id;
	while (i != INVALID_ID) {
		assert(is_correct_element_id(i));
		assert(elements_buffer[i].list_type == ACTIVE_LIST);
		f(i);
		i = elements_buffer[i].next_el_id;
	}
}

void expiration_queue_t::verify_correctness() const {
	std::size_t in_list_cnt[LISTS_CNT] {  };
	for (std::size_t i = 0; i < get_all_elements_cnt(); ++i) {
		const list_type_t list_type = elements_buffer[i].list_type;
		const list_state_t list_state = lists_states[list_type];
		assert(is_correct_list_type(list_type));
		++in_list_cnt[list_type];
		const std::size_t prev_el_id = elements_buffer[i].prev_el_id;
		const std::size_t next_el_id = elements_buffer[i].next_el_id;
		if (prev_el_id != INVALID_ID) {
			assert(is_correct_element_id(prev_el_id));
			assert(elements_buffer[prev_el_id].next_el_id == i);
		} else
			assert(list_state.tail_element_id == i);
		if (next_el_id != INVALID_ID) {
			assert(is_correct_element_id(next_el_id));
			assert(elements_buffer[next_el_id].prev_el_id == i);
		} else
			assert(list_state.head_element_id == i);
	}
	assert(in_list_cnt[QUEUE_LIST]  == queue_list_state.elements_cnt);
	assert(in_list_cnt[ACTIVE_LIST] == active_list_state.elements_cnt);
	assert(queue_list_state.elements_cnt
		<= queue_list_state.max_elements_cnt);
	assert(active_list_state.elements_cnt
		<= active_list_state.max_elements_cnt);
}
