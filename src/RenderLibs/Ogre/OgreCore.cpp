#include "GraphicsInterface.hpp"

#include "graphics/graphics.hpp"
#include <SFML/System.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#include "OgreArchiveManager.h"
#include "OgreCamera.h"
#include "OgreConfigFile.h"
#include "OgreRoot.h"
#include "OgreWindow.h"

#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsUnlit.h"

#include "OgreItem.h"
#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMesh2.h"
#include "OgreMeshManager2.h"

#include "Compositor/OgreCompositorManager2.h"
#pragma GCC diagnostic pop

#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>    // vec3

static Ogre::Root* root = nullptr;

bool g_graphicsIntialized = false;

static void registerHlms(void)
{
	using namespace Ogre;

	String resourcePath = "data/";

	ConfigFile cf;
	cf.load(resourcePath + "resources2.cfg");

	String rootHlmsFolder = resourcePath + cf.getSetting("DoNotUseAsResource", "Hlms", "");

	if (rootHlmsFolder.empty())
		rootHlmsFolder = "./";
	else if (*(rootHlmsFolder.end() - 1) != '/')
		rootHlmsFolder += "/";

	// At this point rootHlmsFolder should be a valid path to the Hlms data folder

	HlmsUnlit* hlmsUnlit = 0;
	HlmsPbs* hlmsPbs = 0;

	// For retrieval of the paths to the different folders needed
	String mainFolderPath;
	StringVector libraryFoldersPaths;
	StringVector::const_iterator libraryFolderPathIt;
	StringVector::const_iterator libraryFolderPathEn;

	ArchiveManager& archiveManager = ArchiveManager::getSingleton();

	{
		// Create & Register HlmsUnlit
		// Get the path to all the subdirectories used by HlmsUnlit
		HlmsUnlit::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
		Archive* archiveUnlit =
		    archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);
		ArchiveVec archiveUnlitLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while (libraryFolderPathIt != libraryFolderPathEn)
		{
			Archive* archiveLibrary =
			    archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
			archiveUnlitLibraryFolders.push_back(archiveLibrary);
			++libraryFolderPathIt;
		}

		// Create and register the unlit Hlms
		hlmsUnlit = OGRE_NEW HlmsUnlit(archiveUnlit, &archiveUnlitLibraryFolders);
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsUnlit);
	}

	{
		// Create & Register HlmsPbs
		// Do the same for HlmsPbs:
		HlmsPbs::getDefaultPaths(mainFolderPath, libraryFoldersPaths);
		Archive* archivePbs =
		    archiveManager.load(rootHlmsFolder + mainFolderPath, "FileSystem", true);

		// Get the library archive(s)
		ArchiveVec archivePbsLibraryFolders;
		libraryFolderPathIt = libraryFoldersPaths.begin();
		libraryFolderPathEn = libraryFoldersPaths.end();
		while (libraryFolderPathIt != libraryFolderPathEn)
		{
			Archive* archiveLibrary =
			    archiveManager.load(rootHlmsFolder + *libraryFolderPathIt, "FileSystem", true);
			archivePbsLibraryFolders.push_back(archiveLibrary);
			++libraryFolderPathIt;
		}

		// Create and register
		hlmsPbs = OGRE_NEW HlmsPbs(archivePbs, &archivePbsLibraryFolders);
		Root::getSingleton().getHlmsManager()->registerHlms(hlmsPbs);
	}

	RenderSystem* renderSystem = Root::getSingletonPtr()->getRenderSystem();
	if (renderSystem->getName() == "Direct3D11 Rendering Subsystem")
	{
		// Set lower limits 512kb instead of the default 4MB per Hlms in D3D 11.0
		// and below to avoid saturating AMD's discard limit (8MB) or
		// saturate the PCIE bus in some low end machines.
		bool supportsNoOverwriteOnTextureBuffers;
		renderSystem->getCustomAttribute("MapNoOverwriteOnDynamicBufferSRV",
		                                 &supportsNoOverwriteOnTextureBuffers);

		if (!supportsNoOverwriteOnTextureBuffers)
		{
			hlmsPbs->setTextureBufferDefaultSize(512 * 1024);
			hlmsUnlit->setTextureBufferDefaultSize(512 * 1024);
		}
	}
}

namespace Graphics
{
void Initialize(window& win, int winWidth, int winHeight)
{
	using namespace Ogre;

	const String pluginsFolder = "./data/";
	const String writeAccessFolder = "./";

#ifndef OGRE_STATIC_LIB
#if OGRE_DEBUG_MODE
	const char* pluginsFile = "plugins_d.cfg";
#else
	const char* pluginsFile = "plugins.cfg";
#endif
#endif
	root = OGRE_NEW Root(pluginsFolder + pluginsFile,     //
	                     writeAccessFolder + "ogre.cfg",  //
	                     writeAccessFolder + "Ogre.log");

	// TODO: Make this return false and quit the app
	if (!root->showConfigDialog())
		return; // false

	// Initialize Root
	root->getRenderSystem()->setConfigOption("sRGB Gamma Conversion", "Yes");
	// Window* window = root->initialise(/*autoCreateWindow=*/true, "Spargus Ogre");
	root->initialise(/*autoCreateWindow=*/false);

	Ogre::NameValuePairList windowSettings;
	unsigned long winHandle = reinterpret_cast<unsigned long>(win.getBase()->getSystemHandle());
#ifdef _WIN32
	unsigned long winGlContext = reinterpret_cast<unsigned long>(wglGetCurrentContext());

	windowSettings["externalWindowHandle"] = StringConverter::toString(winHandle);
	windowSettings["externalGLContext"] = StringConverter::toString(winGlContext);
	windowSettings["externalGLControl"] = String("True");
#else
	// Deprecated. See ogre-next/RenderSystems/GL3Plus/src/windowing/GLX/OgreGLXWindow.cpp:237
	// windowSettings["externalWindowHandle"] = StringConverter::toString(winHandle);
	windowSettings["parentWindowHandle"] = StringConverter::toString(winHandle);
	// sf::Context context;
	// unsigned long activeContextId = (unsigned long)context.getActiveContextId();
	windowSettings["currentGLContext"] = String("True");
// windowSettings["currentGLContext"] = StringConverter::toString(activeContextId);
	// windowSettings["externalGLControl"] = String("True");
#endif

	// Because of windowSettings, we'll actually use the SFML window instead of making one
	Window* window =
	    root->createRenderWindow("Spargus Ogre", winWidth, winHeight, false, &windowSettings);

	// window->_setVisible(true);

	registerHlms();

	// Create SceneManager
	const size_t numThreads = 1u;
	SceneManager* sceneManager =
	    root->createSceneManager(ST_GENERIC, numThreads, "SceneManager");

	// Create & setup camera
	Camera* camera = sceneManager->createCamera("Main Camera");

	// Position it at 500 in Z direction
	camera->setPosition(Vector3(0, 5, 15));
	// Look back along -Z
	camera->lookAt(Vector3(0, 0, 0));
	camera->setNearClipDistance(0.2f);
	camera->setFarClipDistance(1000.0f);
	camera->setAutoAspectRatio(true);

	// Setup a basic compositor with a blue clear colour
	CompositorManager2* compositorManager = root->getCompositorManager2();
	const String workspaceName("Demo Workspace");
	// const IdString definitionNameId = workspaceName;
	const ColourValue backgroundColour(0.2f, 0.4f, 0.6f);
	compositorManager->createBasicWorkspaceDef(workspaceName, backgroundColour, IdString());
	compositorManager->addWorkspace(sceneManager, window->getTexture(), camera, workspaceName,
	                                true);

	// Mesh importing
	{
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("data/Models", "FileSystem",
		                                                               "Models");

		Ogre::v1::MeshPtr v1Mesh;
		Ogre::MeshPtr v2Mesh;
		// Load the v1 mesh. Notice the v1 namespace
		// Also notice the HBU_STATIC flag; since the HBU_WRITE_ONLY
		// bit would prohibit us from reading the data for importing.
		v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
		    "Chassis.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		    Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC);

		bool halfPosition = true;
		bool halfUVs = true;
		bool useQtangents = true;

		// Create a v2 mesh to import to, with a different name (arbitrary).
		// Import the v1 mesh to v2
		v2Mesh = Ogre::MeshManager::getSingleton().createByImportingV1(
		    "Chassis.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		    v1Mesh.get(), halfPosition, halfUVs, useQtangents);

		// We don't need the v1 mesh. Free CPU memory, get it out of the GPU.
		// Leave it loaded if you want to use athene with v1 Entity.
		v1Mesh->unload();

		// Create an Item with the model we just imported.
		// Notice we use the name of the imported model. We could also use the overload
		// with the mesh pointer:
		//  item = sceneManager->createItem( v2Mesh, Ogre::SCENE_DYNAMIC );
		Ogre::Item* item = sceneManager->createItem(
		    "Chassis.mesh Imported", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
		    Ogre::SCENE_DYNAMIC);
		// Ogre::SceneNode* rootSceneNode = sceneManager->getRootSceneNode(Ogre::SCENE_DYNAMIC);
		Ogre::SceneNode* rootSceneNode = sceneManager->getRootSceneNode();
		if (!rootSceneNode)
		{
			return;
		}
		// WTF
		// Ogre::SceneNode* sceneNode = rootSceneNode->createChildSceneNode(Ogre::SCENE_DYNAMIC);
		Ogre::SceneNode* sceneNode = static_cast<SceneNode*>(rootSceneNode->createChild()); //rootSceneNode->createChildSceneNode();
		if (!sceneNode)
		{
			return;
		}
		sceneNode->attachObject(item);
		// sceneNode->scale( 0.1f, 0.1f, 0.1f );
	}

	g_graphicsIntialized = true;
}

void SetViewport(int x, int y, int width, int height)
{
}

void OnWindowResized(int winWidth, int winHeight)
{
}

void SetCameraTransform(const glm::mat4& newTransform)
{
}

glm::mat4 GetCameraTransformCopy()
{
	return glm::mat4(1.f);
}

glm::mat4 GetCameraProjectionMatrixCopy()
{
	return glm::mat4(1.f);
}

void Update(float frameTime)
{
	root->renderOneFrame();
}

void Destroy()
{
	OGRE_DELETE root;
	root = nullptr;
}
}  // namespace Graphics
