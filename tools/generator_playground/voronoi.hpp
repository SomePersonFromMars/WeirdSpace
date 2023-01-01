#pragma once
#ifndef VORONOI_HPP
#define VORONOI_HPP

#include <vector>
#include <glm/glm.hpp>

struct voronoi_diagram_t;
struct voronoi_t {
	glm::dvec2 center;
	std::vector<glm::dvec2> points;

	struct edge_t {
		std::size_t neighbor_id;
		// Variable useful for drawing voronoi diagram
		// without drawing edges twice
		std::size_t smaller_half_edge_id;
		bool visible;
		enum type_t {
			NONE = 0,
			USUAL,
			TO_LEFT,
			TO_RIGHT
		} type = NONE;

		// Beg and end are the edge's ends and
		// a quadrilateral's side points.
		// The quadrilateral is useful for
		// making the edge noisy while drawing.
		glm::dvec2 quad_top;
		glm::dvec2 quad_bottom;
		glm::dvec2 beg;
		glm::dvec2 end;

		void correct_quad();
	};
	std::vector<edge_t> al;
private:
	// Polygon fully calculated
	bool completed = false;
	bool clipped = false;

	friend voronoi_diagram_t;
};
struct voro_id_t {
	std::size_t id;
	enum type_t : uint8_t {
		BASE = 0,
		LEFT,
		RIGHT
	} type = BASE;
};

struct voronoi_diagram_t {
	glm::dvec2 space_max;
	double space_max_x_duplicate_off;
	glm::dvec2 duplicate_off_vec;
	std::vector<voronoi_t> voronois;

	// Array useful for drawing voronoi diagram
	// without drawing edges twice
	std::vector<bool> half_edge_drawn;

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

	glm::dvec2 triangle_circumcenter(
			glm::dvec2 A, glm::dvec2 B, glm::dvec2 C) const;

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
