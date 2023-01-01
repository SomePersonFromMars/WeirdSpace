#pragma once
#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <glm/glm.hpp>

std::pair<glm::dvec2, bool> intersect_lines(
		glm::dvec2 P1, glm::dvec2 P2, glm::dvec2 Q1, glm::dvec2 Q2
		);

// This function is buggy and unreliable
bool intersect_full_box_full_polygon(
		glm::dvec2 box_lower_corner, double box_dim,
		std::vector<glm::dvec2> points
		);

#endif
