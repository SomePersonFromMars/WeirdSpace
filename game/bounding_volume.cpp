#include "bounding_volume.hpp"

bool AABB_t::is_on_or_forward_plane(const plane_t& plane) const {
	// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
	const float r
		= extents.x * std::abs(plane.normal.x)
		+ extents.y * std::abs(plane.normal.y)
		+ extents.z * std::abs(plane.normal.z);

	return -r <= plane.get_signed_distance_to_plane(center);
}
