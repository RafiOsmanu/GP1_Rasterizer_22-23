#pragma once
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"
#include<memory>
#include "Vector3.h"

namespace dae
{
	struct Vector2;

	class Texture
	{
	public:
		~Texture();

		static std::unique_ptr<Texture> LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;
		Vector3 SampleNormal(const Vector2& uv) const;
		Texture(SDL_Surface* pSurface);

	private:

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}