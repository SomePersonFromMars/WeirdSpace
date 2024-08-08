// Copyright (C) 2024, Kacper Orszulak
// GNU General Public License v3.0+ (see LICENSE.txt or https://www.gnu.org/licenses/gpl-3.0.txt)

#pragma once
#ifndef BLOCK_MODEL_HPP
#define BLOCK_MODEL_HPP

#include "GL/glew.h"

namespace block_model {
    // Vertices positions, textures' UVs and normals
	// Faces order: Front, Top, Left, Right, Bottom, Back
	static constexpr GLfloat POSITIONS[] = {
		0, 0, 0,  0,
		0, 1, 0,  0,
		1, 0, 0,  0,
		1, 0, 0,  0,
		0, 1, 0,  0,
		1, 1, 0,  0,
		0, 1, 0,  0,
		0, 1, 1,  0,
		1, 1, 0,  0,
		1, 1, 0,  0,
		0, 1, 1,  0,
		1, 1, 1,  0,
		1, 0, 0,  0,
		1, 1, 0,  0,
		1, 0, 1,  0,
		1, 0, 1,  0,
		1, 1, 0,  0,
		1, 1, 1,  0,
		0, 0, 1,  0,
		0, 1, 1,  0,
		0, 0, 0,  0,
		0, 0, 0,  0,
		0, 1, 1,  0,
		0, 1, 0,  0,
		0, 0, 1,  0,
		0, 0, 0,  0,
		1, 0, 1,  0,
		1, 0, 1,  0,
		0, 0, 0,  0,
		1, 0, 0,  0,
		1, 0, 1,  0,
		1, 1, 1,  0,
		0, 0, 1,  0,
		0, 0, 1,  0,
		1, 1, 1,  0,
		0, 1, 1,  0,
	};

#define VERTEX_UV(off_x, off_y, mult_x, mult_y, x, y) \
	(off_x) + (mult_x)*static_cast<double>(x), \
	(off_y) + (mult_y)*static_cast<double>(y), \
	0, 0,
#define FACE_UVS(off_x, off_y, mult_x, mult_y) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 0) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 1) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 1, 0) \
	VERTEX_UV(off_x, off_y, mult_x, mult_y, 0, 0)

#define SAND_FACE_UVS \
	FACE_UVS(0.0, 0, 0.125, 1.0)

#define BRICK_FACE_UVS \
	FACE_UVS(0.375, 0, 0.125, 1.0)

#define CACTUS_FACE_UVS \
	FACE_UVS(0.75, 0, 0.125, 1.0)

	static constexpr GLfloat UVS[] = {
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		SAND_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		BRICK_FACE_UVS
		CACTUS_FACE_UVS
		CACTUS_FACE_UVS
		CACTUS_FACE_UVS
		CACTUS_FACE_UVS
		CACTUS_FACE_UVS
		CACTUS_FACE_UVS
	};

#undef SAND_FACE_UVS
#undef BRICK_FACE_UVS
#undef CACTUS_FACE_UVS
#undef FACES_UVS
#undef VERTEX_UV

	static constexpr GLfloat NORMALS[] = {
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 0, -1,  0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		0, 1, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		1, 0, 0,   0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		-1, 0, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, -1, 0,  0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
		0, 0, 1,   0,
	};
}

#endif
