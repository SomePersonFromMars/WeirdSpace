#include "geometry.hpp"
#include "useful.hpp"

using namespace glm;

// P, V
// Q, W
// V.y*(x - P.x) - V.x*(y - P.y) = 0
// W.y*(x - Q.x) - W.x*(y - Q.y) = 0
// V.y*(x - P.x) - V.x*(y - P.y) = 0
// Vy*x - Vy*Px - Vx*y + Vx*Py = 0
// Vy*x - Vx*y + (-Vy*Px + Vx*Py) = 0
// a = Vy
// b = -Vx
// c = -Vy*Px + Vx*Py
std::pair<dvec2, bool> intersect_segments(
		dvec2 P1, dvec2 P2, dvec2 Q1, dvec2 Q2
		) {
	const dvec2 P = P1;
	const dvec2 Q = Q1;
	const dvec2 V = P2 - P1;
	const dvec2 W = Q2 - Q1;
	const double a1 = V.y;
	const double b1 = -V.x;
	const double c1 = -V.y*P.x + V.x*P.y;
	const double a2 = W.y;
	const double b2 = -W.x;
	const double c2 = -W.y*Q.x + W.x*Q.y;

	dvec2 intersection(
			(b1*c2 - b2*c1)/(a1*b2 - a2*b1),
			(c1*a2 - c2*a1)/(a1*b2 - a2*b1));

	return {
		intersection,
		in_between_inclusive(P1.x, P2.x, intersection.x) &&
		in_between_inclusive(P1.y, P2.y, intersection.y) &&
		in_between_inclusive(Q1.x, Q2.x, intersection.x) &&
		in_between_inclusive(Q1.y, Q2.y, intersection.y)
	};
}

bool intersect_full_box_segment(
		dvec2 box_lower_corner, double box_dim, dvec2 P, dvec2 Q) {
	const bool P_inside =
		in_between_inclusive(box_lower_corner.x, box_lower_corner.x+box_dim,
				P.x) &&
		in_between_inclusive(box_lower_corner.y, box_lower_corner.y+box_dim,
				P.y);
	const bool Q_inside =
		in_between_inclusive(box_lower_corner.x, box_lower_corner.x+box_dim,
				Q.x) &&
		in_between_inclusive(box_lower_corner.y, box_lower_corner.y+box_dim,
				Q.y);
	if (P_inside || Q_inside)
		return true;

	const dvec2 p0 = box_lower_corner;
	const dvec2 p1 = box_lower_corner + dvec2(box_dim, 0);
	const dvec2 p2 = box_lower_corner + dvec2(0, box_dim);
	const dvec2 p3 = box_lower_corner + dvec2(box_dim, box_dim);

	// bool b1 = intersect_segments(
	// 		P, Q, p0, p1
	// 		).second;
	// bool b2 = intersect_segments(
	// 		P, Q, p0, p2
	// 		).second;
	// bool b3 = intersect_segments(
	// 		P, Q, p1, p3
	// 		).second;
	// bool b4 = intersect_segments(
	// 		P, Q, p2, p3
	// 		).second;
	// return b1 || b2 || b3 || b4;

	return
		intersect_segments(
			P, Q, p0, p1
			).second ||
		intersect_segments(
			P, Q, p0, p2
			).second ||
		intersect_segments(
			P, Q, p1, p3
			).second ||
		intersect_segments(
			P, Q, p2, p3
			).second;
}

bool intersect_full_box_full_polygon(
		dvec2 box_lower_corner, double box_dim, std::vector<dvec2> points
		) {

	bool intersects = false;
	for (std::size_t i = 0; i < points.size(); ++i) {
		const std::size_t j = i == points.size()-1 ? 0 : i+1;
		if (intersect_full_box_segment(
					box_lower_corner, box_dim,
					points[i], points[j])) {
			intersects = true;
			break;
		}
	}

	return intersects;
}
