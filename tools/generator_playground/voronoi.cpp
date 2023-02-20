#include "voronoi.hpp"
#include "useful.hpp"
#include "geometry.hpp"
#include <delaunator.hpp>

using namespace glm;

glm::dvec2 voronoi_diagram_t::triangle_circumcenter(
		glm::dvec2 A, glm::dvec2 B, glm::dvec2 C) const {

	const double A_len_sq = len_sq(A);
	const double B_len_sq = len_sq(B);
	const double C_len_sq = len_sq(C);

	const double denominator
		= A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y);

	dvec2 O(0, 0);
	O.x += A_len_sq * (B.y - C.y);
	O.x += B_len_sq * (C.y - A.y);
	O.x += C_len_sq * (A.y - B.y);
	O.x /= 2.0;
	O.x /= denominator;

	O.y += A_len_sq * (B.x - C.x);
	O.y += B_len_sq * (C.x - A.x);
	O.y += C_len_sq * (A.x - B.x);
	O.y /= -2.0;
	O.y /= denominator;

	return O;
}

std::pair<dvec2, bool> voronoi_diagram_t::intersect_line_h_segment(
		dvec2 P, dvec2 v,
		dvec2 S) {
	const double &A = v.x;
	const double &B = v.y;
	dvec2 X;
	X.y = S.y;
	X.x = P.x + A * (S.y - P.y) / B;
	const bool on_line = in_between_inclusive(0.0, S.x, X.x);
	return {X, on_line};
}

std::pair<dvec2, bool> voronoi_diagram_t::intersect_line_v_segment(
		dvec2 P, dvec2 v,
		dvec2 S) {
	const double &A = v.x;
	const double &B = v.y;
	dvec2 X;
	X.x = S.x;
	X.y = P.y + B * (S.x - P.x) / A;
	const bool on_line = in_between_inclusive(0.0, S.y, X.y);
	return {X, on_line};
}

std::pair<dvec2, voronoi_diagram_t::inters_t>
	voronoi_diagram_t::find_closest_box_intersection_directed_edge(dvec2 P,
		dvec2 v, dvec2 S) {

	const double v_len_sq = len_sq(v);

	std::pair<dvec2, bool> r;
	inters_t inters = inters_t::OUTSIDE;

	double closest_X_dist_sq = std::numeric_limits<double>::max();
	dvec2 closest_X;

	double dist_sq;
	dvec2 w;

	// Find intersection starting from P
	// Left box edge
	r = intersect_line_v_segment(P, v, {0, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::LEFT;
	}

	// Right box edge
	r = intersect_line_v_segment(P, v, {S.x, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::RIGHT;
	}

	// Top box edge
	r = intersect_line_h_segment(P, v, {S.x, 0});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::TOP;
	}

	// Bottom box edge
	r = intersect_line_h_segment(P, v, {S.x, S.y});
	w = r.first - P;
	dist_sq = len_sq(w);
	if (
			r.second &&
			dist_sq < closest_X_dist_sq &&
			dist_sq <= v_len_sq &&
			same_direction_knowing_same_line(v, w)
			) {
		closest_X_dist_sq = dist_sq;
		closest_X = r.first;
		inters = inters_t::BOTTOM;
	}

	if (inters == inters_t::OUTSIDE) {
		if (in_between_inclusive(0.0, S.x, P.x)
			&& in_between_inclusive(0.0, S.y, P.y))
			inters = inters_t::INSIDE;
	}

	return {closest_X, inters};
}

std::pair<voronoi_diagram_t::reduced_edge_t, bool>
	voronoi_diagram_t::trim_edge(glm::dvec2 beg, glm::dvec2 end, glm::dvec2 S)
{
	// Return value
	reduced_edge_t red_edge;
	red_edge.beg = beg;
	red_edge.end = end;
	red_edge.beg_inters = inters_t::INSIDE;
	red_edge.end_inters = inters_t::INSIDE;

	std::pair<dvec2, inters_t> r;
	// Find intersection starting from beg
	r = find_closest_box_intersection_directed_edge(
			beg, end-beg, S);
	if (r.second == inters_t::OUTSIDE) {
		return {reduced_edge_t(), false};
	}
	if (
			r.second != inters_t::INSIDE &&
			(
				!in_between_inclusive(0.0, S.x, beg.x) ||
				!in_between_inclusive(0.0, S.y, beg.y)
			)
		) {
		red_edge.beg = r.first;
		red_edge.beg_inters = r.second;
	}

	// Find intersection starting from end
	r = find_closest_box_intersection_directed_edge(
			end, -(end-beg), S);
	if (
			r.second != inters_t::INSIDE &&
			(
				!in_between_inclusive(0.0, S.x, end.x) ||
				!in_between_inclusive(0.0, S.y, end.y)
			)
		) {
		red_edge.end = r.first;
		red_edge.end_inters = r.second;
	}

	return {red_edge, true};
}

std::pair<voronoi_diagram_t::reduced_edge_t, bool>
	voronoi_diagram_t::trim_inf_edge(
		const glm::dvec2 beg,
		const glm::dvec2 direction_vec,
		const glm::dvec2 S
		)
{

	dvec2 closest_intersections[2];
	inters_t closest_intersections_type[2] {
		inters_t::OUTSIDE,
		inters_t::OUTSIDE
	};
	double closest_intersections_dist_sq[2] {
		std::numeric_limits<double>::infinity(),
		std::numeric_limits<double>::infinity()
	};

	std::pair<dvec2, bool> r;
	dvec2 w;
	double w_len_sq;

	// Find intersections starting from beg
	// Left box edge
	r = intersect_line_v_segment(beg, direction_vec, {0, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::LEFT;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::LEFT;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Right box edge
	r = intersect_line_v_segment(beg, direction_vec, {S.x, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::RIGHT;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::RIGHT;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Top box edge
	r = intersect_line_h_segment(beg, direction_vec, {S.x, 0});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::TOP;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::TOP;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	// Bottom box edge
	r = intersect_line_h_segment(beg, direction_vec, {S.x, S.y});
	w = r.first - beg;
	w_len_sq = len_sq(w);
	if (
			r.second &&
			w_len_sq < closest_intersections_dist_sq[1] &&
			same_direction_knowing_same_line(direction_vec, w)
			) {
		if (w_len_sq < closest_intersections_dist_sq[0]) {
			closest_intersections[1]
				= closest_intersections[0];
			closest_intersections_type[1]
				= closest_intersections_type[0];
			closest_intersections_dist_sq[1]
				= closest_intersections_dist_sq[0];

			closest_intersections[0] = r.first;
			closest_intersections_type[0] = inters_t::BOTTOM;
			closest_intersections_dist_sq[0] = w_len_sq;
		} else {
			closest_intersections[1] = r.first;
			closest_intersections_type[1] = inters_t::BOTTOM;
			closest_intersections_dist_sq[1] = w_len_sq;
		}
	}

	if (closest_intersections_dist_sq[0]
			== std::numeric_limits<double>::infinity()) {
		return {reduced_edge_t(), false};
	} else if (closest_intersections_dist_sq[1]
			== std::numeric_limits<double>::infinity()) {
		reduced_edge_t red_edge;
		red_edge.beg = beg;
		red_edge.beg_inters = inters_t::INSIDE;
		red_edge.end = closest_intersections[0];
		red_edge.end_inters = closest_intersections_type[0];
		return {red_edge, true};
	} else {
		reduced_edge_t red_edge;
		red_edge.beg = closest_intersections[0];
		red_edge.beg_inters = closest_intersections_type[0];
		red_edge.end = closest_intersections[1];
		red_edge.end_inters = closest_intersections_type[1];
		return {red_edge, true};
	}
}

void voronoi_t::edge_t::correct_quad() {
	auto correct_side = [](
			const dvec2 &A, const dvec2 &B, dvec2 &X,
			bool rotate_right_cw) -> void
	{
		const dvec2 BA = A - B;
		double det;
		if (rotate_right_cw)
			det = determinant(dvec2(-BA.y, BA.x), X - B);
		else
			det = determinant(dvec2(BA.y, -BA.x), X - B);
		if (det >= 0)
			return;

		const dvec2 AX = X - A;
		const dvec2 AB = B - A;

		const double s = AX.x;
		const double t = AX.y;
		const double u = AB.x;
		const double w = AB.y;

		const double a = A.x;
		const double b = A.y;
		const double c = B.x;
		const double d = B.y;

		if (s*u + t*w == 0)
			return;

		X.x = a*t*w - b*s*w + c*s*u + d*s*w;
		X.x /= s*u + t*w;
		X.y = -a*t*u + b*s*u + c*t*u + d*t*w;
		X.y /= s*u + t*w;
	};

	// Not sure why there is `false` everywhere
	correct_side(end, beg, quad_top, false);
	correct_side(beg, end, quad_top, false);
	correct_side(end, beg, quad_bottom, false);
	correct_side(beg, end, quad_bottom, false);
}

void voronoi_diagram_t::generate() {
	const delaunator::Delaunator d(centers);
	half_edge_drawn.resize(d.halfedges.size());
	voronois.assign(voronois_cnt(), voronoi_t());
	std::vector<dvec2> tri_circumcenter(d.triangles.size() / 3);
	for (std::size_t i = 0; i < d.triangles.size() / 3; ++i) {
		dvec2 A, B, C;
		A.x = d.coords[2 * d.triangles[3*i]];
		A.y = d.coords[2 * d.triangles[3*i] + 1];
		B.x = d.coords[2 * d.triangles[3*i + 1]];
		B.y = d.coords[2 * d.triangles[3*i + 1] + 1];
		C.x = d.coords[2 * d.triangles[3*i + 2]];
		C.y = d.coords[2 * d.triangles[3*i + 2] + 1];

		tri_circumcenter[i] = triangle_circumcenter(A, B, C);
	}

	// Iterate over all half edges
	for (std::size_t i = 0; i < d.triangles.size(); ++i) {
		const std::size_t center_id = d.triangles[i];
		const bool voronoi_is_duplicate = center_id >= voronois_cnt();
		if (voronoi_is_duplicate)
			continue;
		// const std::size_t voronoi_id
		// 	= center_id < voronois_cnt()
		// 	? center_id : center_id - voronois_cnt();
		const std::size_t voronoi_id = center_id;
		voronoi_t &voronoi = voronois[voronoi_id];

		// There are many half edges starting in a voronoi
		// center, but this loop finds only one of them
		// and then iterates over the others.
		if (voronoi.completed)
			continue;
		voronoi.completed = true;

		// Half edges' indices from Delaunay triangulation
		// equivalent to the voronoi's edges
		std::vector<std::size_t> tri_half_edges;

		// Finding the triangle (if such exists)
		// that is adjacent to void that is also
		// the most counterclockwise rotated about
		// the voronoi center.
		const std::size_t start_half_edge = i;
		std::size_t beg_half_edge = start_half_edge;
		do {
			const std::size_t next_half_edge_in_triangle
				= beg_half_edge - beg_half_edge%3 + (beg_half_edge+2)%3;
			const std::size_t prev_voronoi_half_edge
				= d.halfedges[next_half_edge_in_triangle];
			if (prev_voronoi_half_edge == delaunator::INVALID_INDEX) {
				voronoi.clipped = true;
				break;
			}

			beg_half_edge = prev_voronoi_half_edge;
		} while (beg_half_edge != start_half_edge);

		// Finding the triangle (if such exists)
		// that is adjacent to void that is also
		// the most clockwise rotated about
		// the voronoi center.
		// Also add all visited triangles' half edges
		// such that they have their beg (or end) in the voronoi center
		// as the current voronoi's half edges.
		if (voronoi.clipped) {
			// This is the only case that a half edge has its end
			// in the voronoi center.
			tri_half_edges.push_back(
				beg_half_edge - beg_half_edge%3 + (beg_half_edge+2)%3
				);
		}
		std::size_t cur_half_edge = beg_half_edge;
		do {
			tri_half_edges.push_back(cur_half_edge);

			const std::size_t twin_half_edge
				= d.halfedges[cur_half_edge];
			if (twin_half_edge == delaunator::INVALID_INDEX)
				break;
			cur_half_edge
				= twin_half_edge - twin_half_edge%3 + (twin_half_edge+1)%3;
		} while (cur_half_edge != beg_half_edge);

		// Find neighbors
		voronoi.al.resize(tri_half_edges.size());
		for (std::size_t j = 0; j < tri_half_edges.size(); ++j) {
			const std::size_t half_edge = tri_half_edges[j];
			voronoi_t::edge_t &edge = voronoi.al[j];
			edge.smaller_half_edge_id = half_edge;
			if (voronoi.clipped && j == 0) {
				edge.neighbor_id = d.triangles[half_edge];
			} else if (d.halfedges[half_edge] == delaunator::INVALID_INDEX) {
				edge.neighbor_id = d.triangles[
					half_edge-half_edge%3+(half_edge+1)%3 ];
			} else {
				edge.neighbor_id = d.triangles[d.halfedges[half_edge]];
				min_replace(edge.smaller_half_edge_id, d.halfedges[half_edge]);
			}

			edge.quad_top.x = d.coords[2*center_id+0];
			edge.quad_top.y = d.coords[2*center_id+1];
			edge.quad_bottom.x = d.coords[2*edge.neighbor_id+0];
			edge.quad_bottom.y = d.coords[2*edge.neighbor_id+1];

			if (edge.neighbor_id >= 2*voronois_cnt()) {
				edge.type = voronoi_t::edge_t::TO_RIGHT;
				edge.neighbor_id -= 2*voronois_cnt();
			} else if (edge.neighbor_id >= 1*voronois_cnt()) {
				edge.type = voronoi_t::edge_t::TO_LEFT;
				edge.neighbor_id -= 1*voronois_cnt();
			} else {
				edge.type = voronoi_t::edge_t::USUAL;
			}
		}

		// Converting Delaunay triangulation half edges
		// to voronoi diagram edges, clipping them
		// and ignoring them if they don't exist
		// on the visible plane.
		std::vector<reduced_edge_t> red_edges;
		std::size_t last_incoming_red_edge_id = INVALID_ID;
		for (std::size_t j = 0; j < tri_half_edges.size(); ++j) {
			const std::size_t h = tri_half_edges[j];
			bool exists;
			reduced_edge_t red_edge;
			if (d.halfedges[h] == delaunator::INVALID_INDEX) {
				const dvec2 v1(
						centers[2 * d.triangles[h]],
						centers[2 * d.triangles[h] + 1]);
				const dvec2 v2(
						centers[2 * d.triangles[h-h%3+(h+1)%3]],
						centers[2 * d.triangles[h-h%3+(h+1)%3] + 1]);
				const dvec2 &beg = tri_circumcenter[h/3];
				const dvec2 v = v2 - v1;
				// Rotating the new edge 90 degrees clockwise
				// relatively to the current tri_half_edge.
				const dvec2 direction_vec(-v.y, v.x);
				const dvec2 S = space_max;

				const std::pair<reduced_edge_t, bool> r
					= trim_inf_edge(beg, direction_vec, S);
				red_edge = r.first;
				exists = r.second;
				if (r.second && red_edges.empty()) {
					// This is the only case that a half edge has its end
					// in the voronoi center, so it is necessary to swap
					// beg with end in the new edge.
					std::swap(red_edge.beg, red_edge.end);
					std::swap(red_edge.beg_inters, red_edge.end_inters);
				}
			} else {
				const dvec2 beg = tri_circumcenter[h/3];
				const dvec2 end = tri_circumcenter[d.halfedges[h]/3];
				const dvec2 S = space_max;

				const std::pair<reduced_edge_t, bool> r
					= trim_edge(beg, end, S);
				red_edge = r.first;
				exists = r.second;
			}

			voronoi.al[j].visible = exists;
			if (exists) {
				if (red_edge.end_inters != inters_t::INSIDE) {
					last_incoming_red_edge_id
						= red_edges.size();
					voronoi.clipped = true;
				}
				red_edges.push_back(red_edge);

				voronoi_t::edge_t &edge = voronoi.al[j];
				edge.beg = red_edge.beg;
				edge.end = red_edge.end;
				edge.correct_quad();
				edge.tri_edge_len
					= std::sqrt(len_sq(edge.quad_bottom - edge.quad_top));
				edge.voro_edge_len
					= std::sqrt(len_sq(edge.end - edge.beg));
				edge.to_mid = (edge.beg + edge.end) / 2.0 - edge.quad_top;
				edge.to_mid_len = std::sqrt(len_sq(edge.to_mid));
			}
		}

		// Converting clipped edges to points and if
		// a voronoi polygon is clipped adding points
		// such that whole visible plane is occupied
		// with voronoi polygons.
		if (voronoi.clipped) {
			std::size_t j = last_incoming_red_edge_id;
			assert(j != INVALID_ID);

			do {
				const reduced_edge_t &e1 = red_edges[j];
				voronoi.points.push_back(e1.beg);
				if (!(e1.end_inters != inters_t::INSIDE)) {
					continue;
				}
				voronoi.points.push_back(e1.end);
				const reduced_edge_t &e2
					= red_edges[(j+1) % red_edges.size()];
				assert(e2.beg_inters != inters_t::INSIDE);

				const inters_t beg_edge_inters = e1.end_inters;
				const inters_t end_edge_inters = e2.beg_inters;
				const uint8_t beg_side_id
					= static_cast<uint8_t>(beg_edge_inters);
				const uint8_t end_side_id
					= static_cast<uint8_t>(end_edge_inters);
				constexpr uint8_t SIDES_CNT
					= static_cast<uint8_t>(inters_t::SIDES_CNT);
				assert(beg_side_id < SIDES_CNT);
				assert(end_side_id < SIDES_CNT);

				if (beg_side_id != end_side_id)
					for (
							uint8_t k = (beg_side_id+0)%SIDES_CNT;
							k != end_side_id;
							k = (k+1)%SIDES_CNT) {
						switch (static_cast<inters_t>(k)) {
							case inters_t::LEFT:
								voronoi.points.push_back({0, 0});
								break;
							case inters_t::TOP:
								voronoi.points.push_back({space_max.x, 0});
								break;
							case inters_t::RIGHT:
								voronoi.points.push_back(
										{space_max.x, space_max.y});
								break;
							case inters_t::BOTTOM:
								voronoi.points.push_back({0, space_max.y});
								break;
							default:
								break;
						}
					}

			} while (
					j = (j+1) % red_edges.size(),
					j != last_incoming_red_edge_id
					);
		} else {
			for (const reduced_edge_t &e : red_edges)
				voronoi.points.push_back(e.beg);
		}

		// Add a dummy edge, if voronoi is clipped
		if (voronoi.clipped) {
			voronoi_t::edge_t &edge = voronoi.dummy_edge;
			edge.neighbor_id = voronoi_id; // Edge to itself
			edge.smaller_half_edge_id = INVALID_ID;
			edge.visible = false;
			edge.type = voronoi_t::edge_t::NONE;
			edge.quad_top.x = NAN;
			edge.quad_top.y = NAN;
			edge.quad_bottom.x = NAN;
			edge.quad_bottom.y = NAN;
			edge.beg = voronoi.points.back();
			edge.end = voronoi.points.front();
			edge.tri_edge_len = NAN;
			edge.voro_edge_len = std::sqrt(len_sq(edge.end - edge.beg));
			edge.to_mid = (edge.beg + edge.end) / 2.0 - voronoi.center;
			edge.to_mid_len = std::sqrt(len_sq(edge.to_mid));
		}
	}
}

void voronoi_diagram_t::voronoi_iteration() {
	const std::size_t cnt = voronois_cnt();
	for (std::size_t i = 0; i < voronois_cnt(); ++i) {
		dvec2 new_center(0);
		for (const dvec2 &p : voronois[i].points)
			new_center += p;
		new_center /= static_cast<double>(voronois[i].points.size());
		centers[2*i+0] = new_center.x;
		centers[2*i+1] = new_center.y;
		centers[2*(cnt*1+i) + 0]
			= new_center.x - space_max_x_duplicate_off;
		centers[2*(cnt*1+i) + 1] = new_center.y;
		centers[2*(cnt*2+i) + 0]
			= new_center.x + space_max_x_duplicate_off;
		centers[2*(cnt*2+i) + 1] = new_center.y;

		// Previous calculations are not valid anymore
		voronois[i] = voronoi_t();
	}
	half_edge_drawn.clear();
}

void voronoi_diagram_t::generate_relaxed(std::size_t iterations_cnt) {
	const std::size_t cnt = voronois_cnt();
	centers.resize(cnt*2 *3);
	for (std::size_t i = 0; i < cnt; ++i) {
		centers[2*i+0] = voronois[i].center.x;
		centers[2*i+1] = voronois[i].center.y;
		centers[2*(cnt*1+i) + 0]
			= voronois[i].center.x - space_max_x_duplicate_off;
		centers[2*(cnt*1+i) + 1] = voronois[i].center.y;
		centers[2*(cnt*2+i) + 0]
			= voronois[i].center.x + space_max_x_duplicate_off;
		centers[2*(cnt*2+i) + 1] = voronois[i].center.y;
	}

	generate();
	for (std::size_t iteration = 0; iteration < iterations_cnt; ++iteration) {
		voronoi_iteration();
		generate();
	}

	for (std::size_t i = 0; i < cnt; ++i) {
		voronois[i].center = dvec2(
				centers[2*i+0],
				centers[2*i+1]
				);
	}
}
