#pragma once
#ifndef VORONOI_HPP
#define VORONOI_HPP

#include <vector>
#include <glm/glm.hpp>

struct voronoi_diagram_t;
struct voronoi_t {
	glm::dvec2 center;
	std::vector<glm::dvec2> points;
	std::vector<std::size_t> al;
private:
	// Polygon fully calculated
	bool complete = false;
	bool clipped = false;
	friend voronoi_diagram_t;
};

struct voronoi_diagram_t {
	glm::dvec2 space_max;
	std::vector<voronoi_t> voronois;

	inline std::size_t voronois_cnt();
	void generate_relaxed(std::size_t iterations_cnt);

private:
	std::vector<double> centers;
	void generate();
	// Applies Lloyd's relaxation algorithm
	// Calculations are based on the `voronois`
	// array. After calculating new voronoi centers
	// it restores default voronoi polygons state
	// without clearing `voronois` array.
	void voronoi_iteration();

	enum class inters_t : uint8_t {
		// Clockwise sides order
		LEFT = 0,
		TOP,
		RIGHT,
		BOTTOM,
		SIDES_CNT,
		INSIDE,
		OUTSIDE,
	};
	struct reduced_edge_t {
		glm::dvec2 beg, end;
		inters_t beg_inters = inters_t::OUTSIDE;
		inters_t end_inters = inters_t::OUTSIDE;
	};

	glm::vec2 triangle_circumcenter(
			glm::vec2 A, glm::vec2 B, glm::vec2 C) const;

	// Find intersection of a line with a horizontal segment
	// line: parallel to vector v and passing through P
	// horizontal segment: (0, S.y), (S.x, S.y)
	static std::pair<glm::dvec2, bool> intersect_line_h_segment(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);
	// Find intersection of a line with a vertical segment
	// line: parallel to vector v and passing through P
	// vertical segment: (S.x, 0), (S.x, S.y)
	static std::pair<glm::dvec2, bool> intersect_line_v_segment(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);

	static std::pair<glm::dvec2, inters_t>
		find_closest_box_intersection_directed_edge(
			glm::dvec2 P, glm::dvec2 v, glm::dvec2 S);

	static std::pair<reduced_edge_t, bool> trim_edge(
			glm::dvec2 beg, glm::dvec2 end, glm::dvec2 S
			);

	static std::pair<reduced_edge_t, bool> trim_inf_edge(
			const glm::dvec2 beg,
			const glm::dvec2 direction_vec,
			const glm::dvec2 S
			);
};

inline std::size_t voronoi_diagram_t::voronois_cnt() {
	return voronois.size();
}

#endif
