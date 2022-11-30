//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{

	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//init depthBuffer
	m_pDepthBufferPixels = new float[m_Width * m_Height]();

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;


	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

	/*std::vector<Vector3> veritces_ndc
	{
		{0.f, .5f, 1.f},
		{.5f, -.5f, 1.f},
		{-.5f, -.5f, 1.f}
	};*/



};

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//init Depth Buffer with FLT_MAX
	std::fill_n(m_pDepthBufferPixels, (m_Width * m_Height), FLT_MAX);
	
	//clear buffer
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));

	//@START
	
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);



	//render_W1_Part1(); //rasterize only 
	//render_W1_Part2(); //projection stage
	//render_W1_Part3(); //barycentric coordinates
	//render_W1_Part4(); //Depth Buffer
	//render_W1_Part5();//optimization
	render_W2_Part1();//Quads



	//@END
	
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//Todo > W1 Projection Stage
	float aspectRatio{ float(m_Width) / float(m_Height) };
	vertices_out = vertices_in;
	for (auto& vertexOut : vertices_out)
	{

		//to viewSpace
		vertexOut.position = m_Camera.viewMatrix.TransformPoint(vertexOut.position);

		//perspective divide
		vertexOut.position.x = vertexOut.position.x / vertexOut.position.z;
		vertexOut.position.y = vertexOut.position.y / vertexOut.position.z;
		
		//projectedSpace
		vertexOut.position.x = vertexOut.position.x / (aspectRatio * m_Camera.fov);
		vertexOut.position.y = vertexOut.position.y / m_Camera.fov;


		//screenSpace
		vertexOut.position.x = (vertexOut.position.x + 1) / 2 * float(m_Width);
		vertexOut.position.y = (1 - vertexOut.position.y) / 2 * float(m_Height);
	}
}

bool dae::Renderer::IsInTriangle(const std::vector<Vector2>& verticesScreenspace, const Vector2& pixelPos)
{

	Vector2 v0{ verticesScreenspace[0].x, verticesScreenspace[0].y };
	Vector2 v1{ verticesScreenspace[1].x, verticesScreenspace[1].y };
	Vector2 v2{ verticesScreenspace[2].x, verticesScreenspace[2].y };
	

	Vector2 edgeA{ v1 - v0 };
	Vector2 edgeB{ v2 - v1 };
	Vector2 edgeC{ v0 - v2 };

	Vector2 pointToSideA{ pixelPos - v0 };
	Vector2 pointToSideB{ pixelPos - v1 };
	Vector2 pointToSideC{ pixelPos - v2 };


	if (Vector2::Cross(edgeA, pointToSideA) < 0) return false;
	if (Vector2::Cross(edgeB, pointToSideB) < 0) return false;
	if (Vector2::Cross(edgeC, pointToSideC) < 0) return false;

	return true;
}



//void dae::Renderer::render_W1_Part1()
//{
//	std::vector<Vertex>veritces_ndc =
//	{
//	   {{0.f, .5f, 1.f}},
//	   {{.5f, -.5f, 1.f}},
//	   {{-.5f, -.5f, 1.f}},
//	};
//
//	for (auto& Vertex : veritces_ndc)
//	{
//		Vertex.position.x = (Vertex.position.x + 1) / 2 * float(m_Width);
//		Vertex.position.y = (1 - Vertex.position.y) / 2 * float(m_Height);
//	}
//
//	//RENDER LOGIC
//	for (int i{}; i < veritces_ndc.size(); i += 3)
//	{
//		for (int px{}; px < m_Width; ++px)
//		{
//			for (int py{}; py < m_Height; ++py)
//			{
//				ColorRGB finalColor{};
//				Vector2 currentPixel = { float(px), float(py) };
//
//				bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i);
//
//				if (isIntTriangleBounds)
//				{
//					finalColor = { 1, 1, 1 };
//
//					//Update Color in Buffer
//					finalColor.MaxToOne();
//
//					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//						static_cast<uint8_t>(finalColor.r * 255),
//						static_cast<uint8_t>(finalColor.g * 255),
//						static_cast<uint8_t>(finalColor.b * 255));
//				}
//
//			}
//		}
//	}
//
//}
//
//void dae::Renderer::render_W1_Part2()
//{
//	std::vector<Vertex>veritces_world = 
//	{
//		// Triangle 0
//	   {{0.f, 2.f, 0.f}},
//	   {{1.5f, -1.f, 0.f}},
//	   {{-1.5f, -1.f, 0.f}}
//	};
//
//	//vertices_out
//	std::vector<Vertex> veritces_ndc{};
//
//	m_Camera.CalculateViewMatrix();
//	VertexTransformationFunction(veritces_world, veritces_ndc);
//	
//	for (int i{}; i < veritces_ndc.size(); i +=3 )
//	{
//		//RENDER LOGIC
//		for (int px{}; px < m_Width; ++px)
//		{
//			for (int py{}; py < m_Height; ++py)
//			{
//				ColorRGB finalColor{};
//				Vector2 currentPixel = { float(px), float(py) };
//
//				bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i );
//
//				if (isIntTriangleBounds)
//				{
//
//					finalColor = {1, 1, 1 };
//				
//					//Update Color in Buffer
//					finalColor.MaxToOne();
//
//					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//						static_cast<uint8_t>(finalColor.r * 255),
//						static_cast<uint8_t>(finalColor.g * 255),
//						static_cast<uint8_t>(finalColor.b * 255));
//				}
//			}
//		}
//	}
//}
//
//void dae::Renderer::render_W1_Part3()
//{
//	//triangle worldSpace
//	std::vector<Vertex>veritces_world = 
//	{
//		// Triangle 0
//	   {{0.f, 2.f, 0.f}, {1, 0, 0}},
//	   {{1.5f, -1.f, 0.f}, {0, 1, 0}},
//	   {{-1.5f, -1.f, 0.f}, {0, 0, 1}}
//	};
//
//	//vertices_out
//	std::vector<Vertex> veritces_ndc{};
//
//	m_Camera.CalculateViewMatrix();
//	VertexTransformationFunction(veritces_world, veritces_ndc);
//	
//	for (int i{}; i < veritces_ndc.size(); i +=3 )
//	{
//		//cach vertices
//		Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
//		Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
//		Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };
//
//		//RENDER LOGIC
//		for (int px{}; px < m_Width; ++px)
//		{
//			for (int py{}; py < m_Height; ++py)
//			{
//				ColorRGB finalColor{};
//				Vector2 currentPixel = { float(px), float(py) };
//
//				
//					float weight0, weight1, weight2;
//
//					weight0 = Vector2::Cross((currentPixel - v1), v2 - v1) / 2.f;
//					weight1 = Vector2::Cross((currentPixel - v2), v0 - v2) / 2.f;
//					weight2 = Vector2::Cross((currentPixel - v0), v1 - v0) / 2.f;
//					
//					float totalTriangleWeight{weight0 + weight1 + weight2};
//					float invTotalTriangleWeight{ 1 / totalTriangleWeight };
//
//					weight0 *= invTotalTriangleWeight;
//					weight1 *= invTotalTriangleWeight;
//					weight2 *= invTotalTriangleWeight;
//
//					bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i);
//
//				if (isIntTriangleBounds)
//				{
//					//calculate finalColor
//					finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i+ 2].color };
//					
//					//Update Color in Buffer
//					finalColor.MaxToOne();
//
//					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//						static_cast<uint8_t>(finalColor.r * 255),
//						static_cast<uint8_t>(finalColor.g * 255),
//						static_cast<uint8_t>(finalColor.b * 255));
//				}
//			}
//		}
//	}
//
//}
//
//void dae::Renderer::render_W1_Part4()
//{
//	//triangle worldSpace
//	std::vector<Vertex>veritces_world = 
//	{
//		// Triangle 0
//	   {{0.f, 2.f, 0.f}, {1, 0, 0}},
//	   {{1.5f, -1.f, 0.f}, {1, 0, 0}},
//	   {{-1.5f, -1.f, 0.f}, {1, 0, 0}},
//
//	   // Triangle 1
//	   {{0.f, 4.f, 2.f}, {1, 0, 0}},
//	   {{3.f, -2.f, 2.f}, {0, 1, 0}},
//	   {{-3.f, -2.f, 2.f}, {0, 0, 1}}
//	};
//
//	//vertices_out
//	std::vector<Vertex> veritces_ndc{};
//
//	m_Camera.CalculateViewMatrix();
//	VertexTransformationFunction(veritces_world, veritces_ndc);
//
//	for (int i{}; i < veritces_world.size(); i +=3 )
//	{
//		//cach vertices
//		const Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
//		const Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
//		const Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };
//	 
//		//RENDER LOGIC
//		for (int px{}; px < m_Width; ++px)
//		{
//			for (int py{}; py < m_Height; ++py)
//			{
//				ColorRGB finalColor{};
//				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };
//
//					float weight0, weight1, weight2;
//					float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };
//
//					//calculate weights
//					weight0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
//					weight1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
//					weight2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;
//					
//					float totalTriangleWeight{weight0 + weight1 + weight2};
//					float invTotalTriangleWeight{ 1 / totalTriangleWeight };
//					
//
//					weight0 *= invTotalTriangleWeight;
//					weight1 *= invTotalTriangleWeight;
//					weight2 *= invTotalTriangleWeight;
//
//
//					//Depth
//					Vector3 depth{};
//					depth = weight0 * veritces_ndc[i].position + weight1 * veritces_ndc[i + 1].position + weight2 * veritces_ndc[i + 2].position;
//
//
//					/*std::vector<Vertex> vertices
//					{
//						{Vector3{ veritces_ndc[i].position.x, veritces_ndc[i].position.y,  veritces_ndc[i].position.z  }},
//						{Vector3{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y,  veritces_ndc[i + 1].position.z  }},
//						{Vector3{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y,  veritces_ndc[i + 2].position.z  }},
//					};*/
//					bool isInTriangleBounds = IsInTriangle(veritces_ndc, Vector2(depth.x ,depth.y), i);
//					//bool isPointInTriangle{ PointInTriangle(veritces_ndc, depth) };
//
//					if (isInTriangleBounds)
//					{
//						
//						if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
//						{
//
//
//							finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i + 2].color };
//							m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;
//
//							//Update Color in Buffer
//							finalColor.MaxToOne();
//
//							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//								static_cast<uint8_t>(finalColor.r * 255),
//								static_cast<uint8_t>(finalColor.g * 255),
//								static_cast<uint8_t>(finalColor.b * 255));
//						}
//					}
//				
//			}
//		}
//	}
//}
//
//void dae::Renderer::render_W1_Part5()
//{
//	//triangle worldSpace
//	std::vector<Vertex>veritces_world =
//	{
//		// Triangle 0
//	   {{0.f, 2.f, 0.f}, {1, 0, 0}},
//	   {{1.5f, -1.f, 0.f}, {1, 0, 0}},
//	   {{-1.5f, -1.f, 0.f}, {1, 0, 0}},
//
//	   // Triangle 1
//	   {{0.f, 4.f, 2.f}, {1, 0, 0}},
//	   {{3.f, -2.f, 2.f}, {0, 1, 0}},
//	   {{-3.f, -2.f, 2.f}, {0, 0, 1}}
//	};
//
//	//vertices_out
//	std::vector<Vertex> veritces_ndc{};
//
//	m_Camera.CalculateViewMatrix();
//	VertexTransformationFunction(veritces_world, veritces_ndc);
//
//	for (int i{}; i < veritces_world.size(); i += 3)
//	{
//		//cach vertices
//		const Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
//		const Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
//		const Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };
//
//		std::vector<Vector2> vertices;
//		vertices.push_back(v0);
//		vertices.push_back(v1);
//		vertices.push_back(v2);
//
//		Vector2 topLeft{};
//		Vector2 bottomRight{};
//
//		BoundingBox(topLeft, bottomRight, vertices);
//
//		//RENDER LOGIC
//		for (int px{int(topLeft.x)}; px < int(bottomRight.x); ++px)
//		{
//			for (int py{ int(topLeft.y )}; py < int(bottomRight.y); ++py)
//			{
//				ColorRGB finalColor{};
//				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };
//
//				float weight0, weight1, weight2;
//				float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };
//
//				//calculate weights
//				weight0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
//				weight1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
//				weight2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;
//
//				float totalTriangleWeight{ weight0 + weight1 + weight2 };
//				float invTotalTriangleWeight{ 1 / totalTriangleWeight };
//
//
//				weight0 *= invTotalTriangleWeight;
//				weight1 *= invTotalTriangleWeight;
//				weight2 *= invTotalTriangleWeight;
//
//
//				//Depth
//				Vector3 depth{};
//				depth = weight0 * veritces_ndc[i].position + weight1 * veritces_ndc[i + 1].position + weight2 * veritces_ndc[i + 2].position;
//
//				bool isInTriangleBounds = IsInTriangle(veritces_ndc, Vector2(depth.x, depth.y), i);
//				
//
//				if (isInTriangleBounds)
//				{
//
//					if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
//					{
//
//
//						finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i + 2].color };
//						m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;
//
//						//Update Color in Buffer
//						finalColor.MaxToOne();
//
//						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
//							static_cast<uint8_t>(finalColor.r * 255),
//							static_cast<uint8_t>(finalColor.g * 255),
//							static_cast<uint8_t>(finalColor.b * 255));
//					}
//				}
//
//			}
//		}
//	}
//}

void dae::Renderer::render_W2_Part1()
{
	//triangle worldSpace
	std::vector<Mesh> Meshes =
	{
		Mesh
		{
			{
					{{ -3.0f,  3.0f, -2.0f},{1, 1, 1}},
					{{  0.0f,  3.0f, -2.0f},{1, 1, 1}},
					{{  3.0f,  3.0f, -2.0f},{1, 1, 1}},
					{{ -3.0f,  0.0f, -2.0f},{1, 1, 1}},
					{{  0.0f,  0.0f, -2.0f},{1, 1, 1}},
					{{  3.0f,  0.0f, -2.0f},{1, 1, 1}},
					{{ -3.0f, -3.0f, -2.0f},{1, 1, 1}},
					{{  0.0f, -3.0f, -2.0f},{1, 1, 1}},
					{{  3.0f, -3.0f, -2.0f},{1, 1, 1}},
			},
			{
				3,0,4,
				0,1,4,
				4,1,5,
				1,2,5,
				6,3,7,
				3,4,7,
				7,4,8,
				4,5,8
			},
		//PrimitiveTopology::TriangleStrip
		}
	};
	//vertices_in
	std::vector<Vertex> veritces_world{};
	for (int i{}; i < Meshes[0].indices.size(); ++i)
	{
		veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i]]);
	}


	//vertices_out
	std::vector<Vertex> veritces_out{};

	m_Camera.CalculateViewMatrix();
	VertexTransformationFunction(veritces_world, veritces_out);

	for (int i{}; i < Meshes[0].indices.size(); i += 3)
	{
		uint32_t indc0{ Meshes[0].indices[i] };
		uint32_t indc1{ Meshes[0].indices[i + 1] };
		uint32_t indc2{ Meshes[0].indices[i + 2] };
		//cach vertices
		const Vector2 v0{ veritces_out[indc0].position.x, veritces_out[indc0].position.y };
		const Vector2 v1{ veritces_out[indc1].position.x, veritces_out[indc1].position.y };
		const Vector2 v2{ veritces_out[indc2].position.x, veritces_out[indc2].position.y };

		std::vector<Vector2> vertices{};
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);

		Vector2 topLeft{};
		Vector2 bottomRight{};

		BoundingBox(topLeft, bottomRight, vertices);

		//RENDER LOGIC
		for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
		{
			for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
			{
				ColorRGB finalColor{};
				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				float weight0, weight1, weight2;
				float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };

				//calculate weights
				weight0 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;
				weight1 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
				weight2 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;

				float totalTriangleWeight{ weight0 + weight1 + weight2 };
				float invTotalTriangleWeight{ 1 / totalTriangleWeight };


				weight0 *= invTotalTriangleWeight;
				weight1 *= invTotalTriangleWeight;
				weight2 *= invTotalTriangleWeight;


				//Depth
				Vector3 depth{};
				depth = weight0 * veritces_out[indc0].position + weight1 * veritces_out[indc1].position + weight2 * veritces_out[indc2].position;

				bool isInTriangleBounds = IsInTriangle(vertices, Vector2(depth.x, depth.y));


				if (isInTriangleBounds)
				{
					finalColor = { weight0 * veritces_out[indc0].color + weight1 * veritces_out[indc1].color + weight2 * veritces_out[indc2].color };
					if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
					{
						m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;
						//Update Color in Buffer
						finalColor.MaxToOne();

						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}

			}
		}
	}
}



bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer::BoundingBox(Vector2& topLeft, Vector2& bottomRight, std::vector<Vector2> v)
{
	//bounding box optimization

	//top left
	//x
	topLeft.x = std::min(v[0].x, std::min(v[1].x, v[2].x));
	if (topLeft.x <= 0) topLeft.x = 0;
	if (topLeft.x >= m_Width) topLeft.x = m_Width - 1;

	//y
	topLeft.y = std::min(v[0].y, std::min(v[1].y, v[2].y));
	if (topLeft.y <= 0) topLeft.y = 0;
	if (topLeft.y >= m_Height) topLeft.y = m_Height - 1;

	//bottomRight
	
	//x
	bottomRight.x = std::max(v[0].x, std::max(v[1].x, v[2].x));
	if (bottomRight.x <= 0) bottomRight.x = 0;
	if (bottomRight.x >= (m_Width - 1)) bottomRight.x = m_Width - 1;

	//y
	bottomRight.y = std::max(v[0].y, std::max(v[1].y, v[2].y));
	if (bottomRight.y <= 0) bottomRight.y = 0;
	if (bottomRight.y >= (m_Height - 1)) bottomRight.y = m_Height - 1;
}

	
		