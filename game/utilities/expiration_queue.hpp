#pragma once
#ifndef EXPIRATION_QUEUE_HPP
#define EXPIRATION_QUEUE_HPP

#include <vector>
#include <functional>
#include <utility>
#include <useful.hpp>

// TODO Test this data structure.

class expiration_queue_t {
public:
	expiration_queue_t(
		std::size_t elements_buffer_size,
		std::size_t max_active_elements_cnt);

	inline void push_back_element_to_active_list(std::size_t element_id);
	void push_back_all_active_elements_to_queue();

	inline std::size_t get_all_elements_cnt() const;
	inline std::size_t get_in_active_list_elements_cnt() const;
	inline bool is_active_list_empty() const;
	inline std::size_t get_oldest_queue_element_id() const;

	inline bool is_correct_element_id(std::size_t element_id) const;

	void for_each_active_element_id(
		const std::function<void(std::size_t)> f) const;
	void verify_correctness() const;

private:
	enum list_type_t : std::size_t {
		QUEUE_LIST = 0,
		ACTIVE_LIST = 1,
		LISTS_CNT = 2
	};

	void push_back_element_to_another_list(std::size_t element_id);
	void cut_element_from_list_with_type(
		std::size_t element_id);
	std::pair<std::size_t, std::size_t> cut_element_from_simple_list(
		std::size_t element_id);
	void connect_two_simple_lists(
			std::size_t left_list_last_element_id,
			std::size_t right_list_first_element_id);
	static inline list_type_t get_opposite_list_type(list_type_t list_type);
	static inline bool is_correct_list_type(list_type_t list_type);

	struct list_state_t {
		inline explicit list_state_t(std::size_t max_elements_cnt);
		const std::size_t max_elements_cnt;
		std::size_t elements_cnt = 0;
		std::size_t tail_element_id = INVALID_ID;
		std::size_t head_element_id = INVALID_ID;
	};

	// Double ended linked list element structure
	struct de_linked_list_el_t {
		list_type_t list_type = QUEUE_LIST;
		std::size_t prev_el_id = INVALID_ID;
		std::size_t next_el_id = INVALID_ID;
	};

	std::vector<de_linked_list_el_t> elements_buffer;
	list_state_t lists_states[LISTS_CNT];
	list_state_t &queue_list_state = lists_states[QUEUE_LIST];
	list_state_t &active_list_state = lists_states[ACTIVE_LIST];
};

inline expiration_queue_t::list_state_t::list_state_t(
		std::size_t max_elements_cnt)
	:max_elements_cnt{max_elements_cnt}
{ }

inline void expiration_queue_t::push_back_element_to_active_list(
		std::size_t element_id) {
	assert(is_correct_element_id(element_id));
	assert(elements_buffer[element_id].list_type == QUEUE_LIST);
	push_back_element_to_another_list(element_id);
}

inline std::size_t expiration_queue_t::get_all_elements_cnt() const {
	return elements_buffer.size();
}

inline std::size_t expiration_queue_t::get_in_active_list_elements_cnt() const {
	return lists_states[ACTIVE_LIST].elements_cnt;
}

inline bool expiration_queue_t::is_active_list_empty() const {
	return get_in_active_list_elements_cnt() == 0;
}

inline std::size_t expiration_queue_t::get_oldest_queue_element_id() const {
	return queue_list_state.tail_element_id;
}

inline bool expiration_queue_t::is_correct_element_id(
		std::size_t element_id) const {
	return element_id < get_all_elements_cnt();
}

inline expiration_queue_t::list_type_t
	expiration_queue_t::get_opposite_list_type(
			list_type_t list_type) {
	assert(is_correct_list_type(list_type));
	if (list_type == QUEUE_LIST)
		return ACTIVE_LIST;
	else
		return QUEUE_LIST;
}

inline bool expiration_queue_t::is_correct_list_type(list_type_t list_type) {
	return list_type == QUEUE_LIST or list_type == ACTIVE_LIST;
}

#endif
