#include "gs/System/System.h"
#include "gs/Rendering/GraphicsEngine.h"
#include "gs/System/FrameTimer.h"
#include "gs/Platform/GL/GLHeaders.h"
#include "gs/Base/string_helpers.h"
#include "gs/Input/KeyboardMgr.h"

#include "FbxLoader.h"
#include "StaticMesh.h"
#include "DebugDraw.h"
#include "GameInput.h"
#include "CameraComponent.h"
#include "PlayerControlComponent.h"
#include "AnchorComponent.h"
#include "StaticMeshComponent.h"
#include "GroundComponent.h"

//const float32 SCREEN_WIDTH_HEIGHT_RATIO = 4.f / 3.f;
const float32 SCREEN_WIDTH_HEIGHT_RATIO = 16.f / 9.f;
//const float32 SCREEN_WIDTH_HEIGHT_RATIO = 16.f / 10.f;

//const float32 SCREEN_WIDTH = 1680.f;
//const float32 SCREEN_WIDTH = 1280.f;
const float32 SCREEN_WIDTH = 1024.f;
//const float32 SCREEN_WIDTH = 800.f;

const float32 SCREEN_HEIGHT = SCREEN_WIDTH / SCREEN_WIDTH_HEIGHT_RATIO;

bool g_drawNormals = false;
bool g_drawSockets = false;
float32 g_normalScale = 10.0f;
bool g_renderSceneGraph = false;

int main()
{
	extern void UnitTest_Math();
	UnitTest_Math();

	ScreenMode::Type screenMode = ScreenMode::Windowed;
	VertSync::Type vertSync = VertSync::Disable;

	GraphicsEngine& gfxEngine = GraphicsEngine::Instance();
	gfxEngine.Initialize("Star Fox", (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, 32, screenMode, vertSync);

	// By default, when lighting is enabled, material parameters are used for shading, but vertex colors are ignored;
	// while the opposite is true when lighting is disabled.
	// Enabling GL_COLOR_MATERIAL tells OpenGL to substitute the current color for the ambient and for the diffuse material
	// color when doing lighting computations.
	glEnable(GL_COLOR_MATERIAL);

	GLUtil::SetBlending(false);
	GLUtil::SetDepthTesting(true);
	GLUtil::SetFrontFace(Winding::CounterClockwise, CullBackFace::True);
	GLUtil::SetLighting(true);
	GLUtil::SetShadeModel(ShadeModel::Smooth);
	GLUtil::SetTexturing(true);

	ProjectionInfo perspMain;
	perspMain.SetPerspective(45.f, SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 10000.f);
	GLUtil::SetProjection(perspMain);

	GLfloat light_position[] = { 1.0, 1.0, 0.5, 0.0 }; // Directional
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glEnable(GL_LIGHT0);
	
	FrameTimer frameTimer;
	frameTimer.SetMinFPS(10.0f);
	//frameTimer.SetMaxFPS(60.f);

	KeyboardMgr& kbMgr = KeyboardMgr::Instance();

	// Create SceneNodes

	SceneNodeWeakPtr pwCamera;

	{
		FbxLoader fbxLoader;
		fbxLoader.Init();

		auto psShip = SceneNode::Create("Ship");
		auto psStaticMesh = fbxLoader.LoadStaticMesh("data/Arwing_001.fbx");
		psShip->AddComponent<StaticMeshComponent>()->Init(psStaticMesh);
		psShip->AddComponent<PlayerControlComponent>();
		psShip->ModifyLocalToWorld().trans.y = 50.f; // The ship starts above the ground

		auto psGround = SceneNode::Create("Ground");
		psGround->AddComponent<GroundComponent>();		
		
		auto psAnchor = SceneNode::Create("Anchor");
		psAnchor->AddComponent<AnchorComponent>();
		psAnchor->AttachChild(psShip);
		psAnchor->AttachChild(psGround);

		auto psCamera = SceneNode::Create("Camera");
		//auto pCamerComponent = psCamera->AddComponent<OrbitTargetCameraComponent>();
		auto pCamerComponent = psCamera->AddComponent<FollowShipCameraComponent>();		
		pCamerComponent->SetTarget(psShip);
		pwCamera = psCamera;

		// Create a bunch of randomly positioned buildings
		{
			const float32 firstZ = 5000.f;
			const float32 deltaZ = 2000.f;

			auto psStaticMesh = fbxLoader.LoadStaticMesh("data/Building1.fbx");

			for (uint32 i = 0; i < 100; ++i)
			{
				auto psBuilding = SceneNode::Create(str_format("Building_%d", i));
				psBuilding->AddComponent<StaticMeshComponent>()->Init(psStaticMesh);

				auto& mLocal = psBuilding->ModifyLocalToParent();
				mLocal.trans.z = firstZ + deltaZ * i;
				mLocal.trans.x = MathEx::Rand(-500.f, 500.f);
			}
			psStaticMesh = nullptr;
		}

		fbxLoader.Shutdown();
	}

	// Main game loop

	bool bQuit = false;
	while (!bQuit)
	{
		// Handle pause
		if ( !System::IsDebuggerAttached() && !gfxEngine.HasFocus() ) // Auto-pause when we window loses focus
		{
			frameTimer.SetPaused(true);
		}
		else if (kbMgr[vkeyPause].JustPressed())
		{
			frameTimer.TogglePaused();
		}

		// Frame time update
		frameTimer.Update();

		// Update and apply time scale
		//@TODO: Fold this into frameTimer?
		static float32 timeScales[] = {0.1f, 0.25f, 0.5f, 1.f, 2.f, 4.f};
		static int32 timeScaleIndex = 3;
		if (kbMgr[vkeyTimeScaleInc].JustPressed())
		{
			timeScaleIndex = MathEx::Min(timeScaleIndex+1, (int)ARRAYSIZE(timeScales)-1);
		}
		else if (kbMgr[vkeyTimeScaleDec].JustPressed())
		{
			timeScaleIndex = MathEx::Max(timeScaleIndex-1, 0);
		}
		else if (kbMgr[vkeyTimeScaleReset].JustPressed())
		{
			timeScaleIndex = 3;
		}
		const float32 timeScale = timeScales[timeScaleIndex];

		const float32 deltaTime = timeScale * frameTimer.GetFrameDeltaTime();

		gfxEngine.SetTitle( 
			str_format("Star Fox (Real Time: %.2f, Game Time: %.2f, GameDT: %.4f (scale: %.2f), FPS: %.2f)",
			frameTimer.GetRealElapsedTime(),
			frameTimer.GetElapsedTime(),
			frameTimer.GetFrameDeltaTime(),
			timeScale,
			frameTimer.GetFPS()).c_str() );

		kbMgr.Update(deltaTime);

		if (kbMgr[VK_CONTROL].IsDown())
		{
			if (kbMgr[VK_F1].JustPressed())
			{
				static bool bLighting = glIsEnabled(GL_LIGHTING) == GL_TRUE;
				bLighting = !bLighting;
				bLighting? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
			}
			if (kbMgr[VK_F2].JustPressed())
			{
				//static bool bSmoothShading = GLUtil::GetShadeModel() == ShadeModel::Smooth;
				//bSmoothShading = !bSmoothShading;
				//GLUtil::SetShadeModel(bSmoothShading? ShadeModel::Smooth : ShadeModel::Flat);
				g_drawSockets = !g_drawSockets;
			}
			if (kbMgr[VK_F3].JustPressed())
			{
				g_drawNormals = !g_drawNormals;
			}
			if (kbMgr[VK_F4].JustPressed())
			{
				GLUtil::SetTexturing( !GLUtil::GetTexturing() );
			}
			if (kbMgr[VK_F5].JustPressed())
			{
				static bool bWireframe = false;
				bWireframe = !bWireframe;
				GLUtil::SetWireFrame(bWireframe);
			}
			if (kbMgr[VK_F6].JustPressed())
			{
				g_renderSceneGraph = !g_renderSceneGraph;
			}
		}

		// UPDATE
		std::vector<SceneNodeWeakPtr> sceneNodeList = SceneNode::GetAllNodesSnapshot();
		if ( !frameTimer.IsPaused() )
		{
			for (auto pwNode : sceneNodeList)
			{
				if (const auto& psNode = pwNode.lock())
					psNode->Update(deltaTime);
			}
		}

		SceneNode::ValidateSceneGraph();


		// RENDER
		glClearColor(0.f, 0.f, 0.3f, 0.f);
		glClearDepth(1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Load inverse camera matrix so future transforms are in camera space
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		Matrix43 mInvCam = pwCamera.lock()->GetLocalToWorld();
		//assert(mInvCam.IsOrthogonal());
		mInvCam.axisZ = -mInvCam.axisZ; // Game -> OpenGL (flip Z axis)
		mInvCam.InvertSRT();
		GLfloat mCamGL[16];
		GLUtil::Matrix43ToGLMatrix(mInvCam, mCamGL);
		glLoadMatrixf(mCamGL);

		// Render scene nodes in camera space
		for (auto pwNode : sceneNodeList)
		{
			if (const auto& psNode = pwNode.lock())
			{
				psNode->Render();
			}
		}

		// Render debug objects
		g_debugDrawManager.Render();

		// Render scene graph
		if (g_renderSceneGraph)
		{
			for (auto pwNode : sceneNodeList)
			{
				if (const auto& psNode = pwNode.lock())
				{
					if (is_weak_to_shared_ptr(pwCamera, psNode))
						continue;

					GLUtil::PushAndMultMatrix(psNode->GetLocalToWorld());

					auto pQuadric = gluNewQuadric();
					glColor3f(1.f, 0.f, 0.f);
					gluSphere(pQuadric, 10.f, 8, 8);
					gluDeleteQuadric(pQuadric);

					for (const auto& pwChildNode : psNode->GetChildren())
					{
						if (const auto& psChildNode = pwChildNode.lock())
						{
							const auto& mChildLocal = psChildNode->GetLocalToParent();

							glBegin(GL_LINES);
							glVertex3fv(Vector3::Zero().v);
							glVertex3fv(mChildLocal.trans.v);
							glEnd();
						}
					}
						
					glPopMatrix();
				}
			}
		}

		// Flip buffers, process msgs, etc.
		gfxEngine.Update(bQuit);

		// Handle frame stepping
		static bool stepFrame = false;
		const bool inputStepSingleFrame = kbMgr[vkeyStepFrame].JustPressed();
		const bool inputStepMultipleFrames = kbMgr[vkeyStepFrameRepeatModifier].IsDown() && kbMgr[vkeyStepFrame].IsDown();
		if ( !stepFrame && (inputStepSingleFrame || inputStepMultipleFrames) )
		{
			// Unpause for one frame
			stepFrame = true;
			frameTimer.SetPaused(false);
		}
		else if ( stepFrame && !inputStepMultipleFrames )
		{
			// Go back to being paused
			stepFrame = false;
			frameTimer.SetPaused(true);
		}
	}

	SceneNode::DestroyAllNodes();

	gfxEngine.Shutdown();
}
