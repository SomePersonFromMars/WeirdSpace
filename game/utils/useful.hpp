#pragma once
#ifndef USEFUL_HPP
#define USEFUL_HPP

// template<class T>
// T clamp(T x, T a, T b) {
// 	return x < a ? a : (x > b ? b : x);
// }

#define AT3(x, y, z) \
	[static_cast<size_t>(x)] \
	[static_cast<size_t>(y)] \
	[static_cast<size_t>(z)]

#endif
