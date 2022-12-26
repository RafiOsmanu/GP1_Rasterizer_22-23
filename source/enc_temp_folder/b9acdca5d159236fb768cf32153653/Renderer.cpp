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
	m_AspectRatio = float(m_Width) / float(m_Height);
	m_IsMeshLoadedIn = false;

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f, 5.f,-30.f }, m_AspectRatio);

	m_CurrentRenderState = RenderState::combined;

	//loadTexture
	m_pDiffuseTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");

	m_pNormalTexture = Texture::LoadFromFile("Resources/vehicle_normal.png");

	m_pGlossTexture = Texture::LoadFromFile("Resources/vehicle_gloss.png");

	m_pSpecularTexture = Texture::LoadFromFile("Resources/vehicle_specular.png");

	Utils::ParseOBJ("Resources/vehicle.obj", m_Meshes[0].vertices, m_Meshes[0].indices);
	


};

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);

	if(m_RotationToggle) RotateMesh(pTimer);
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
	//render_W1_Part5(); //optimization
	//render_W2_Part1(); //Quads
	//render_W3_Part1(); //matrix
	//render_W3_Part2(); //tuc tuc
	render_W4_Part1();  //PixelShading



	//@END
	
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(Mesh& mesh) const
{
	//Todo > W1 Projection Stage
	Matrix worldViewProjectionMatrix;
	Matrix WorldMatrix{ mesh.worldMatrix };
	//vertices_out = vertices_in;
	for (auto& vertexOut : mesh.vertices)
	{
		worldViewProjectionMatrix = mesh.worldMatrix * m_Camera.viewProjectionMatrix;

		//Vector4 newVertex{vertexOut.position.x, vertexOut.position.y, vertexOut.position.z, 1 };
		Vertex_Out newVertexOut
		{
			worldViewProjectionMatrix.TransformPoint({vertexOut.position, 1.f}),
			vertexOut.color,
			vertexOut.uv,
			mesh.worldMatrix.TransformVector(vertexOut.normal).Normalized(),
			mesh.worldMatrix.TransformVector(vertexOut.tangent).Normalized(),
			vertexOut.position
		};

		//perspective divide
		newVertexOut.position.x /= newVertexOut.position.w;
		newVertexOut.position.y /= newVertexOut.position.w;
		newVertexOut.position.z /= newVertexOut.position.w;
		newVertexOut.position.w = newVertexOut.position.w;

		mesh.vertices_out.emplace_back(newVertexOut);

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



void dae::Renderer::render_W1_Part1()
{
	std::vector<Vertex>veritces_ndc =
	{
	   {{0.f, .5f, 1.f}},
	   {{.5f, -.5f, 1.f}},
	   {{-.5f, -.5f, 1.f}},
	};

	for (auto& Vertex : veritces_ndc)
	{
		Vertex.position.x = (Vertex.position.x + 1) / 2 * float(m_Width);
		Vertex.position.y = (1 - Vertex.position.y) / 2 * float(m_Height);
	}

	//RENDER LOGIC
	for (int i{}; i < veritces_ndc.size(); i += 3)
	{
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				ColorRGB finalColor{};
				Vector2 currentPixel = { float(px), float(py) };

				//bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i);

				//if (isIntTriangleBounds)
				{
					finalColor = { 1, 1, 1 };

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

void dae::Renderer::render_W1_Part2()
{
	//std::vector<Vertex>veritces_world = 
	//{
	//	// Triangle 0
	//   {{0.f, 2.f, 0.f}},
	//   {{1.5f, -1.f, 0.f}},
	//   {{-1.5f, -1.f, 0.f}}
	//};

	////vertices_out
	//std::vector<Vertex> veritces_ndc{};

	//m_Camera.CalculateViewMatrix();
	//VertexTransformationFunction(veritces_world, veritces_ndc);
	//
	//for (int i{}; i < veritces_ndc.size(); i +=3 )
	//{
	//	//RENDER LOGIC
	//	for (int px{}; px < m_Width; ++px)
	//	{
	//		for (int py{}; py < m_Height; ++py)
	//		{
	//			ColorRGB finalColor{};
	//			Vector2 currentPixel = { float(px), float(py) };

	//			//bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i );

	//			//if (isIntTriangleBounds)
	//			{

	//				finalColor = {1, 1, 1 };
	//			
	//				//Update Color in Buffer
	//				finalColor.MaxToOne();

	//				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//					static_cast<uint8_t>(finalColor.r * 255),
	//					static_cast<uint8_t>(finalColor.g * 255),
	//					static_cast<uint8_t>(finalColor.b * 255));
	//			}
	//		}
	//	}
	//}
}

void dae::Renderer::render_W1_Part3()
{
	////triangle worldSpace
	//std::vector<Vertex>veritces_world = 
	//{
	//	// Triangle 0
	//   {{0.f, 2.f, 0.f}, {1, 0, 0}},
	//   {{1.5f, -1.f, 0.f}, {0, 1, 0}},
	//   {{-1.5f, -1.f, 0.f}, {0, 0, 1}}
	//};

	////vertices_out
	//std::vector<Vertex> veritces_ndc{};

	//m_Camera.CalculateViewMatrix();
	//VertexTransformationFunction(veritces_world, veritces_ndc);
	//
	//for (int i{}; i < veritces_ndc.size(); i +=3 )
	//{
	//	//cach vertices
	//	Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
	//	Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
	//	Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };

	//	//RENDER LOGIC
	//	for (int px{}; px < m_Width; ++px)
	//	{
	//		for (int py{}; py < m_Height; ++py)
	//		{
	//			ColorRGB finalColor{};
	//			Vector2 currentPixel = { float(px), float(py) };

	//			
	//				float weight0, weight1, weight2;

	//				weight0 = Vector2::Cross((currentPixel - v1), v2 - v1) / 2.f;
	//				weight1 = Vector2::Cross((currentPixel - v2), v0 - v2) / 2.f;
	//				weight2 = Vector2::Cross((currentPixel - v0), v1 - v0) / 2.f;
	//				
	//				float totalTriangleWeight{weight0 + weight1 + weight2};
	//				float invTotalTriangleWeight{ 1 / totalTriangleWeight };

	//				weight0 *= invTotalTriangleWeight;
	//				weight1 *= invTotalTriangleWeight;
	//				weight2 *= invTotalTriangleWeight;

	//				//bool isIntTriangleBounds = IsInTriangle(veritces_ndc, currentPixel, i);

	//			//if (isIntTriangleBounds)
	//			{
	//				//calculate finalColor
	//				finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i+ 2].color };
	//				
	//				//Update Color in Buffer
	//				finalColor.MaxToOne();

	//				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//					static_cast<uint8_t>(finalColor.r * 255),
	//					static_cast<uint8_t>(finalColor.g * 255),
	//					static_cast<uint8_t>(finalColor.b * 255));
	//			}
	//		}
	//	}
	//}

}

void dae::Renderer::render_W1_Part4()
{
	////triangle worldSpace
	//std::vector<Vertex>veritces_world = 
	//{
	//	// Triangle 0
	//   {{0.f, 2.f, 0.f}, {1, 0, 0}},
	//   {{1.5f, -1.f, 0.f}, {1, 0, 0}},
	//   {{-1.5f, -1.f, 0.f}, {1, 0, 0}},

	//   // Triangle 1
	//   {{0.f, 4.f, 2.f}, {1, 0, 0}},
	//   {{3.f, -2.f, 2.f}, {0, 1, 0}},
	//   {{-3.f, -2.f, 2.f}, {0, 0, 1}}
	//};

	////vertices_out
	//std::vector<Vertex> veritces_ndc{};

	//m_Camera.CalculateViewMatrix();
	//VertexTransformationFunction(veritces_world, veritces_ndc);

	//for (int i{}; i < veritces_world.size(); i +=3 )
	//{
	//	//cach vertices
	//	const Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
	//	const Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
	//	const Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };
	// 
	//	//RENDER LOGIC
	//	for (int px{}; px < m_Width; ++px)
	//	{
	//		for (int py{}; py < m_Height; ++py)
	//		{
	//			ColorRGB finalColor{};
	//			Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

	//				float weight0, weight1, weight2;
	//				float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };

	//				//calculate weights
	//				weight0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
	//				weight1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
	//				weight2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;
	//				
	//				float totalTriangleWeight{weight0 + weight1 + weight2};
	//				float invTotalTriangleWeight{ 1 / totalTriangleWeight };
	//				

	//				weight0 *= invTotalTriangleWeight;
	//				weight1 *= invTotalTriangleWeight;
	//				weight2 *= invTotalTriangleWeight;


	//				//Depth
	//				Vector3 depth{};
	//				depth = weight0 * veritces_ndc[i].position + weight1 * veritces_ndc[i + 1].position + weight2 * veritces_ndc[i + 2].position;


	//				/*std::vector<Vertex> vertices
	//				{
	//					{Vector3{ veritces_ndc[i].position.x, veritces_ndc[i].position.y,  veritces_ndc[i].position.z  }},
	//					{Vector3{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y,  veritces_ndc[i + 1].position.z  }},
	//					{Vector3{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y,  veritces_ndc[i + 2].position.z  }},
	//				};*/
	//				//bool isInTriangleBounds = IsInTriangle(veritces_ndc, Vector2(depth.x ,depth.y), i);
	//				//bool isPointInTriangle{ PointInTriangle(veritces_ndc, depth) };

	//				//if (isInTriangleBounds)
	//				{
	//					
	//					if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
	//					{


	//						finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i + 2].color };
	//						m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;

	//						//Update Color in Buffer
	//						finalColor.MaxToOne();

	//						m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//							static_cast<uint8_t>(finalColor.r * 255),
	//							static_cast<uint8_t>(finalColor.g * 255),
	//							static_cast<uint8_t>(finalColor.b * 255));
	//					}
	//				}
	//			
	//		}
	//	}
	//}
}

void dae::Renderer::render_W1_Part5()
{
	////triangle worldSpace
	//std::vector<Vertex>veritces_world =
	//{
	//	// Triangle 0
	//   {{0.f, 2.f, 0.f}, {1, 0, 0}},
	//   {{1.5f, -1.f, 0.f}, {1, 0, 0}},
	//   {{-1.5f, -1.f, 0.f}, {1, 0, 0}},

	//   // Triangle 1
	//   {{0.f, 4.f, 2.f}, {1, 0, 0}},
	//   {{3.f, -2.f, 2.f}, {0, 1, 0}},
	//   {{-3.f, -2.f, 2.f}, {0, 0, 1}}
	//};

	////vertices_out
	//std::vector<Vertex> veritces_ndc{};

	//m_Camera.CalculateViewMatrix();
	//VertexTransformationFunction(veritces_world, veritces_ndc);

	//for (int i{}; i < veritces_world.size(); i += 3)
	//{
	//	//cach vertices
	//	const Vector2 v0{ veritces_ndc[i].position.x, veritces_ndc[i].position.y };
	//	const Vector2 v1{ veritces_ndc[i + 1].position.x, veritces_ndc[i + 1].position.y };
	//	const Vector2 v2{ veritces_ndc[i + 2].position.x, veritces_ndc[i + 2].position.y };

	//	std::vector<Vector2> vertices;
	//	vertices.push_back(v0);
	//	vertices.push_back(v1);
	//	vertices.push_back(v2);

	//	Vector2 topLeft{};
	//	Vector2 bottomRight{};

	//	BoundingBox(topLeft, bottomRight, vertices);

	//	//RENDER LOGIC
	//	for (int px{int(topLeft.x)}; px < int(bottomRight.x); ++px)
	//	{
	//		for (int py{ int(topLeft.y )}; py < int(bottomRight.y); ++py)
	//		{
	//			ColorRGB finalColor{};
	//			Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

	//			float weight0, weight1, weight2;
	//			float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };

	//			//calculate weights
	//			weight0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
	//			weight1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
	//			weight2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;

	//			float totalTriangleWeight{ weight0 + weight1 + weight2 };
	//			float invTotalTriangleWeight{ 1 / totalTriangleWeight };


	//			weight0 *= invTotalTriangleWeight;
	//			weight1 *= invTotalTriangleWeight;
	//			weight2 *= invTotalTriangleWeight;


	//			//Depth
	//			Vector3 depth{};
	//			depth = weight0 * veritces_ndc[i].position + weight1 * veritces_ndc[i + 1].position + weight2 * veritces_ndc[i + 2].position;

	//			bool isInTriangleBounds = IsInTriangle(vertices, Vector2(depth.x, depth.y));
	//			

	//			if (isInTriangleBounds)
	//			{

	//				if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
	//				{


	//					finalColor = { weight0 * veritces_ndc[i].color + weight1 * veritces_ndc[i + 1].color + weight2 * veritces_ndc[i + 2].color };
	//					m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;

	//					//Update Color in Buffer
	//					finalColor.MaxToOne();

	//					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//						static_cast<uint8_t>(finalColor.r * 255),
	//						static_cast<uint8_t>(finalColor.g * 255),
	//						static_cast<uint8_t>(finalColor.b * 255));
	//				}
	//			}

	//		}
	//	}
	//}
}

void dae::Renderer::render_W2_Part1()
{
	////loadTexture
	//m_pSurfaceTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");


	////triangle worldSpace
	//std::vector<Mesh> Meshes =
	//{
	//	Mesh
	//	{
	//		{
	//			Vertex{{ -3.0f,  3.0f, -2.0f},{1, 1, 1},{ 0.0f, 0.0f}},
 //               Vertex{{  0.0f,  3.0f, -2.0f},{1, 1, 1},{ 0.5f, 0.0f}},
 //               Vertex{{  3.0f,  3.0f, -2.0f},{1, 1, 1},{ 1.0f, 0.0f}},
 //               Vertex{{ -3.0f,  0.0f, -2.0f},{1, 1, 1},{ 0.0f, 0.5f}},
 //               Vertex{{  0.0f,  0.0f, -2.0f},{1, 1, 1},{ 0.5f, 0.5f}},
 //               Vertex{{  3.0f,  0.0f, -2.0f},{1, 1, 1},{ 1.0f, 0.5f}},
 //               Vertex{{ -3.0f, -3.0f, -2.0f},{1, 1, 1},{ 0.0f, 1.0f}},
 //               Vertex{{  0.0f, -3.0f, -2.0f},{1, 1, 1},{ 0.5f, 1.0f}},
 //               Vertex{{  3.0f, -3.0f, -2.0f},{1, 1, 1},{ 1.0f, 1.0f}},
	//		},
	//		{
	//			3,0,4,1,5,2,2,6,6,3,7,4,8,5

	//			/* 3,0,1,   1,4,3,   4,1,2,
	//			 2,5,4,   6,3,4,   4,7,6,
	//			 7,4,5,   5,8,7,*/
	//		},
	//	PrimitiveTopology::TriangleStrip
	//	}
	//};
	////vertices_in
	//std::vector<Vertex> veritces_world{};

	//switch (Meshes[0].primitiveTopology)
	//{
	//case PrimitiveTopology::TriangleList:

	//	for (int i{}; i < Meshes[0].indices.size(); ++i)
	//	{
	//		veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i]]);
	//	}
	//	break;
	//case PrimitiveTopology::TriangleStrip:

	//	for (int i{}; i < Meshes[0].indices.size() - 2; ++i)
	//	{
	//		veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i]]);
	//		if (i % 2 != 0)
	//		{
	//			veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i + 2]]);
	//			veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i + 1]]);
	//		}
	//		else
	//		{
	//			veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i + 1]]);
	//			veritces_world.push_back(Meshes[0].vertices[Meshes[0].indices[i + 2]]);
	//		}
	//	}
	//	break;
	//}
	//
	//


	////vertices_out
	//std::vector<Vertex> veritces_out{};

	//m_Camera.CalculateViewMatrix();
	//VertexTransformationFunction(veritces_world, veritces_out);

	////int count{};
	//for (int i{}; i < veritces_out.size(); i += 3)
	//{
	//	int indc0{ i };
	//	int indc1{ i + 1 };
	//	int indc2{ i + 2 };
	//	//cach vertices
	//	const Vector2 v0{ veritces_out[indc0].position.x, veritces_out[indc0].position.y };
	//	const Vector2 v1{ veritces_out[indc1].position.x, veritces_out[indc1].position.y };
	//	const Vector2 v2{ veritces_out[indc2].position.x, veritces_out[indc2].position.y };

	//	//vertices 2D
	//	std::vector<Vector2> vertices{};
	//	vertices.push_back(v0);
	//	vertices.push_back(v1);
	//	vertices.push_back(v2);

	//	Vector2 topLeft{};
	//	Vector2 bottomRight{};

	//	BoundingBox(topLeft, bottomRight, vertices);

	//	//RENDER LOGIC
	//	for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
	//	{
	//		for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
	//		{
	//			ColorRGB finalColor{};
	//			Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

	//			float weight0 = 0, weight1 = 0, weight2 = 0;
	//			float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };

	//			//calculate weights
	//			weight0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
	//			weight1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
	//			weight2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;

	//			float totalTriangleWeight{ weight0 + weight1 + weight2 };
	//			float invTotalTriangleWeight{ 1 / totalTriangleWeight };


	//			weight0 *= invTotalTriangleWeight;
	//			weight1 *= invTotalTriangleWeight;
	//			weight2 *= invTotalTriangleWeight;


	//			//interpolatedZ
	//			float interpolatedZ =
	//				1 / ((1 / veritces_out[indc0].position.z) * weight0 +
	//					(1 / veritces_out[indc1].position.z) * weight1 +
	//					(1 / veritces_out[indc2].position.z) * weight2);


	//			//Depth
	//			Vector3 depth{};
	//			depth = weight0 * veritces_out[indc0].position +
	//				weight1 * veritces_out[indc1].position +
	//				weight2 * veritces_out[indc2].position;

	//			bool isInTriangleBounds = IsInTriangle(vertices, Vector2(depth.x, depth.y));

	//			if (isInTriangleBounds)
	//			{
	//				Vector2 interpolatedUv =  
	//					((veritces_out[indc0].uv / veritces_out[indc0].position.z) * weight0  
	//					+ (veritces_out[indc1].uv / veritces_out[indc1].position.z) * weight1
	//					+ (veritces_out[indc2].uv / veritces_out[indc2].position.z) * weight2) * interpolatedZ;

	//				//sample pixel color
	//				ColorRGB sampledColor = m_pSurfaceTexture->Sample(interpolatedUv);

	//				finalColor = { sampledColor };

	//				if (depth.z < m_pDepthBufferPixels[(py * m_Width) + px])
	//				{

	//					m_pDepthBufferPixels[(py * m_Width) + px] = depth.z;

	//					//Update Color in Buffer
	//					finalColor.MaxToOne();

	//					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//						static_cast<uint8_t>(finalColor.r * 255),
	//						static_cast<uint8_t>(finalColor.g * 255),
	//						static_cast<uint8_t>(finalColor.b * 255));
	//				}
	//			}

	//		}
	//	}
	//}
	///*std::cout << count << "\n";*/
}

void dae::Renderer::render_W3_Part1()
{
	//loadTexture
	m_pDiffuseTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

	//triangle worldSpace
	std::vector<Mesh> Meshes =
	{
		Mesh
		{
			{
				Vertex{{ -3.0f,  3.0f, -2.0f},{1, 1, 1},{ 0.0f, 0.0f}},
				Vertex{{  0.0f,  3.0f, -2.0f},{1, 1, 1},{ 0.5f, 0.0f}},
				Vertex{{  3.0f,  3.0f, -2.0f},{1, 1, 1},{ 1.0f, 0.0f}},
				Vertex{{ -3.0f,  0.0f, -2.0f},{1, 1, 1},{ 0.0f, 0.5f}},
				Vertex{{  0.0f,  0.0f, -2.0f},{1, 1, 1},{ 0.5f, 0.5f}},
				Vertex{{  3.0f,  0.0f, -2.0f},{1, 1, 1},{ 1.0f, 0.5f}},
				Vertex{{ -3.0f, -3.0f, -2.0f},{1, 1, 1},{ 0.0f, 1.0f}},
				Vertex{{  0.0f, -3.0f, -2.0f},{1, 1, 1},{ 0.5f, 1.0f}},
				Vertex{{  3.0f, -3.0f, -2.0f},{1, 1, 1},{ 1.0f, 1.0f}},
			},
			{
				3,0,4,1,5,2,2,6,6,3,7,4,8,5

				/* 3,0,1,   1,4,3,   4,1,2,
				 2,5,4,   6,3,4,   4,7,6,
				 7,4,5,   5,8,7,*/
			},
		PrimitiveTopology::TriangleStrip
		}
	};

	//transform vector to display on your screen
	VertexTransformationFunction(Meshes[0]);

	

	int adder{};
	if (Meshes[0].primitiveTopology == PrimitiveTopology::TriangleList)
	{
		adder = 3;
	}
	else
	{
		adder = 1;
	}

	//int count{};
	for (int i{}; i < Meshes[0].indices.size() - 2; i += adder)
	{

		

		uint32_t indc0{ Meshes[0].indices[i]};
		uint32_t indc1{ Meshes[0].indices[i + 1] };
		uint32_t indc2{ Meshes[0].indices[i + 2] };

		if (Meshes[0].primitiveTopology == PrimitiveTopology::TriangleStrip && (i & 2) != 0)
		{
			std::swap(indc1, indc2);
		}
		//cach vertices
		const Vector2 v0{ Meshes[0].vertices_out[indc0].position.x, Meshes[0].vertices_out[indc0].position.y };
		const Vector2 v1{Meshes[0].vertices_out[indc1].position.x, Meshes[0].vertices_out[indc1].position.y };
		const Vector2 v2{Meshes[0].vertices_out[indc2].position.x, Meshes[0].vertices_out[indc2].position.y };

		//vertices 2D
		std::vector<Vector2> vertices{};
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);

		Vector2 topLeft{};
		Vector2 bottomRight{};

		BoundingBox(topLeft, bottomRight, vertices);

		//RENDER LOGIC
		for (int px{ static_cast<int>(topLeft.x) }; px < static_cast<int>(bottomRight.x); ++px)
		{
			for (int py{ static_cast<int>(topLeft.y) }; py < static_cast<int>(bottomRight.y); ++py)
			{
				ColorRGB finalColor{};
				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				float weight0 = 0, weight1 = 0, weight2 = 0;
				float triangleSide0 = 0, triangleSide1 = 0, triangleSide2 = 0;
				float area{ Vector2::Cross((v1 - v0), (v2 - v0)) };

				//calculate weights
				weight0 = triangleSide0 = Vector2::Cross(v2 - v1, (currentPixel - v1)) / area;
				weight1 = triangleSide1 = Vector2::Cross(v0 - v2, (currentPixel - v2)) / area;
				weight2 = triangleSide2 = Vector2::Cross(v1 - v0, (currentPixel - v0)) / area;

				float totalTriangleWeight{ weight0 + weight1 + weight2 };
				float invTotalTriangleWeight{ 1 / totalTriangleWeight };


				weight0 *= invTotalTriangleWeight;
				weight1 *= invTotalTriangleWeight;
				weight2 *= invTotalTriangleWeight;


				//interpolatedZ
				float interpolatedZ =
					1 / ((1 / Meshes[0].vertices_out[indc0].position.z) * weight0 +
						(1 / Meshes[0].vertices_out[indc1].position.z) * weight1 +
						(1 / Meshes[0].vertices_out[indc2].position.z) * weight2);


				//interpolatedW
				float interpolatedW =
					1 / ((1 / Meshes[0].vertices_out[indc0].position.w) * weight0 +
						(1 / Meshes[0].vertices_out[indc1].position.w) * weight1 +
						(1 / Meshes[0].vertices_out[indc2].position.w) * weight2);


				////Depth
				//float depth{};
				//depth = weight0 * Meshes[0].vertices_out[indc0].position.w +
				//	weight1 * Meshes[0].vertices_out[indc1].position.w +
				//	weight2 * Meshes[0].vertices_out[indc2].position.w;



				//isInTriangleCheck
				if (triangleSide0 < 0) continue;
				if (triangleSide1 < 0) continue;
				if (triangleSide2 < 0) continue;

				
				Vector2 interpolatedUv =
					((Meshes[0].vertices_out[indc0].uv / Meshes[0].vertices_out[indc0].position.w) * weight0
						+ (Meshes[0].vertices_out[indc1].uv / Meshes[0].vertices_out[indc1].position.w) * weight1
						+ (Meshes[0].vertices_out[indc2].uv / Meshes[0].vertices_out[indc2].position.w) * weight2) * interpolatedW;


				ColorRGB sampledColor = m_pDiffuseTexture->Sample(interpolatedUv);

				//sample pixel color
				switch (m_ColorOutput)
				{
				case 0:
					finalColor = { sampledColor };
					break;
				case 1:
					finalColor = { interpolatedZ, interpolatedZ, interpolatedZ };
					break;
				}


				if (interpolatedZ < m_pDepthBufferPixels[(py * m_Width) + px])
				{

					m_pDepthBufferPixels[(py * m_Width) + px] = interpolatedZ;

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

void dae::Renderer::render_W3_Part2()
{
	//clear global mesh variable
	m_Meshes[0].vertices_out.clear();

	//transform vector to display on your screen
	VertexTransformationFunction(m_Meshes[0]);

	int adder{};
	if (m_Meshes[0].primitiveTopology == PrimitiveTopology::TriangleList)
	{
		adder = 3;
	}
	else
	{
		adder = 1;
	}

	//int count{};
	for (int i{}; i < m_Meshes[0].indices.size(); i += adder)
	{
		uint32_t indc0{ m_Meshes[0].indices[i] };
		uint32_t indc1{ m_Meshes[0].indices[i + 1] };
		uint32_t indc2{ m_Meshes[0].indices[i + 2] };

		if (m_Meshes[0].primitiveTopology == PrimitiveTopology::TriangleStrip && (i & 2) != 0)
		{
			std::swap(indc1, indc2);
		}

		//cach vertices
		//vertex_out
		const Vertex_Out v0{ m_Meshes[0].vertices_out[indc0]};
		const Vertex_Out v1{ m_Meshes[0].vertices_out[indc1]};
		const Vertex_Out v2{ m_Meshes[0].vertices_out[indc2]};

		//Vector2 for cross
		const Vector2 vec0{v0.position.x, v0.position.y};
		const Vector2 vec1{v1.position.x, v1.position.y};
		const Vector2 vec2{v2.position.x, v2.position.y}; 

		//vertices 2D
		std::vector<Vector2> vertices{};
		vertices.resize(3);
		vertices[0] = vec0;
		vertices[1] = vec1;
		vertices[2] = vec2;
		
		Vector2 topLeft{};
		Vector2 bottomRight{};

		BoundingBox(topLeft, bottomRight, vertices);

		//RENDER LOGIC
		for (int px{ static_cast<int>(topLeft.x) }; px < static_cast<int>(bottomRight.x); ++px)
		{
			for (int py{ static_cast<int>(topLeft.y) }; py < static_cast<int>(bottomRight.y); ++py)
			{
				ColorRGB finalColor{};
				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				float weight0 = 0, weight1 = 0, weight2 = 0;
				float triangleSide0 = 0, triangleSide1 = 0, triangleSide2 = 0;
				
				float area{ Vector2::Cross((vec1 - vec0), (vec2 - vec0)) };

				//calculate weights
				weight0 = triangleSide0 = Vector2::Cross(vec2 - vec1, (currentPixel - vec1)) / area;
				weight1 = triangleSide1 = Vector2::Cross(vec0 - vec2, (currentPixel - vec2)) / area;
				weight2 = triangleSide2 = Vector2::Cross(vec1 - vec0, (currentPixel - vec0)) / area;

				float totalTriangleWeight{ weight0 + weight1 + weight2 };
				float invTotalTriangleWeight{ 1 / totalTriangleWeight };


				weight0 *= invTotalTriangleWeight;
				weight1 *= invTotalTriangleWeight;
				weight2 *= invTotalTriangleWeight;


				//depth
				//interpolatedZ
				const float interpolatedZ =
					1 / ((1 / v0.position.z) * weight0 +
						(1 / v1.position.z) * weight1 +
						(1 / v2.position.z) * weight2);


				//interpolatedW
				const float interpolatedW =
					1 / ((1 / v0.position.w) * weight0 +
						 (1 / v1.position.w) * weight1 +
						 (1 / v2.position.w) * weight2);

				//isInTriangleCheck
				if (triangleSide0 < 0) continue;
				if (triangleSide1 < 0) continue;
				if (triangleSide2 < 0) continue;

				const Vector2 interpolatedUv =
					((v0.uv / v0.position.w) * weight0
						+ (v1.uv / v1.position.w) * weight1
						+ (v2.uv / v2.position.w) * weight2) * interpolatedW;

				//ColorRGB sampledColor = m_pSurfaceTexture->Sample(interpolatedUv);
				//sample pixel color
				switch (m_ColorOutput)
				{
				case 0:
					//finalColor = { sampledColor };
					break;
				case 1:
					finalColor = { interpolatedZ, 0.985f, 1.f };
					break;
				}


				if (interpolatedZ < m_pDepthBufferPixels[(py * m_Width) + px])
				{

					m_pDepthBufferPixels[(py * m_Width) + px] = interpolatedZ;

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

void Renderer::render_W4_Part1()
{
	//clear global mesh variable
	m_Meshes[0].vertices_out.clear();

	VertexTransformationFunction(m_Meshes[0]);

	int adder{};
	if (m_Meshes[0].primitiveTopology == PrimitiveTopology::TriangleList)
	{
		adder = 3;
	}
	else
	{
		adder = 1;
	}

	//int count{};
	for (int i{}; i < m_Meshes[0].vertices_out.size(); i += adder)
	{

		uint32_t indc0{ m_Meshes[0].indices[i] };
		uint32_t indc1{ m_Meshes[0].indices[i + 1] };
		uint32_t indc2{ m_Meshes[0].indices[i + 2] };

		if (m_Meshes[0].primitiveTopology == PrimitiveTopology::TriangleStrip && (i & 2) != 0)
		{
			std::swap(indc1, indc2);
		}

		//cach vertices
		//vertex_out
		Vertex_Out v0{ m_Meshes[0].vertices_out[indc0]};
		Vertex_Out v1{ m_Meshes[0].vertices_out[indc1]};
		Vertex_Out v2{ m_Meshes[0].vertices_out[indc2]};

		//fustrum culling
		if (FustrumCulling(v0.position, v1.position, v2.position)) continue;

		//screenSpace
		ToScreenSpace(v0.position, v1.position, v2.position);
		
		//Vector2 for cross
		const Vector2 vec0{ v0.position.x, v0.position.y };
		const Vector2 vec1{ v1.position.x, v1.position.y };
		const Vector2 vec2{ v2.position.x, v2.position.y };

		//vertices 2D
		std::vector<Vector2> vertices{};
		vertices.resize(3);
		vertices[0] = vec0;
		vertices[1] = vec1;
		vertices[2] = vec2;

		//boundingBox Optimization
		Vector2 topLeft{};
		Vector2 bottomRight{};

		BoundingBox(topLeft, bottomRight, vertices);

		//RENDER LOGIC
		for (int px{ static_cast<int>(topLeft.x) }; px <= static_cast<int>(bottomRight.x); ++px)
		{
			for (int py{ static_cast<int>(topLeft.y) }; py <= static_cast<int>(bottomRight.y); ++py)
			{
				ColorRGB finalColor{};
				Vector2 currentPixel = { static_cast<float>(px) + 0.5f, static_cast<float>(py) + 0.5f };

				float weight0 = 0, weight1 = 0, weight2 = 0;
				float triangleSide0 = 0, triangleSide1 = 0, triangleSide2 = 0;

				float area{ Vector2::Cross((vec1 - vec0), (vec2 - vec0)) };

				//calculate weights
				weight0 = triangleSide0 = Vector2::Cross(vec2 - vec1, (currentPixel - vec1)) / area;
				weight1 = triangleSide1 = Vector2::Cross(vec0 - vec2, (currentPixel - vec2)) / area;
				weight2 = triangleSide2 = Vector2::Cross(vec1 - vec0, (currentPixel - vec0)) / area;

				float totalTriangleWeight{ weight0 + weight1 + weight2 };
				float invTotalTriangleWeight{ 1 / totalTriangleWeight };


				weight0 *= invTotalTriangleWeight;
				weight1 *= invTotalTriangleWeight;
				weight2 *= invTotalTriangleWeight;

				//isInTriangleCheck
				if (triangleSide0 < 0) continue;
				if (triangleSide1 < 0) continue;
				if (triangleSide2 < 0) continue;



				//depth
				//interpolatedZ
				const float interpolatedZ =
					1 / ((1 / v0.position.z) * weight0 +
						(1 / v1.position.z) * weight1 +
						(1 / v2.position.z) * weight2);


				if (interpolatedZ < m_pDepthBufferPixels[(py * m_Width) + px])
				{ 
					//interpolatedW
					const float interpolatedW =
						1 / ((1 / v0.position.w) * weight0 +
							(1 / v1.position.w) * weight1 +
							(1 / v2.position.w) * weight2);



					//Position
					const Vector4 pos = { ((v0.position * weight0) + (v1.position * weight1) + (v2.position * weight2)) * interpolatedW };


					//uv
					const Vector2 interpolatedUv =
						((v0.uv / v0.position.w) * weight0
							+ (v1.uv / v1.position.w) * weight1
							+ (v2.uv / v2.position.w) * weight2) * interpolatedW;

					//normal
					const Vector3 normal{ ((v0.normal / (v0.position.w)) * weight0
						+ (v1.normal / v1.position.w) * weight1
						+ (v2.normal / v2.position.w) * weight2) * interpolatedW };

					//tangent
					const Vector3 tangent = { ((v0.tangent / v0.position.w) * weight0
							+ (v1.tangent / v1.position.w) * weight1
							+ (v2.tangent / v2.position.w) * weight2) * interpolatedW };

					// viewDirection
						const Vector3 viewDirection = { ((v0.viewDirection / v0.position.w) * weight0
								+ (v1.viewDirection / v1.position.w) * weight1
								+ (v2.viewDirection / v2.position.w) * weight2) * interpolatedW };




					ColorRGB sampledColor = m_pDiffuseTexture->Sample(interpolatedUv);
					//sample pixel color
					switch (m_ColorOutput)
					{
					case 0:
						finalColor = { sampledColor };
						break;
					case 1:
						float remapedValue{ Remap(interpolatedZ, 0.985f, 1.f)};
						//std::cout << interpolatedZ << "\n";
						finalColor = { remapedValue, remapedValue, remapedValue };
						break;
					}
				

					m_pDepthBufferPixels[(py * m_Width) + px] = interpolatedZ;

					//pixel shading
					Vertex_Out finalPixel{ pos, finalColor, interpolatedUv, normal, tangent, viewDirection };
					finalColor = PixelShading(finalPixel);

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

float Renderer::Remap(float value, float minValue, float maxValue) 
{
	//map to range[0,1]
	return (value - minValue) / (maxValue - minValue);
}

bool Renderer::FustrumCulling(const Vector3 v0, const Vector3 v1, const Vector3 v2)
{
	if ((v0.x < -1 || v0.x > 1) ||
		(v0.y < -1 || v0.y > 1) ||
		(v0.z < 0 || v0.z > 1) ||
		(v1.x < -1 || v1.x > 1) ||
		(v1.y < -1 || v1.y > 1) ||
		(v1.z < 0 || v1.z > 1) ||
		(v2.x < -1 || v2.x > 1) ||
		(v2.y < -1 || v2.y > 1) ||
		(v2.z < 0 || v2.z > 1))
	{
		return true;
	}
	return false;
}

void Renderer::ToScreenSpace(Vector4& v0, Vector4& v1, Vector4& v2)
{
	//v0
	v0.x = (v0.x + 1) / 2 * float(m_Width);
	v0.y = (1 - v0.y) / 2 * float(m_Height);

	//v1
	v1.x = (v1.x + 1) / 2 * float(m_Width);
	v1.y = (1 - v1.y) / 2 * float(m_Height);

	//v2
	v2.x = (v2.x + 1) / 2 * float(m_Width);
	v2.y = (1- v2.y) / 2 * float(m_Height);
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
	if (topLeft.x >= m_Width) topLeft.x = static_cast<float>(m_Width) - 1;

	//y
	topLeft.y = std::min(v[0].y, std::min(v[1].y, v[2].y));
	if (topLeft.y <= 0) topLeft.y = 0;
	if (topLeft.y >= m_Height) topLeft.y = static_cast<float>(m_Height) - 1;

	//bottomRight
	
	//x
	bottomRight.x = std::max(v[0].x, std::max(v[1].x, v[2].x));
	if (bottomRight.x <= 0) bottomRight.x = 0;
	if (bottomRight.x >= (m_Width - 1)) bottomRight.x = static_cast<float>(m_Width) - 1;

	//y
	bottomRight.y = std::max(v[0].y, std::max(v[1].y, v[2].y));
	if (bottomRight.y <= 0) bottomRight.y = 0;
	if (bottomRight.y >= (m_Height - 1)) bottomRight.y = static_cast<float>(m_Height) - 1;
}

void Renderer::RotateMesh(const Timer* timer)
{
	const auto yawAngle = (PI_DIV_4 * (timer->GetTotal()));

	const Matrix rotMatrix{Matrix::CreateRotationY(yawAngle)};

	m_Meshes[0].worldMatrix = rotMatrix;

}

ColorRGB Renderer::PixelShading(const Vertex_Out& v)
{
	Vector3 lightDirection = { .577f, -.577f, .577f };
	const float lightIntensity{ 7.f };
	ColorRGB ambient{ 0.025f, 0.025f , 0.025f };

	Vector3 sampledNormal{v.normal};
	
	Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
	Matrix tangentSpaceAxis = Matrix{ v.tangent, binormal.Normalized(), v.normal, Vector3::Zero};
	sampledNormal = m_pNormalTexture->SampleNormal(v.uv);
	sampledNormal = 2.f * sampledNormal - Vector3(1.f, 1.f, 1.f);
	sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
	sampledNormal.Normalize();
	
	

	//observedArea
	float observedArea{};
	if (m_NormalMapToggle) observedArea = Vector3::Dot(sampledNormal, -lightDirection);

	else observedArea = Vector3::Dot(v.normal, -lightDirection);

	if (observedArea < 0) observedArea = 0;

	//Diffuse
	const ColorRGB lambertDiffuse{ (1 * v.color) / float(M_PI) };

	//phong 
	const float shininess{ 25.f };
	const ColorRGB specularColor{ m_pSpecularTexture->Sample(v.uv) };
	const float phongExp{ m_pGlossTexture->SampleNormal(v.uv).x * shininess };

	const Vector3 reflect{ Vector3::Reflect(-lightDirection, v.normal) };
	float cosAngle{ Vector3::Dot(reflect, v.viewDirection) };
	if (cosAngle < 0.f) cosAngle = 0.f;

	const float specReflection{ 1 * powf(cosAngle, phongExp) };
	const ColorRGB phong{ specReflection * specularColor };

	switch (m_CurrentRenderState)
	{
	case dae::Renderer::RenderState::observedArea:
		
		return ColorRGB(observedArea, observedArea, observedArea);

	case dae::Renderer::RenderState::lambert:

		return lambertDiffuse * observedArea;

	case dae::Renderer::RenderState::phong:

		return phong * observedArea;

	case dae::Renderer::RenderState::combined:

		return ColorRGB(lightIntensity * lambertDiffuse + ambient + phong) * observedArea ;
	}
	return ColorRGB();
}

//with int
void Renderer::ToggleColorOutput()
{
	m_ColorOutput = (m_ColorOutput + 1) % 2;
}

//with enum
void Renderer::ToggleRenderOutput()
{
	m_CurrentRenderState = RenderState((int(m_CurrentRenderState) + 1) % 4);
}

//with bool
void dae::Renderer::ToggleNormalMap()
{
	if (!m_NormalMapToggle)
	{
		m_NormalMapToggle = true;
	}
	else
	{
		m_NormalMapToggle = false;
	}
}

void dae::Renderer::ToggleRotation()
{
	if (!m_RotationToggle)
	{
		m_RotationToggle = true;
	}
	else
	{
		m_RotationToggle = false;
	}
}




	
		