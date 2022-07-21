#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

// Minimal frame duration in ms
// #define FRAME_MIN_DURATION 16 // 60fps
#define FRAME_MIN_DURATION 33 // 30fps
// #define FRAME_MIN_DURATION 120

#define SHADER_A_VERTEX_PATH "runtime/shader_A_vertex.glsl"
#define SHADER_A_FRAGMENT_PATH "runtime/shader_A_fragment.glsl"

#define ATMOSPHERE 4
#if ATMOSPHERE == 1
	// Soft-blue, soft-yellow
	#define SKY_COLOR 0xd7e6e8
	#define LIGHT_COLOR 0xfce7b5
#endif
#if ATMOSPHERE == 2
	// Soft-blue, soft-blue
	// #define SKY_COLOR 0x00ccff // Blue
	// #define SKY_COLOR 0x9deff9
	#define SKY_COLOR 0xc9edf2
	// #define SKY_COLOR 0xd7e6e8 // Soft-blue

	// #define LIGHT_COLOR 0xeafcfa // Soft-blue
	// #define LIGHT_COLOR 0xedfcfa
	// #define LIGHT_COLOR 0xf4fcfb
	#define LIGHT_COLOR 0xffffff // White
#endif
#if ATMOSPHERE == 3
	// Orange sunset
	#define SKY_COLOR 0xfcb967
	#define LIGHT_COLOR 0xf7d5ad
#endif
#if ATMOSPHERE == 4
	// Soft orange desert
	#define SKY_COLOR 0xfccc92
	#define LIGHT_COLOR 0xf7d5ad
#endif
#if ATMOSPHERE == 5
	// Contrast
	#define SKY_COLOR 0x000000
	#define LIGHT_COLOR 0xffffff
#endif

#endif
