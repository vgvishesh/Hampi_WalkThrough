/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#include <math.h>
#include "TutorialApplication.h"
#include "OgreCommon.h"


//#define DYNAMIC_LOD
#define STATIC_LOD

//-------------------------------------------------------------------------------------
TutorialApplication::TutorialApplication(void)
{
}
//-------------------------------------------------------------------------------------
TutorialApplication::~TutorialApplication(void)
{
}

void TutorialApplication::createCamera(void)
{
	
	// create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");
    // set the near clip distance
	mCamera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);
    mCamera->setNearClipDistance(5);
    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
	
	camnode = mSceneMgr->getRootSceneNode()->createChildSceneNode("camnode",Ogre::Vector3(0,12000,0));
    camnode->attachObject(mCamera);
	camnode->pitch(Ogre::Degree(-90.0));

	//perspective camera
	mCamera2 = mSceneMgr->createCamera("PlayerCam2");
    // set the near clip distance
    mCamera2->setNearClipDistance(5);
    mCameraMan2 = new OgreBites::SdkCameraMan(mCamera2);
}

void TutorialApplication::createViewports(void)
{
	// Create one viewport, entire window
	vp1 = mWindow->addViewport(mCamera,0,0,0,500.0/1200.0,1);
	vp1->setBackgroundColour(Ogre::ColourValue(0,0,0));
	//vp1->setOverlaysEnabled(false);
	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(Ogre::Real(vp1->getActualWidth()) / Ogre::Real(vp1->getActualHeight()));

	vp2 = mWindow->addViewport(mCamera2,1,(500.0/1200.0),0,1.0-(500.0/1200.0),1);
	vp2->setBackgroundColour(Ogre::ColourValue(0,0,0));
	//vp1->setOverlaysEnabled(false);
	vp2->setOverlaysEnabled(false);
	// Alter the camera aspect ratio to match the viewport
	mCamera2->setAspectRatio(Ogre::Real(vp2->getActualWidth()) / Ogre::Real(vp2->getActualHeight()));
}

void TutorialApplication::CalcPosition(float &xPos, float& yPos)
{
   float templeW = 493;
   float templeH = 787;
   float boxX = 240;
   float boxY = 170;

   float midY = templeH/2;
   yPos = midY - boxY;
   yPos = yPos/midY;

   float midX = templeW/2;
   xPos = midX - boxX;
   xPos = xPos/midX;
}

void TutorialApplication::LoadMeshes(Ogre::Vector3 dim)
{
	
	Ogre::StringVector meshList;
	/*meshList.push_back("chariotColor2.5.mesh");
	meshList.push_back("chariotColor1.4.mesh");
	meshList.push_back("chariotColor0.75.mesh");
	meshList.push_back("chariotColor0.4.mesh");*/
	meshList.push_back("scaled_mesh.mesh");
	meshList.push_back("scaled_mesh_half3.mesh");
	meshList.push_back("scaled_mesh_half4.mesh");
	meshList.push_back("scaled_mesh_half5.mesh");
	meshList.push_back("scaled_mesh_half6.mesh");
	Ogre::Quaternion rot1(Ogre::Degree(90),Ogre::Vector3::UNIT_Z);
	Ogre::Quaternion rot2(Ogre::Degree(-90),Ogre::Vector3::UNIT_Y);
   for (int i = 0; i < 5; i++)
   {
      mLODEntity[i] = mSceneMgr->createEntity(Ogre::String("LODEntity").append(Ogre::StringConverter::toString(i)), meshList[i]);

      Ogre::AxisAlignedBox abbChariot = mLODEntity[i]->getBoundingBox();
      Ogre::Vector3 bbCenter = abbChariot.getCenter();
      Ogre::Vector3 bbSize = abbChariot.getSize();

      // Create a SceneNode and attach the Entity to it
      Ogre::SceneNode* parentNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::String("LODEntityParent").append(Ogre::StringConverter::toString(i)));
      mLODSceneNode[i] = parentNode->createChildSceneNode(Ogre::String("LODNode").append(Ogre::StringConverter::toString(i)),Ogre::Vector3(-1*bbCenter.x,-1*bbCenter.y,-1*bbCenter.z));
      mLODSceneNode[i]->attachObject(mLODEntity[i]);

      
      mLODEntity[i]->setRenderQueueGroup(Ogre::RENDER_QUEUE_WORLD_GEOMETRY_2);

      //Size of chariot is 2% of the temple length
      float scaleFactor = dim.z * 0.05;
      scaleFactor = scaleFactor / bbSize.z;
      mLODSceneNode[i]->scale(scaleFactor, scaleFactor, scaleFactor);
      bbSize *= scaleFactor;

      float xPos = 0, yPos = 0;
      CalcPosition(xPos, yPos);

      //x = breadth, y = depth, z = length
      mLODSceneNode[i]->translate(-xPos * (dim.x/2), -(dim.y/2 - bbSize.y/2), -yPos * (dim.z/2));
	  mLODSceneNode[i]->rotate(rot1);
	  mLODSceneNode[i]->rotate(rot2);

	  mLODSceneNode[i]->setVisible(false);

   }
}

//-------------------------------------------------------------------------------------
void TutorialApplication::createScene(void)
{
    // Set the scene's ambient light
    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5f, 0.5f, 0.5f));
 
    // Create an Entity
    Ogre::Entity* ogreHead = mSceneMgr->createEntity("Head", "templeTest111.mesh");
	b= ogreHead->getBoundingBox();
	Ogre::Vector3 t= b.getCenter();
	Ogre::Vector3 dim = b.getSize();
    // Create a SceneNode and attach the Entity to it
	Ogre::SceneNode* headNode1 = mSceneMgr->getRootSceneNode()->createChildSceneNode("HeadNode1");
    Ogre::SceneNode* headNode2 = headNode1->createChildSceneNode("HeadNode2",Ogre::Vector3(-1*t.x,-1*t.y,-1*t.z));
    headNode2->attachObject(ogreHead);
	//headNode1->scale(0.15,0.15,0.15);



	#ifdef STATIC_LOD
	   LoadMeshes(dim);

	   for (int i = 0; i < 4; i++)
	   {
		  mLODSceneNode[i]->setVisible(false);
	   }
	#endif

    //Create a Light and set its position
    Ogre::Light* light = mSceneMgr->createLight("MainLight");
    light->setPosition(0.0f, 10000.0f, 0.0f);

	////LOD CODE
	
   mCameraLight = mSceneMgr->createLight("cameraLight");
   mCameraLight->setType(Ogre::Light::LT_POINT);
   mCameraLight->setDiffuseColour(0.5, 0.5, 0.5);
   mCameraLight->setSpecularColour(0.5, 0.5, 0.5);

	//create skybox.
	mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

	//create arrow
	/*
	Ogre::Entity* arrow = mSceneMgr->createEntity("arrow", "arrow.mesh");
	Ogre::SceneNode* arrowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("arrowNode");
    arrowNode->attachObject(arrow);
	arrowNode->translate(4000,4000,0);
	arrowNode->scale(50,1,100);
	*/

	// Assuming scene_mgr is your SceneManager.
    Ogre::ManualObject * circle = mSceneMgr->createManualObject("circle");
	float const radius = 7;
    float const accuracy = 35;
    circle->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    unsigned point_index = 0;
    for(float theta = 0; theta <= 2 * Ogre::Math::PI; theta += Ogre::Math::PI / accuracy) {
        circle->position(radius * cos(theta), 0, radius * sin(theta));
        circle->index(point_index++);
    }
    circle->index(0);// Rejoins the last point to the first.
    circle->end();
	circleNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("circleNode");
    circleNode->attachObject(circle);
	circleNode->scale(100,1,100);
	//circleNode->translate(4000,-4000,0);

	mCamera->setOrthoWindow(dim.x,dim.z);

	mWindow->resize(500+700,500*dim.z/dim.x);
	//winsize.x = 500;
	winsize.x = 0.416*mWindow->getWidth();
	winsize.y = mWindow->getHeight();
	mCamera->setAspectRatio(dim.x/dim.z);
	
	mCamera2->setAspectRatio(700.0/mWindow->getHeight());
	mCamera2->setPosition(4000,(-dim.y/2)+1000,0);
	mCamera2->lookAt(4000,(-dim.y/2)+1000,-1);
	
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
    int main(int argc, char *argv[])
#endif
    {
        // Create application object
        TutorialApplication app;

        try {
            app.go();
        } 
	
		catch( Ogre::Exception& e ) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
#endif
        }
		

        return 0;
    }

#ifdef __cplusplus
}
#endif
