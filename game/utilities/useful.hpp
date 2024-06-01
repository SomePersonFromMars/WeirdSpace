#pragma once
#ifndef USEFUL_HPP
#define USEFUL_HPP

#include <cstdio>
#include <glm/glm.hpp>

#ifdef DEBUG
    #define DEBUGONLY(x) x
	#define PRINT_NL printf("\n")
	#define PRINT_D(var) printf(#var " = %d\n", var)
	#define PRINT_LD(var) printf(#var " = %ld\n", var)
	#define PRINT_U(var) printf(#var " = %u\n", var)
	#define PRINT_LU(var) printf(#var " = %lu\n", var)
	#define PRINT_ZU(var) printf(#var " = %zu\n", var)
	#define PRINT_F(var) printf(#var " = %f\n", var)
	#define PRINT_VEC2(vec) \
		printf(#vec " = (%f, %f)\n", vec.x, vec.y)
	#define PRINT_VEC3(vec) \
		printf(#vec " = (%f, %f, %f)\n", vec.x, vec.y, vec.z)
	#define PRINT_VEC4(vec) \
		printf(#vec " = (%f, %f, %f, %f)\n", vec.x, vec.y, vec.z, vec.w)
	#define WHERE printf("%s:%d\n", __FILE__, __LINE__)
	#define GL_GET_ERROR WHERE; PRINT_U(glGetError())
	#include <csignal>
	#define BREAKPOINT (enable_breakpoints) ? std::raise(SIGINT) : 0
	#define BREAKPOINT_IF(expr) (expr) ? (BREAKPOINT) : 0
#else
    #define DEBUGONLY(x)
	#define PRINT_NL
	#define PRINT_D(var)
	#define PRINT_LD(var)
	#define PRINT_U(var)
	#define PRINT_LU(var)
	#define PRINT_ZU(var)
	#define PRINT_F(var)
	#define WHERE
	#define GL_GET_ERROR
	#define BREAKPOINT
	#define BREAKPOINT_IF(expr)
#endif
extern bool enable_breakpoints;

[[maybe_unused]] constexpr std::size_t INVALID_ID
	= std::numeric_limits<std::size_t>::max();

#define ARR_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define AT3(x, y, z) \
	[static_cast<size_t>(x)] \
	[static_cast<size_t>(y)] \
	[static_cast<size_t>(z)]

#define AT3_M(x, y, z, Mx, My, Mz) \
	[static_cast<size_t>((x%Mx+Mx)%Mx)] \
	[static_cast<size_t>((y%My+My)%My)] \
	[static_cast<size_t>((z%Mz+Mz)%Mz)]

template<class F>
inline F mod_f(F a, F m) {
	static_assert(std::is_floating_point_v<F>);
	assert(m > F(0));

	return a > F(0) ?
		a - std::floor(a/m)*m :
		a + std::ceil(-a/m)*m;
}

int floor_div(int num, int den);
std::pair<long long, long long> floor_div_rem(long long num, long long den);

inline int ceil_div(const int num, const int den) {
	return 1 + ((num - 1) / den);
}

inline std::size_t ceil_div(const std::size_t num, const std::size_t den) {
	return (num + den - 1) / den;
}

template<class T>
inline T sign_of(const T x) {
	return
		x < T(0) ?
		T(-1) :

		x > T(0) ?
		T( 1) :

		T(0);
}

template<class T>
inline bool in_between_inclusive(T a, T b, T x) {
	return a <= x && x <= b;
}

template<class T>
bool min_replace(T &a, const T &b) {
	if (b < a) {
		a = b;
		return true;
	} else
		return false;
}

template<class T>
bool max_replace(T &a, const T &b) {
	if (b > a) {
		a = b;
		return true;
	} else
		return false;
}

template<class T>
struct vec2_cmp_t {
	inline bool operator()(
			const glm::tvec2<T, glm::highp> &a,
			const glm::tvec2<T, glm::highp> &b) const {
		return a.x < b.x || (a.x == b.x && a.y < b.y);
	}
};

inline glm::vec3 color_hex_to_vec3(uint32_t hex) {
	return glm::vec3(
		static_cast<double>( (hex & 0xff0000u) >> 16  ) / 255.0,
		static_cast<double>( (hex & 0x00ff00u) >> 8  ) / 255.0,
		static_cast<double>( (hex & 0x0000ffu) >> 0 ) / 255.0
	);
}

inline uint32_t color_vec3_to_hex(const glm::vec3 v) {
	return
		static_cast<uint32_t>(v.r * 255.0) << 16 |
		static_cast<uint32_t>(v.g * 255.0) << 8  |
		static_cast<uint32_t>(v.b * 255.0) << 0;
}

inline glm::u8vec3 color_hex_to_u8vec3(uint32_t hex) {
	return glm::u8vec3(
		(hex & 0x0000ffu) >> 0,
		(hex & 0x00ff00u) >> 8,
		(hex & 0xff0000u) >> 16
	);
}

inline constexpr uint32_t rgba_to_abgr(uint32_t hex) {
	return
		((hex & 0x00'00'00'ffu) << 24u) |
		((hex & 0x00'00'ff'00u) <<  8u) |
		((hex & 0x00'ff'00'00u) >>  8u) |
		((hex & 0xff'00'00'00u) >> 24u);
}

uint32_t hsv_to_rgb(float h, float s, float v);

inline glm::vec3 vec2_to_vec3(const glm::vec2& vec) {
	return glm::vec3(vec.x, vec.y, 0.0f);
}

template<class T>
inline T len_sq(const glm::tvec3<T, glm::highp> &vec) {
    return vec.x*vec.x + vec.y*vec.y + vec.z*vec.z;
}
inline long long len_sq(const glm::ivec2 &vec) {
	return vec.x*vec.x + vec.y*vec.y;
}
inline double len_sq(const glm::dvec2 &vec) {
	return vec.x*vec.x + vec.y*vec.y;
}
inline float len_sq(const glm::vec2 &vec) {
	return vec.x*vec.x + vec.y*vec.y;
}

inline bool same_direction_knowing_same_line(
		const glm::dvec2 &v, const glm::dvec2 &w) {
	return
		sign_of(v.x) == sign_of(w.x) &&
		sign_of(v.y) == sign_of(w.y);
}

template<class T, class S>
T lerp(T a, T b, S x) {
	x = glm::clamp<S>(x, 0, 1);
	return a*(S(1)-x) + b*x;
}

#endif
