#include "generator.hpp"

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <vector>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "useful.hpp"
#include "settings.hpp"

#include "noise.hpp"
#include <MyGAL/FortuneAlgorithm.h>
using namespace mygal;

generator_A_t::generator_A_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
{
	noise.border_end = double(width)*noise_pos_mult;
	noise.border_beg = noise.border_end;
	noise.border_beg -= double(CHUNK_DIM)*noise_pos_mult*0.1;

	new_seed();
}

void generator_A_t::new_seed() {
	cyclic_noise_t::seed_type seed;
	seed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() / 100;

	noise.reseed(seed);
}

u8vec3 generator_A_t::get(ivec2 ipos) {
	const int x = ipos.x;
	const int y = ipos.y;

	// int octaves = (1.0 - double(y)/double(height)) * 8.0;

	double val;
	val = noise.octave2D_01(
		double(x) * noise_pos_mult,
		double(y) * noise_pos_mult,
		8,
		// octaves,
		0.5
	)*1.0;
	// val = 1.0 - val;

	// val *= val;
	const double t = double(y)/double(height)*4.0/3.0;
	val = lerp(val*0.5+0.5, val*val, t*t*t*t);

	uint8_t val_u8 = static_cast<uint8_t>(val * 255.0);
	return u8vec3(val_u8);

	vec3 val_v3 = lerp<vec3, float>(
		// color_hex_to_vec3(0x826348),
		color_hex_to_vec3(0x826948),

		color_hex_to_vec3(0xf9d186),
		val);

	// vec3 val_v3(1);

	// return val_u8v3;
	return u8vec3(
		static_cast<uint8_t>(val_v3.r * 255.0),
		static_cast<uint8_t>(val_v3.g * 255.0),
		static_cast<uint8_t>(val_v3.b * 255.0)
	);
}


void generator_A_t::generate_bitmap(bitmap_t &bitmap, int resolution_div) {

	for (int x = 0; x < bitmap.WIDTH; x += resolution_div)
		for (int y = 0; y < bitmap.HEIGHT; y += resolution_div) {
			const u8vec3 val = get({x, y});

			for (int dx = 0; dx < resolution_div; ++dx)
				for (int dy = 0; dy < resolution_div; ++dy) {
					if (x+dx >= width) continue;
					if (y+dy >= height) continue;

					bitmap.get(y+dy, x+dx, 0) = val.r;
					bitmap.get(y+dy, x+dx, 1) = val.g;
					bitmap.get(y+dy, x+dx, 2) = val.b;
				}
		}

	// World border
	for (int x = 0; x < bitmap.WIDTH; ++x) {
		bitmap.get(0, x, 0) = 0;
		bitmap.get(0, x, 1) = 0;
		bitmap.get(0, x, 2) = 0xff;

		bitmap.get(bitmap.HEIGHT-1, x, 0) = 0;
		bitmap.get(bitmap.HEIGHT-1, x, 1) = 0;
		bitmap.get(bitmap.HEIGHT-1, x, 2) = 0xff;
	}
	for (int y = 0; y < bitmap.HEIGHT; ++y) {
		bitmap.get(y, 0, 0) = 0;
		bitmap.get(y, 0, 1) = 0;
		bitmap.get(y, 0, 2) = 0xff;

		bitmap.get(y, bitmap.WIDTH-1, 0) = 0;
		bitmap.get(y, bitmap.WIDTH-1, 1) = 0;
		bitmap.get(y, bitmap.WIDTH-1, 2) = 0xff;
	}

	// Chunks visualization
	for (int x = CHUNK_DIM; x < bitmap.WIDTH; x += CHUNK_DIM) {

		for (int y = bitmap.HEIGHT/8*7; y < bitmap.HEIGHT; ++y) {
			bitmap.get(y, x, 0) = 0xff;
		}
	}
}

generator_B_t::generator_B_t()
	:width{bitmap_t::WIDTH}
	,height{bitmap_t::HEIGHT}
	,diagram(generate_diagram(20))
{
}

void generator_B_t::new_seed() {
	diagram = generate_diagram(20);
}

mygal::Diagram<float> generator_B_t::generate_diagram(size_t points_cnt) {
	// Generate points
	const float max_x = float(width) / float(height);
	// const float max_x = 1;
	// const float max_y = float(height) / float(width);
	const float max_y = 1;
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	auto rand_generator = std::default_random_engine(seed);
	auto distribution = std::uniform_real_distribution<float>(0, 1);

	std::set<vec2, vec2_cmp_t<float>> points_set;
	auto points = std::vector<Vector2<float>>(points_cnt*3);
	// auto points = std::vector<Vector2<float>>(points_cnt*2);
	// auto points = std::vector<Vector2<float>>(points_cnt);
	for (auto i = 0; i < points_cnt; ++i) {
		vec2 new_point;
		while (!points_set.insert(new_point = vec2(
				distribution(rand_generator) * max_x,
				distribution(rand_generator) * max_y
			)).second)
			;

		points[i] = Vector2<float>(
			// distribution(rand_generator) * max_x,
			// distribution(rand_generator) * max_y
				new_point.x,
				new_point.y
			);

		points[i+points_cnt] = Vector2<float>(
				points[i].x + max_x,
				points[i].y
			);

		points[i+points_cnt*2] = Vector2<float>(
				points[i].x + max_x*2.0f,
				points[i].y
			);
	}

	Box<float> box{0, 0, max_x*3.0f, max_y};
	// Box<float> box{0, 0, max_x*2.0f, 1.0f};
	// Box<float> box{0, 0, max_x*1.0f, 1.0f};
	Box<float> box_bounding{
		box.left - 0.5f,
		box.bottom - 0.5f,
		box.right + 0.5f,
		box.top + 0.5f
	};

	// Construct diagram
	auto algorithm = FortuneAlgorithm<float>(points);
	algorithm.construct();

	// Bound the diagram
	algorithm.bound(box_bounding);
	auto generated_diagram = algorithm.getDiagram();

	// Intersect the diagram with a box
	// generated_diagram.intersect(Box<float>{-max_x, 0, max_x*2.0f, max_y});
	// generated_diagram.intersect(box);
	// generated_diagram.intersect(Box<float>{0, 0, max_x, max_y});
	// generated_diagram.intersect(Box<float>{-max_x, 0, max_x, max_y});

	// // Relaxation
	// points = generated_diagram.computeLloydRelaxation();

	// // Construct diagram
	// algorithm = FortuneAlgorithm<float>(points);
	// algorithm.construct();

	// // Bound the diagram
	// algorithm.bound(box_bounding);
	// generated_diagram = algorithm.getDiagram();

	return generated_diagram;
}

void generator_B_t::draw_edge(bitmap_t &bitmap,
		Diagram<float>::HalfEdge half_edge) {
	vec2 beg, end;

	beg.x = half_edge.origin->point.x*float(width-1);
	beg.x /= float(width) / float(height);
	beg.y = half_edge.origin->point.y*float(height-1);

	end.x = half_edge.destination->point.x*float(width-1);
	end.x /= float(width) / float(height);
	end.y = half_edge.destination->point.y*float(height-1);

	beg -= vec2(float(width), 0);
	end -= vec2(float(width), 0);

	vec2 off = end - beg;
	const int iterations_cnt = std::max(
			std::abs(int(off.x)), std::abs(int(off.y)));
	if (std::abs(off.x) > std::abs(off.y)) {
		off /= std::abs(off.x);
	} else {
		off /= std::abs(off.y);
	}

	vec2 pos = beg;
	for (int i = 0; i < iterations_cnt; ++i, pos += off) {
		if (pos.x < 0 || pos.x >= float(width)
				|| pos.y < 0 || pos.y >= float(height))
			continue;

		bitmap.get(int(pos.y), int(pos.x), 0) = 0xff;
		bitmap.get(int(pos.y), int(pos.x), 1) = 0xff;
		bitmap.get(int(pos.y), int(pos.x), 2) = 0xff;
	}
}

void generator_B_t::generate_bitmap(bitmap_t &bitmap) {
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			bitmap.get(y, x, 0) = 0;
			bitmap.get(y, x, 1) = 0;
			bitmap.get(y, x, 2) = 0;
		}
	}

	for (const auto& site : diagram.getSites())
	{
		auto center = site.point;
		auto face = site.face;
		auto halfEdge = face->outerComponent;
		if (halfEdge == nullptr)
			continue;
		while (halfEdge->prev != nullptr)
		{
			halfEdge = halfEdge->prev;
			if (halfEdge == face->outerComponent)
				break;
		}
		auto start = halfEdge;
		while (halfEdge != nullptr)
		{
			if (halfEdge->origin != nullptr &&
				halfEdge->destination != nullptr)
			{
				draw_edge(bitmap, *halfEdge);
			}
			halfEdge = halfEdge->next;
			if (halfEdge == start)
				break;
		}
	}
}
