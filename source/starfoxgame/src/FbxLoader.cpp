#include "FbxLoader.h"
#include "StaticMesh.h"
#include "fbxsdk.h"
#include <cassert>
#include <algorithm>
#include "gs/Image/ImageData.h"
#include "gs/System/IO.h"

namespace
{
	void FbxMatrixToMatrix43(const FbxAMatrix& mFbx, Matrix43& m43)
	{
		m43.m[0][0] = (float32)mFbx.mData[0][0];
		m43.m[0][1] = (float32)mFbx.mData[0][1];
		m43.m[0][2] = (float32)mFbx.mData[0][2];

		m43.m[1][0] = (float32)mFbx.mData[1][0];
		m43.m[1][1] = (float32)mFbx.mData[1][1];
		m43.m[1][2] = (float32)mFbx.mData[1][2];

		m43.m[2][0] = (float32)mFbx.mData[2][0];
		m43.m[2][1] = (float32)mFbx.mData[2][1];
		m43.m[2][2] = (float32)mFbx.mData[2][2];

		m43.m[3][0] = (float32)mFbx.mData[3][0];
		m43.m[3][1] = (float32)mFbx.mData[3][1];
		m43.m[3][2] = (float32)mFbx.mData[3][2];
	}

	void Matrix43ToFbxMatrix(const Matrix43& m43, FbxAMatrix& mFbx)
	{
		mFbx.mData[0][0] = m43.m[0][0];
		mFbx.mData[0][1] = m43.m[0][1];
		mFbx.mData[0][2] = m43.m[0][2];
		mFbx.mData[0][3] = 0.f;

		mFbx.mData[1][0] = m43.m[1][0];
		mFbx.mData[1][1] = m43.m[1][1];
		mFbx.mData[1][2] = m43.m[1][2];
		mFbx.mData[1][3] = 0.f;

		mFbx.mData[2][0] = m43.m[2][0];
		mFbx.mData[2][1] = m43.m[2][1];
		mFbx.mData[2][2] = m43.m[2][2];
		mFbx.mData[2][3] = 0.f;

		mFbx.mData[3][0] = m43.m[3][0];
		mFbx.mData[3][1] = m43.m[3][1];
		mFbx.mData[3][2] = m43.m[3][2];
		mFbx.mData[3][3] = 1.f;
	}

	FbxScene* LoadScene(const char* pFileName, FbxManager* pManager)
	{
		assert(pManager);

		// Create an FBX scene. This object holds most objects imported/exported from/to files.
		FbxScene* pScene = FbxScene::Create(pManager, "My Scene");
		if (!pScene)
			throw std::exception("FbxScene::Create failed");

		FbxImporter* pImporter = FbxImporter::Create(pManager,"");
		if ( !pImporter->Initialize(pFileName, -1, pManager->GetIOSettings()) )
			throw std::exception("FbxImporter::Initialize failed");

		assert(pImporter->IsFBX());

		bool bImportedScene = pImporter->Import(pScene);
		pImporter->Destroy();

		if (!bImportedScene)
			throw std::exception("FbxImporter::Import failed");

		return pScene;
	}

	void LoadMaterialsFromMesh(FbxMesh* pMesh, gfx::StaticMesh& staticMesh, gfx::StaticMesh::SubMesh& currSubMesh)
	{
		int numMaterials = pMesh->GetElementMaterialCount();
		if (numMaterials == 0)
			return;
		
		assert(numMaterials == 1 && "Only 1 material per submesh supported");
		
		// This loop does nothing but verify preconditions
		for (int i = 0; i < pMesh->GetElementMaterialCount(); ++i)
		{
			if (FbxGeometryElementMaterial* pElementMat = pMesh->GetElementMaterial(i))
			{
				assert(pElementMat->GetMappingMode() == FbxLayerElement::EMappingMode::eAllSame && "Materials must be mapped to entire surface of submesh");
				assert(pElementMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect && "Material index mode expected");
			}
		}

		FbxNode* pNode = pMesh->GetNode();
		assert(pNode);
		assert(pNode->GetMaterialCount() == 1 && "Only 1 material per submesh supported");
		for (int i = 0; i < pMesh->GetElementMaterialCount(); ++i)
		{
			FbxSurfaceMaterial* pMaterial = pNode->GetMaterial(i);
			
			// See if we've already loaded this material
			auto iter = std::find_if(staticMesh.m_materials.begin(), staticMesh.m_materials.end(), [&](const gfx::Material& mat) { return mat.m_uniqueId == pMaterial->GetUniqueID(); });
			if (iter == staticMesh.m_materials.end())
			{
				// Add entry for new material and populate it
				iter = staticMesh.m_materials.emplace(iter);

				gfx::Material& material = *iter;

				material.m_uniqueId = pMaterial->GetUniqueID();
				material.m_name = pMaterial->GetName();

				const FbxImplementation* pImplementation = GetImplementation(pMaterial, FBXSDK_IMPLEMENTATION_HLSL);
				UNUSED_VAR(pImplementation);
				assert(!pImplementation && "Material: no hardware shader support");				
				assert(pMaterial->GetClassId().Is(FbxSurfacePhong::ClassId) && "Material: must use Phong shading");

				FbxSurfacePhong* pPhongMaterial = static_cast<FbxSurfacePhong*>(pMaterial);
			
				// Store ambient, diffuse, specular, emissive, transparency factor, shininess, etc.
				auto ToMaterialColor = [] (const FbxPropertyT<FbxDouble3>& fbxDbl3, const FbxPropertyT<FbxDouble>& fbxDbl1, bool bApplyAlphaToRGB)
				{
					const float32 xyzScale = bApplyAlphaToRGB? (float32)fbxDbl1.Get() : 1.f;
					const float32 alpha = bApplyAlphaToRGB? 0.f : (float32)fbxDbl1.Get();
					return gfx::Color4((float32)fbxDbl3.Get()[0] * xyzScale, (float32)fbxDbl3.Get()[1] * xyzScale, (float32)fbxDbl3.Get()[2] * xyzScale, alpha);
				};

				// GL fixed pipeline lighting model only takes diffuse alpha into account, so we fold the alphas for the other
				// parameters directly into their rgb values
				//@TODO: Make this an option?
				material.m_ambient = ToMaterialColor(pPhongMaterial->Ambient, pPhongMaterial->AmbientFactor, true);
				material.m_diffuse = ToMaterialColor(pPhongMaterial->Diffuse, pPhongMaterial->DiffuseFactor, false);
				material.m_specular = ToMaterialColor(pPhongMaterial->Specular, pPhongMaterial->SpecularFactor, true);
				material.m_emmissive = ToMaterialColor(pPhongMaterial->Emissive, pPhongMaterial->EmissiveFactor, true);
				material.m_shininess = (float32)pPhongMaterial->Shininess.Get();

				// Load texture for material from diffuse channel
				{
					const int diffuseChannelIndex = FbxLayerElement::eTextureDiffuse - FbxLayerElement::sTypeTextureStartIndex;
					FbxProperty lProperty = pMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[diffuseChannelIndex]);
					if (!lProperty.IsValid())
						continue;

					int numTextures = lProperty.GetSrcObjectCount<FbxTexture>();
					if (numTextures > 0)
					{
						assert(numTextures == 1 && "Material: expecting one texture per submesh");
						assert(lProperty.GetSrcObject<FbxLayeredTexture>() == nullptr && "Material: no layered texture support");

						FbxTexture* pTexture = lProperty.GetSrcObject<FbxTexture>();
						assert(pTexture);

						FbxFileTexture* pFileTexture = FbxCast<FbxFileTexture>(pTexture);
						assert(pFileTexture && "Material: texture must be a file");

						material.m_filename = pFileTexture->GetFileName();

						//@TODO: Should we load textures externally? TextureManager?
						std::string path = IO::Path::ChangeExtension(IO::Path::GetFileName(material.m_filename), "tga");
						path = IO::Path::Combine("data", path); //@HACK! At least use directory of input fbx

						ImageData imgData = ImageData::Load(path);
						material.m_textureId = GLUtil::LoadTexture(imgData);
					}
				}
			}

			// Set index to material
			currSubMesh.m_materialIndex = iter - staticMesh.m_materials.begin();
		}
	}

	FbxAMatrix GetMaxToGameTransform()
	{
		Matrix43 mMaxToGame;
		mMaxToGame.SetIdentity();
		float32 scale = 1.f;
		mMaxToGame.axisX = -Vector3::UnitX() * scale;
		mMaxToGame.axisY = -Vector3::UnitZ() * scale;
		mMaxToGame.axisZ = Vector3::UnitY() * scale;
		FbxAMatrix maxToGameTransform;
		Matrix43ToFbxMatrix(mMaxToGame, maxToGameTransform);
		return maxToGameTransform;
	}

	void LoadStaticMeshFromNode(FbxNode* pNode, gfx::StaticMesh& staticMesh)
	{
		const FbxNodeAttribute* pNodeAttribute = pNode->GetNodeAttribute();

		// Ignore nodes with no attributes - usually useless objects in the scene
		if (pNodeAttribute == nullptr)
			return;

		const FbxNodeAttribute::EType attributeType = pNodeAttribute->GetAttributeType();

		//@TODO: Pass this in?
		FbxAMatrix maxToGameTransform = GetMaxToGameTransform();

		// Grab the node global transform, which may have been modified if scene axis + units were changed.
		// We make sure to transform vertex positions and normals by it before storing the values.
		auto pScene = pNode->GetScene();
		const FbxAMatrix nodeGlobalTransform = maxToGameTransform * pScene->GetAnimationEvaluator()->GetNodeGlobalTransform(pNode);

		// Ignore null/dummy objects, which are often used to form the root of a set of meshes
		if (attributeType == FbxNodeAttribute::eNull)
		{
			std::string nodeName = pNode->GetName();
			if (nodeName.find("socket") == 0)
			{
				staticMesh.m_sockets.emplace_back();
				gfx::StaticMesh::Socket& socket = staticMesh.m_sockets.back();
				socket.m_name = nodeName;
					
				//@NOTE: After transforming from "max" to "game" space, the basis vectors will be
				// oriented as in max; example: identity matrix will become "-y forward, x left, z up".
				// This is exactly what we want for transforming all vectors in the mesh (translation,
				// vertex positions and normals), but we would like the basis vectors to be oriented
				// such that the 'forward' in max is the 'forward' in our game, etc. So we 'undo' the
				// transformation on the basis vectors.
				auto socketTransform = nodeGlobalTransform * maxToGameTransform.Inverse();
				FbxMatrixToMatrix43(socketTransform, socket.m_matrix);
			}

			return;
		}

		assert( attributeType == FbxNodeAttribute::eMesh );

		FbxMesh* pMesh = static_cast<FbxMesh*>( pNode->GetNodeAttribute() );

		const int numPolygons = pMesh->GetPolygonCount();
		FbxVector4* pControlPoints = pMesh->GetControlPoints();
		int vertexId = 0;

		staticMesh.m_subMeshes.emplace_back();
		gfx::StaticMesh::SubMesh& currSubMesh = staticMesh.m_subMeshes.back();

		// Load material info -- we assume materials mapped to entire sub mesh
		LoadMaterialsFromMesh(pMesh, staticMesh, currSubMesh);

		for (int i = 0; i < numPolygons; ++i)
		{
			const int polygonSize = pMesh->GetPolygonSize(i);
			assert(polygonSize == 3); // We expect triangles

			for (int j = 0; j < polygonSize; ++j)
			{
				assert(i * 3 + j == vertexId);

				// We keep adding vertices to the current submesh
				gfx::StaticMesh::Vertex& currVertex = *currSubMesh.m_vertices.emplace(currSubMesh.m_vertices.end());
				
				// Position
				const int controlPointIndex = pMesh->GetPolygonVertex(i, j);
				FbxVector4 position = pControlPoints[controlPointIndex];
				position = nodeGlobalTransform.MultT(position);

				currVertex.position.x = (float32)position[0];
				currVertex.position.y = (float32)position[1];
				currVertex.position.z = (float32)position[2];

				staticMesh.m_boundingBox.Include(Vector3(currVertex.position.x, currVertex.position.y, currVertex.position.z));

				// Color
				for (int l = 0; l < pMesh->GetElementVertexColorCount(); ++l)
				{
					FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor(l);
					assert(leVtxc->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);
				
					FbxColor color;

					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						color = leVtxc->GetDirectArray().GetAt(vertexId);
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							int id = leVtxc->GetIndexArray().GetAt(vertexId);
							color = leVtxc->GetDirectArray().GetAt(id);
						}
						break;
					default:
						assert(false);
					}

					currVertex.color.x = (float32)color.mRed;
					currVertex.color.y = (float32)color.mGreen;
					currVertex.color.z = (float32)color.mBlue;
					currVertex.color.w = (float32)color.mAlpha;
				}

				// UV
				for (int l = 0; l < pMesh->GetElementUVCount(); ++l)
				{
					FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
					assert(leUV->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);
					
					FbxVector2 uv;

					int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					case FbxGeometryElement::eIndexToDirect:
						{
							uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
						}
						break;
					default:
						assert(false);
					}

					currVertex.textureCoords.x = (float32)uv[0];
					currVertex.textureCoords.y = (float32)uv[1];
				}

				// Normals
				for(int l = 0; l < pMesh->GetElementNormalCount(); ++l)
				{
					FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
					assert(leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex);

					FbxVector4 normal;
					switch (leNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						normal = leNormal->GetDirectArray().GetAt(vertexId);
						break;
					case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							normal = leNormal->GetDirectArray().GetAt(id);
						}
						break;
					default:
						assert(false);
					}

					//@TODO: Apparently we should be multiplying by the inverse of the transpose?
					auto nodeGlobalTransformNoT = nodeGlobalTransform;
					nodeGlobalTransformNoT.SetT(FbxVector4(0,0,0,1));
					normal = nodeGlobalTransformNoT.MultT(normal);
					normal.Normalize(); // Ignore scale in transform

					currVertex.normal.x = (float32)normal[0];
					currVertex.normal.y = (float32)normal[1];
					currVertex.normal.z = (float32)normal[2];
				}

				//@TEST: randomize vertex colors
				//currVertex.color.r = MathEx::Rand(0.f, 1.f);
				//currVertex.color.g = MathEx::Rand(0.f, 1.f);
				//currVertex.color.b = MathEx::Rand(0.f, 1.f);
				//currVertex.color.a = 1.f;

				++vertexId;
			}
		}
	}

	void RecursiveLoadStaticMesh(FbxNode* pNode, gfx::StaticMesh& staticMesh)
	{
		LoadStaticMeshFromNode(pNode, staticMesh);

		for (int i = 0; i < pNode->GetChildCount(); ++i)
		{
			RecursiveLoadStaticMesh(pNode->GetChild(i), staticMesh);
		}
	}

	void ConvertSceneAxisSystemAndUnits(FbxScene* pScene, const FbxAxisSystem& axisSystem, const FbxSystemUnit& systemUnit)
	{
		const FbxAxisSystem& sceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();
		const FbxSystemUnit& sceneSystemUnit = pScene->GetGlobalSettings().GetSystemUnit();

		// NOTE: You must set the System Units BEFORE the System Axis
		if (sceneSystemUnit != systemUnit)
		{
			systemUnit.ConvertScene(pScene);
		}

		if (sceneAxisSystem != axisSystem)
		{
			axisSystem.ConvertScene(pScene);
		}
	}
} // anonymous namespace

struct FbxLoader::PIMPL
{
	PIMPL()
		: m_pManager(nullptr)
	{
	}

	FbxManager* m_pManager;
};

FbxLoader::FbxLoader()
	: m_pPimpl(new PIMPL)
{
}

void FbxLoader::Init()
{
	auto& pManager = m_pPimpl->m_pManager;

	// The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if (!pManager)
		throw std::exception("FbxManager::Create() failed");

	// Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);
}

void FbxLoader::Shutdown()
{
	if (auto& pManager = m_pPimpl->m_pManager)
	{
		pManager->Destroy();
		pManager = nullptr;
	}
}

std::shared_ptr<gfx::StaticMesh> FbxLoader::LoadStaticMesh(const char* pFileName)
{
	FbxScene* pScene = LoadScene(pFileName, m_pPimpl->m_pManager);

	// Note: doing this just converts the node global and local transforms, but not the mesh vertices.
	// As we load the nodes, we transform the vertex positions and normals by the node global transform.
	//const FbxAxisSystem axisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
	//const FbxAxisSystem& axisSystem = FbxAxisSystem::DirectX;
	//const FbxAxisSystem& axisSystem = FbxAxisSystem::OpenGL;
	//ConvertSceneAxisSystemAndUnits(pScene, axisSystem, FbxSystemUnit::m);

	FbxNode* pRootNode = pScene->GetRootNode();
	assert(pRootNode);

	std::shared_ptr<gfx::StaticMesh> pStaticMesh(new gfx::StaticMesh);

	for (int i = 0; i < pRootNode->GetChildCount(); ++i)
	{
		RecursiveLoadStaticMesh(pRootNode->GetChild(i), *pStaticMesh);
	}

	pScene->Destroy();
	return pStaticMesh;
}
