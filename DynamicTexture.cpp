#include "stdafx.h"

#include "DynamicTexture.h"

void dtkernels::CircleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 radius, uint32 color, bool fade)
{
	int32
		ca = WORD_A(color),
		cr = WORD_R(color),
		cg = WORD_G(color),
		cb = WORD_B(color);
	Real
		rsq = Real(radius * radius);

	parallel_for_each(buffer.extent, [=, &buffer](index<2> idx) restrict(amp)
	{
		Real
			dsq = Real(((idx[1] - x) * (idx[1] - x)) + ((idx[0] - y) * (idx[0] - y)));

		if(dsq < rsq)
		{
			if(fade)
			{
				Real
					opacity = 1.F - (dsq / rsq);

				int32
					ba = WORD_A(buffer(idx)),
					br = WORD_R(buffer(idx)),
					bg = WORD_G(buffer(idx)),
					bb = WORD_B(buffer(idx)),
					a = ba + int32(Real(ca - ba) * opacity),
					r = br + int32(Real(cr - br) * opacity),
					g = bg + int32(Real(cg - bg) * opacity),
					b = bb + int32(Real(cb - bb) * opacity);

				buffer(idx) = WORD_ARGB(a, r, g, b);
			}
			else
			{
				buffer(idx) = color;
			}
		}
	});
}
	
void dtkernels::RectangleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 width, uint32 height, uint32 color)
{
	parallel_for_each(buffer.extent, [=, &buffer](index<2> idx) restrict(amp)
	{
		if(idx[1] >= int32(x) && idx[1] <= int32(x + width) && idx[0] >= int32(y) && idx[0] <= int32(y + height))
		{
			buffer(idx) = color;
		}
	});
}

void dtkernels::TriangleKernel(array<uint32, 2> &buffer, uint32 x, uint32 y, uint32 ax, uint32 ay, uint32 bx, uint32 by, uint32 cx, uint32 cy, uint32 color)
{
	bx -= ax;
	by -= ay;
	cx -= ax;
	cy -= ay;

	int32
		d0 = (cx * cx) + (cy * cy),
		d1 = (cx * bx) + (cy * by),
		d2 = (bx * bx) + (by * by);

	parallel_for_each(buffer.extent, [=, &buffer](index<2> idx) restrict(amp)
	{
		int32
			px = idx[1] - ax, 
			py = idx[0] - ay,
			d3 = (cx * px) + (cy * py),
			d4 = (bx * px) + (by * py);

		Real
			i = 1.F / ((d0 * d2) - (d1 * d1)),
			u = ((d2 * d3) - (d1 * d4)) * i,
			v = ((d0 * d4) - (d1 * d3)) * i;

		if(u >= 0.F && v >= 0.F && u + v <= 1.F)
		{
			buffer(idx) = color;
		}
	});
}

void dtkernels::TextureKernel(array<uint32, 2> &buffer, array_view<const uint32, 2> texture, uint32 x, uint32 y, Real angle, Real scale, bool alpha)
{
	const int32
		cx = texture.extent[1] / 2,
		cy = texture.extent[0] / 2;

	angle += Ogre::Math::HALF_PI;
			
	parallel_for_each(buffer.extent, [=, &buffer](index<2> idx) restrict(amp)
	{
		int32
			tex_x = idx[1] - x,
			tex_y = idx[0] - y;
		Real
			dist = sqrt(Real((tex_x * tex_x) + (tex_y * tex_y))) / scale,
			theta = atan2(Real(tex_y), Real(tex_x)) - angle;

		tex_x = int32(dist * sin(theta)) + cx;
		tex_y = int32(dist * cos(theta)) + cy;

		if(ValidTexCoord(tex_x, tex_y, texture.extent[1], texture.extent[0]))
		{
			int32
				color = texture(tex_y, tex_x);

			if(!alpha || (alpha && color != 0))
			{
				buffer(idx) = color;
			}
		}
	});
}

void dtkernels::ApproachKernel(array<uint32, 2> &buffer, uint32 color)
{
	int32
		ca = WORD_A(color),
		cr = WORD_R(color),
		cg = WORD_G(color),
		cb = WORD_B(color);

	parallel_for_each(buffer.extent, [=, &buffer](index<2> idx) restrict(amp)
	{
		int32
			ba = WORD_A(buffer(idx)),
			br = WORD_R(buffer(idx)),
			bg = WORD_G(buffer(idx)),
			bb = WORD_B(buffer(idx)),
			a = (ca - ba) / 2,
			r = (cr - br) / 2,
			g = (cg - bg) / 2,
			b = (cb - bb) / 2;

		a = (a <= 1) ? (ca) : (a + ba);
		r = (r <= 1) ? (cr) : (r + br);
		g = (g <= 1) ? (cg) : (g + bg);
		b = (b <= 1) ? (cb) : (b + bb);

		buffer(idx) = WORD_ARGB(a, r, g, b);
	});
}

void dtkernels::BleedKernel(array<uint32, 2> &buffer, bool lessthan)
{
	array<uint32, 2>
		tmp(buffer.extent);
   
	parallel_for_each(buffer.extent, [=, &buffer, &tmp](index<2> idx) restrict(amp)
	{
		const int32
			offset[49][2] = 
			{
				{ -4 , 0 }, { -3 , -2 }, { -3 , -1 }, { -3 , 0 }, { -3 , 1 }, { -3 , 2 }, { -2 , -3 }, { -2 , -2 }, { -2 , -1 }, { -2 , 0 }, { -2 , 1 }, { -2 , 2 }, { -2 , 3 }, { -1 , -3 }, { -1 , -2 }, { -1 , -1 }, { -1 , 0 }, { -1 , 1 }, { -1 , 2 }, { -1 , 3 }, { 0 , -4 }, { 0 , -3 }, { 0 , -2 }, { 0 , -1 }, { 0 , 0 }, { 0 , 1 }, { 0 , 2 }, { 0 , 3 }, { 0 , 4 }, { 1 , -3 }, { 1 , -2 }, { 1 , -1 }, { 1 , 0 }, { 1 , 1 }, { 1 , 2 }, { 1 , 3 }, { 2 , -3 }, { 2 , -2 }, { 2 , -1 }, { 2 , 0 }, { 2 , 1 }, { 2 , 2 }, { 2 , 3 }, { 3 , -2 }, { 3 , -1 }, { 3 , 0 }, { 3 , 1 }, { 3 , 2 }, { 4 , 0 }
			};

		int32
			x, y;
		uint32
			color,
			max[4] = { (lessthan) ? (~0) : (0) },
			a = 0, r = 0, g = 0, b = 0,
			num = 0;

		for(uint32 i = 0; i < 49; ++i)
		{
			x = idx[1] + offset[i][0];
			y = idx[0] + offset[i][1];

			if(ValidTexCoord(x, y, buffer.extent[1], buffer.extent[0]))
			{
				color = buffer(y, x);

				a += WORD_A(color);
				r += WORD_R(color);
				g += WORD_G(color);
				b += WORD_B(color);

				num++;

				if((WORD_A(color) < max[0]) == lessthan)
				{
					max[0] = WORD_A(color);
				}

				if((WORD_R(color) < max[1]) == lessthan)
				{
					max[1] = WORD_R(color);
				}

				if((WORD_G(color) < max[2]) == lessthan)
				{
					max[2] = WORD_G(color);
				}

				if((WORD_B(color) < max[3]) == lessthan)
				{
					max[3] = WORD_B(color);
				}
			}
		}

		a = ((a / num) + max[0]) / 2;
		r = ((r / num) + max[1]) / 2;
		g = ((g / num) + max[2]) / 2;
		b = ((b / num) + max[3]) / 2;

		tmp(idx) = WORD_ARGB(a, r, g, b);
	});
   
	buffer = tmp;
}

uint32 
	DynamicTexture::id = 0;

DynamicTexture::DynamicTexture(const int32 width, const int32 height, uint32 color) : width(width), height(height),	buffer(extent<2>(height, width))
{
	SetKernel(buffer, color);
	
	textureptr = Ogre::TextureManager::getSingleton().createManual("DynamicTexture" + Ogre::StringConverter::toString(++id), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TextureType::TEX_TYPE_2D, width, height, 0, Ogre::PixelFormat::PF_A8R8G8B8);
}

DynamicTexture::~DynamicTexture()
{
	Ogre::TextureManager::getSingleton().remove(textureptr->getHandle());
}

void DynamicTexture::Circle(uint32 x, uint32 y, uint32 radius, uint32 color, bool fade)
{
	dtkernels::CircleKernel(buffer, x, y, radius, color, fade);
}

void DynamicTexture::Rectangle(uint32 x, uint32 y, uint32 width, uint32 height, uint32 color)
{
	dtkernels::RectangleKernel(buffer, x, y, width, height, color);
}

void DynamicTexture::Triangle(uint32 x, uint32 y, uint32 ax, uint32 ay, uint32 bx, uint32 by, uint32 cx, uint32 cy, uint32 color)
{
	dtkernels::TriangleKernel(buffer, x, y, ax, ay, bx, by, cx, cy, color);
}

void DynamicTexture::Texture(Ogre::TexturePtr texture, uint32 x, uint32 y, Real angle, Real scale, bool alpha)
{
	Ogre::HardwarePixelBufferSharedPtr 
		pixelbuffer = texture->getBuffer();

	dtkernels::TextureKernel(buffer, array_view<const uint32, 2>(texture->getHeight(), texture->getWidth(), (uint32 *)pixelbuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY)), x, y, angle, scale, alpha);
	
	pixelbuffer->unlock();
}

void DynamicTexture::Approach(uint32 color)
{
	dtkernels::ApproachKernel(buffer, color);
}
	
void DynamicTexture::Bleed(bool lessthan)
{
	dtkernels::BleedKernel(buffer, lessthan);
}

void DynamicTexture::CopyToBuffer()
{
	Ogre::HardwarePixelBufferSharedPtr 
		pixelbuffer = textureptr->getBuffer();

	copy(buffer, stdext::make_checked_array_iterator<uint32 *>((uint32 *)pixelbuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD), width * height));

	pixelbuffer->unlock();
}

void DynamicTexture::Reset(uint32 color)
{
	SetKernel(buffer, color);
}