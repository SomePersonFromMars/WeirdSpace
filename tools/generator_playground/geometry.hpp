#pragma once
#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <vector>
#include <glm/glm.hpp>

bool intersect_full_box_full_polygon(
		glm::dvec2 box_lower_corner, double box_dim,
		std::vector<glm::dvec2> points
		);

#endif
