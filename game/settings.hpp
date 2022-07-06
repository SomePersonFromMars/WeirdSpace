#pragma once
#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#define ATMOSPHERE 2
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

#endif
