#pragma once
#ifndef NOISE_HPP
#define NOISE_HPP

#include <cmath>

// Source: https://en.wikipedia.org/wiki/Perlin_noise

/*
* Function to linearly interpolate between a0 and a1
* Weight w should be in the range [0.0, 1.0]
*/
float interpolate(float a0, float a1, float w);

typedef struct {
	float x, y;
} vector2;

// Create pseudorandom direction vector
vector2 randomGradient(int ix, int iy);

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y);

// Compute Perlin noise at coordinates x, y
float perlin(float x, float y);

#endif
