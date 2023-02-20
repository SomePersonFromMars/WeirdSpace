#pragma once
#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <glm/glm.hpp>

// `a`, `b` - vectors
// `det[a, b] > 0` => directed angle between `a` and `b` is ccw
template<class T>
inline T determinant(
		const glm::tvec2<T, glm::highp> &a,
		const glm::tvec2<T, glm::highp> &b) {
	return a.x*b.y - a.y*b.x;
}

template<class T>
inline T dot_product(
		const glm::tvec2<T, glm::highp> &a,
		const glm::tvec2<T, glm::highp> &b
		) {
	return a.x*b.x + a.y*b.y;
}

std::pair<glm::dvec2, bool> intersect_lines(
		glm::dvec2 P1, glm::dvec2 P2, glm::dvec2 Q1, glm::dvec2 Q2
		);

// This function is buggy and unreliable
bool intersect_full_box_full_polygon(
		glm::dvec2 box_lower_corner, double box_dim,
		std::vector<glm::dvec2> points
		);

#endif
