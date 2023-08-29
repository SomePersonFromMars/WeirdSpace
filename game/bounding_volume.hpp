#pragma once
#ifndef BOUNDING_VOLUME_HPP
#define BOUNDING_VOLUME_HPP

#include <glm/glm.hpp>
#include <geometry.hpp>

struct bounding_volume_t {
	virtual bool is_on_or_forward_plane(const plane_t& plane) const = 0;
	inline bool is_on_frustum(const frustum_t& cam_frustum) const;
};

inline bool bounding_volume_t::is_on_frustum(
		const frustum_t& cam_frustum) const {
	return (is_on_or_forward_plane(cam_frustum.left_face) &&
		is_on_or_forward_plane(cam_frustum.right_face) &&
		is_on_or_forward_plane(cam_frustum.top_face) &&
		is_on_or_forward_plane(cam_frustum.bottom_face) &&
		is_on_or_forward_plane(cam_frustum.near_face) &&
		is_on_or_forward_plane(cam_frustum.far_face));
};

struct AABB_t : public bounding_volume_t {
	glm::vec3 center { 0.f, 0.f, 0.f };
	glm::vec3 extents { 0.f, 0.f, 0.f };

	inline AABB_t(const glm::vec3& min, const glm::vec3& max);
	// See https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
	bool is_on_or_forward_plane(const plane_t& plane) const final;
};

inline AABB_t::AABB_t(const glm::vec3& min, const glm::vec3& max)
	:bounding_volume_t{}
	,center{ (max + min) * 0.5f }
	,extents{ max.x - center.x, max.y - center.y, max.z - center.z }
{}

#endif
