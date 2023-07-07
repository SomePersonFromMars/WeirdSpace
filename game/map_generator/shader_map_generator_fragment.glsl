#version 450 core

out vec4 color;

in VS_OUT {
	float elevation;
	vec2 pos;
} fs_in;

uniform vec2 space_max;
uniform float t;

const float PI = 3.14159265359;

// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20201014 (stegu)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
//
vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i);
	vec4 p = permute( permute( permute(
					i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
				+ i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
			+ i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
				dot(p2,x2), dot(p3,x3) ) );
}

// Source: https://thebookofshaders.com/13/
#define OCTAVES 6
float fbm (in vec3 st) {
	// Initial values
	float value = 0.0;
	float amplitude = .5;
	float frequency = 0.;
	//
	// Loop of octaves
	for (int i = 0; i < OCTAVES; i++) {
		value += amplitude * snoise(st);
		st *= 2.;
		amplitude *= .5;
	}
	return value;
}

float fbm_warped(in vec3 st) {
	float x = fbm(st + vec3(23.234, 64.123, 0.0));
	float y = fbm(st + vec3(98.154, 91.923, 0.0));
	return fbm(st + vec3(x, y, 0.0) * 0.7);
}

vec3 hsv_to_rgb(vec3 c) {
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 hsv_to_rgb(float h, float s, float v) {
	return hsv_to_rgb(vec3(h, s, v));
}

float usual_noise(vec2 P) {
	return fbm_warped(vec3(P * 3.0, t / 10000.0)) + 0.5;
}

// float cyclic_noise_cyllinder(vec2 P) {
// 	const float radius = space_max.x / 2.0 / PI;
// 	const float angle = P.x / radius;
// 	const vec3 Q = vec3(
// 			sin(angle) * radius + t / 10000.0,
// 			cos(angle) * radius + t / 10000.0,
// 			P.y);
// 	return fbm_warped(Q * 3.0) + 0.5;
// }

float cyclic_noise_heuristic(vec2 P) {
	const float width = space_max.x;
	P.x -= floor(P.x / width) * width;
	const float margin = width * 0.1;
	const float n1 = usual_noise(P);
	float n = n1;
	if (P.x > width - margin) {
		const float over_margin = P.x - (width - margin);
		const float n2 = usual_noise(vec2(over_margin - margin, P.y));
		n = mix(n1, n2, over_margin / margin);
	}
	return n;
}

void main(void) {
	float elevation = fs_in.elevation;
	{
		const float x = elevation;
		const float xx = x*x;
		const float xxx = xx*x;
		elevation = (-3.0*xxx + 4.0*xx + x) / 2.0;
	}

	const vec2 P = fs_in.pos - vec2(space_max.x, 0.0);
	// const float noise_val = usual_noise(P);
	// const float noise_val = cyclic_noise_cyllinder(P);
	const float noise_val = cyclic_noise_heuristic(P);

	float noised_elevation
		= -0.4 + elevation * 0.8
		+ pow(noise_val, 1.3) * (0.6 + elevation * 0.0);
	noised_elevation = max(noised_elevation, 0.0);

	float hue = noised_elevation;
	if (hue < 0.5) hue = 0.15 + 0.25 * hue;
	else hue = 0.0 + 1.0 * hue;

	// color.rgb = vec3(elevation);

	color.rgb = hsv_to_rgb(
		(1.0 - hue) * 240.0 / 360.0, 0.6, 0.8);

	color.a = noised_elevation;

	// color = vec4(noise_val, noise_val, noise_val, 1.0);
}
