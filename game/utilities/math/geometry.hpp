// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

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

struct plane_t {
	// Unit vector
	glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
	// Distance from the world origin to the nearest point in the plane
	float distance = 0.f;        // Distance with origin

	plane_t() = default;
	inline plane_t(const glm::vec3& p1, const glm::vec3& norm)
		:normal(glm::normalize(norm))
		,distance(glm::dot(normal, p1))
	{  }

	inline float get_signed_distance_to_plane(const glm::vec3& point) const {
		return glm::dot(normal, point) - distance;
	}
};

struct frustum_t {
	plane_t top_face;
	plane_t bottom_face;

	plane_t right_face;
	plane_t left_face;

	plane_t far_face;
	plane_t near_face;
};

#endif
