#pragma once

template <typename T>
struct Color4
{
	union {
		T components[4];
		struct
		{
			T r;
			T g;
			T b;
			T a;
		};
	};

	Color4() = default;
	Color4(T nr, T ng, T nb, T na)
	{
		r = nr;
		g = ng;
		b = nb;
		a = na;
	}
};

#define COLOR_RGBA_TO_LINEAR(r, g, b, a) (r) / 255.f, (g) / 255.f, (b) / 255.f, (a) / 255.f

// Some nice colors, for convenience
// Note that these are not "true" red etc., so don't rely on them for that
namespace Color
{
extern const Color4<float> Red;
extern const Color4<float> Green;
extern const Color4<float> Blue;
extern const Color4<float> Yellow;
extern const Color4<float> Orange;
extern const Color4<float> Purple;
}  // namespace Color
