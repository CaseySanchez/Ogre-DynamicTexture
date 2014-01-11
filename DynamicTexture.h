#pragma once

#include <amp.h>
#include <amp_math.h>

#include <OgreHardwarePixelBuffer.h>

using namespace concurrency;
using namespace concurrency::fast_math;

#define BYTE_ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define WORD_ARGB(a, r, g, b) ((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff))
#define WORD_A(c) ((c) >> 24)
#define WORD_R(c) (((c) >> 16) & 0xff)
#define WORD_G(c) (((c) >> 8) & 0xff)
#define WORD_B(c) ((c) & 0xff)
#define ValidTexCoord(x, y, width, height) ((x) >= 0 && (x) < (width) && (y) >= 0 && (y) < (height))

namespace dtkernels
{
	void CircleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 radius, uint32 color, bool fade);
	void RectangleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 width, uint32 height, uint32 color);
	void TriangleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 ax, uint32 ay, uint32 bx, uint32 by, uint32 cx, uint32 cy, uint32 color);
	void TextureKernel(array<uint32, 2> &buffer, array_view<const uint32, 2> texture, uint32 x, uint32 y, Real angle, Real scale, bool alpha);
	void ApproachKernel(array<uint32, 2> &buffer, uint32 color);
	void BleedKernel(array<uint32, 2> &buffer, bool lessthan);
};

template<typename T, int32 Rank>
void SetKernel(array<T, Rank> &arr, T val)
{
	parallel_for_each(arr.extent, [&arr, val](index<Rank> idx) restrict(amp)
	{
		arr(idx) = val;
	}); 
}

class DynamicTexture
{
	array<uint32, 2>
		buffer;
   
public:
	static uint32
		id;

	const int32
		width, 
		height;

	Ogre::TexturePtr
		textureptr;

	DynamicTexture(const int32 width, const int32 height, uint32 color = 0);
	~DynamicTexture();

	void Circle(uint32 x, uint32 y, uint32 radius, uint32 color, bool fade = false);
	void Rectangle(uint32 x, uint32 y, uint32 width, uint32 height, uint32 color);
	void Triangle(uint32 x, uint32 y, uint32 ax, uint32 ay, uint32 bx, uint32 by, uint32 cx, uint32 cy, uint32 color);
	void Texture(Ogre::TexturePtr texture, uint32 x, uint32 y, Real angle = 0.F, Real scale = 1.F, bool alpha = false);
	void Approach(uint32 color);
	void Bleed(bool lessthan = false);

	void CopyToBuffer();
	void Reset(uint32 color = 0);
};
