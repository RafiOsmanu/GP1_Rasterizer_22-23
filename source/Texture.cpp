#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <memory>
#include <iostream>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	std::unique_ptr<Texture> Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		SDL_Surface* data = IMG_Load(path.c_str());

		//Create & Return a new Texture Object (using SDL_Surface)
		if (data == nullptr)
		{
			std::cout << "surface is nullptr" << "\n";
		}

		return std::make_unique<Texture> (data);
		
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv
		Uint8 r, g, b;

		const Uint32 U{static_cast<Uint32>(uv.x * m_pSurface->w) };
		const Uint32 V{ static_cast< Uint32>(uv.y * m_pSurface->h) };
		
		
		const Uint32 pixel{ m_pSurfacePixels[static_cast<Uint32>(U + (V * m_pSurface->w))]};

		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		const float colorRemap{ 1 / 255.f };


		return { r * colorRemap, g * colorRemap, b * colorRemap };
	}
}