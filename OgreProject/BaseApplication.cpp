/*
-----------------------------------------------------------------------------
Filename:    BaseApplication.cpp
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
#include "BaseApplication.h"
#include "OgreCommon.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;
#define SPEED_H 30.0
#define SPEED_M 15.0	//used in default speed case.
#define SPEED_L 10.0
#define STOP_COUNTER_L 500
#define STOP_COUNTER_M 750
#define STOP_COUNTER_H 1000


//-------------------------------------------------------------------------------------
BaseApplication::BaseApplication(void)
    : mRoot(0),
    mCamera(0),
	mCamera2(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
	mCameraMan2(0),
    mDetailsPanel(0),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	overlayManager(0),
	mCameraLight(0),
	mbLOD(1),
	mbLodThread(1)
{
}

//-------------------------------------------------------------------------------------
BaseApplication::~BaseApplication(void)
{
    if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
	if(mCameraMan2) delete mCameraMan2;
    //Remove ourself as a Window listener
    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//-------------------------------------------------------------------------------------
bool BaseApplication::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "Hampi WalkThrough");
		mWindow->reposition(100
			,0);

        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
}
//-------------------------------------------------------------------------------------
void BaseApplication::createCamera(void)
{
    // Create the camera
    mCamera = mSceneMgr->createCamera("PlayerCam");

    // Position it at 500 in Z direction
    mCamera->setPosition(Ogre::Vector3(0,0,80));
    // Look back along -Z
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(5);

    mCameraMan = new OgreBites::SdkCameraMan(mCamera);   // create a default camera controller
}
//-------------------------------------------------------------------------------------
void BaseApplication::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    
	//show cursor.
	pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
	pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
    pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
    pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
    
	mInputManager = OIS::InputManager::createInputSystem( pl );

    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

    mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    //mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
    mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");
	items.push_back("clicked_points");
	items.push_back("debug1");
	items.push_back("debug2");
	items.push_back("pathdist");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

	overlayManager = &Ogre::OverlayManager::getSingleton();
    overlay = overlayManager->create( "OverlayName" );
	overlay->show();

    mRoot->addFrameListener(this);
}
//-------------------------------------------------------------------------------------
void BaseApplication::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void BaseApplication::createViewports(void)
{
    // Create one viewport, entire window
    Ogre::Viewport* vp = mWindow->addViewport(mCamera);
    vp->setBackgroundColour(Ogre::ColourValue(0,0,0));

    // Alter the camera aspect ratio to match the viewport
    mCamera->setAspectRatio(
        Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void BaseApplication::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void BaseApplication::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void BaseApplication::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void BaseApplication::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool BaseApplication::setup(void)
{
    mRoot = new Ogre::Root(mPluginsCfg);

    setupResources();

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();

	walk=false;
	//speed_persp = 25;
	//speed = 25;
	pathdist = 0;
	disttrav = 0;
	import = false;
	totalselectedpoints = 0;
	lastselectedpointindex = -1;
	prev_index_for_serach=0;
	rclick_elemid=-1;
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
int BaseApplication::getindex(float dist)
{
	if(dist==0.0)return 0;
	for(int i=prev_index_for_serach;i<dists.size();i++)
	{
		if(dists[i]>dist)
		{
			prev_index_for_serach=i-1;
			return i-1;
		}
	}
}
//-------------------------------------------------------------------------------------
bool BaseApplication::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;
	
	//if (mbLOD == true)
	//{
 //     updateStaticLod();
	//}
	Ogre::Vector3 pos = mCamera2->getPositionForViewUpdate();
	mCameraLight->setPosition(pos);

    //Need to capture/update each device
    mKeyboard->capture();
    mMouse->capture();
    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
		mCameraMan2->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }
	if(walk)
	{
		int temp1 = getindex(disttrav);
		float t = (disttrav-dists[temp1])/(dists[temp1+1]-dists[temp1]);
		Ogre::Vector3 pos,lookat;
		if(!import) pos = sp->interpolate(disttrav/pathdist);
		else pos = clicked_points[temp1]*(1-t)+clicked_points[temp1+1]*(t);

		mCamera2->setPosition(convert_persp(pos));
		circleNode->setPosition(convert(pos));
		mSceneMgr->getSceneNode("gazedirnode")->setPosition(circleNode->getPosition());

		float yaw = gazeAtp[temp1]*(1-t)+gazeAtp[temp1+1]*(t);
		float r = Ogre::Degree(yaw).valueRadians();
		//const Ogre::Quaternion *q = new Ogre::Quaternion(Ogre::Radian(yaw/2.0),Ogre::Vector3(0,1,0));
		mSceneMgr->getSceneNode("gazedirnode")->resetOrientation();
		mSceneMgr->getSceneNode("gazedirnode")->yaw(Ogre::Radian(r));
		
		lookat = convert_persp(pos)+Ogre::Vector3(-sin(r),0,-cos(r));
		mCamera2->lookAt(lookat);

		if(stopflagtemp[temp1])
		{
			stopAtptemp[temp1]--;
			if(stopAtptemp[temp1]<0)stopflagtemp[temp1]=false;
		}
		else
		{
			float speed_persp = speedAtp[temp1]*(1-t)+speedAtp[temp1+1]*(t);
			disttrav+=speed_persp;
			if(disttrav>pathdist)
			{
				walk=false;
				disttrav = 0;
			}
			/*
			else
			{
				mDetailsPanel->setParamValue(11, Ogre::StringConverter::toString(temp1));
				mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(dists[temp1+1]));
				mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(yaw));
			}
			*/
		}
	}
    return true;
}
//-------------------------------------------------------------------------------------
bool BaseApplication::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
            newVal = "None";
            tfo = Ogre::TFO_NONE;
            aniso = 1;
            break;
        default:
            newVal = "Bilinear";
            tfo = Ogre::TFO_BILINEAR;
            aniso = 1;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
        mDetailsPanel->setParamValue(9, newVal);
    }
    else if (arg.key == OIS::KC_R) //reset 
    {
		Ogre::SceneNode *itemNode;
		if(!import)
		{
			for(int i=1;i<=clicked_points.size();i++)
			{
				itemNode = mSceneMgr->getSceneNode(Ogre::String("arrownode").append( Ogre::StringConverter::toString(i)));
				DestroyAllAttachedMovableObjects(itemNode);
				itemNode->removeAndDestroyAllChildren();
				mSceneMgr->destroySceneNode(itemNode);
				overlayManager->destroyOverlayElement(overlayManager->getOverlayElement(Ogre::String("PanelName").append( Ogre::StringConverter::toString(i))));
			}
			/*
			for(int j=0;j<clicked_points.size()-1;j++)
			{
				for(int i=1;i<=20;i++)
				{
					Ogre::OverlayElement *e = overlayManager->getOverlayElement(Ogre::StringConverter::toString(j).append
						(Ogre::String("Splinepoint").append( Ogre::StringConverter::toString(i))));
					overlayManager->destroyOverlayElement(e);
				}
			}
			*/
		}
		else
		{
			for(int i=1;i<=totalselectedpoints;i++)
			{
				itemNode = mSceneMgr->getSceneNode(Ogre::String("arrownode_import").append( Ogre::StringConverter::toString(i)));
				DestroyAllAttachedMovableObjects(itemNode);
				itemNode->removeAndDestroyAllChildren();
				mSceneMgr->destroySceneNode(itemNode);
				overlayManager->destroyOverlayElement(overlayManager->getOverlayElement(Ogre::String("PanelName_import").append( Ogre::StringConverter::toString(i))));
			}
			/*
			for(int j=0;j<clicked_points.size();j++)
			{
				Ogre::OverlayElement *e = overlayManager->getOverlayElement(Ogre::String("Splinepoint_import").append( Ogre::StringConverter::toString(j)));
				overlayManager->destroyOverlayElement(e);
			}
			*/
		}

		//destroy the path.
		itemNode = mSceneMgr->getSceneNode("manual1_node");
		DestroyAllAttachedMovableObjects(itemNode);
		itemNode->removeAndDestroyAllChildren();
		mSceneMgr->destroySceneNode(itemNode);
		
		//destroy gazedir arrow.
		itemNode = mSceneMgr->getSceneNode(Ogre::String("gazedirnode"));
		DestroyAllAttachedMovableObjects(itemNode);
		itemNode->removeAndDestroyAllChildren();
		mSceneMgr->destroySceneNode(itemNode);
		/*
		if(!import || (import&&totalselectedpoints>0))
		{
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s1"));
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s2"));
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s3"));
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s4"));
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("sbar"));
			overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("stopstatus"));
		}
		*/
		clicked_points.clear();
		dists.clear();
		if(!import)sp->clear();

		stopflag.clear();
		stopAtp.clear();
		speedflag.clear();
		speedAtp.clear();
		gazeflag.clear();
		gazeAtp.clear();
		
		walk=false;
		pathdist = 0;
		disttrav = 0;
		overlay->clear();
		import = false;
		totalselectedpoints = 0;
		prev_index_for_serach=0;
		indices.clear();
		stopflagtemp.clear();
		stopAtptemp.clear();
    }
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_P)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
	else if (arg.key == OIS::KC_M)//make interpolated spline and initialise associated variables.
	{
		
		Ogre::Vector3 interpolated;
		Ogre::Vector3 prev;
		Ogre::Entity* arrow;
		Ogre::SceneNode* arrowNode;
		float temp;
		if(!import)
		{
			sp = new Ogre::SimpleSpline();
			sp->setAutoCalculate(true);
			for(int i=0;i<clicked_points.size();i++)
			{
				sp->addPoint(clicked_points[i]);
			}
			Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1"); 
			Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node"); 
 
			myManualObject->begin("Template/Blue", Ogre::RenderOperation::OT_LINE_STRIP); 
			myManualObject->position(convert(clicked_points[0]));
		
			dists.push_back(0.0);
			for(int j=0;j<(clicked_points.size()-1);j++)
			{
				temp = 0.0;
				prev = clicked_points[j];
				for(int i=1;i<=20;i++)//sample spline at 19 intermediate points.
				{
					interpolated = sp->interpolate(j,i*0.05);
					myManualObject->position(convert(interpolated));
					temp+= (convert(interpolated) - convert(prev)).length();
					/*
					panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( 
						"Panel", Ogre::StringConverter::toString(j).append(Ogre::String("Splinepoint").append( Ogre::StringConverter::toString(i)))));
					panel->setMetricsMode(Ogre::GMM_PIXELS);
					panel->setPosition(interpolated.x,interpolated.y);
					panel->setDimensions( 2, 2 );
					if(i==20)panel->setMaterialName( "BaseWhite" );
					else panel->setMaterialName( "Template/Red1" );
					overlay->add2D( panel );
					*/
					prev = interpolated;
				}
				pathdist+=temp;
				dists.push_back(pathdist);
			}
			myManualObject->end(); 
			myManualObjectNode->attachObject(myManualObject);

			//pathdist+=(convert(sp->interpolate(20*0.05)) - convert(sp->interpolate(19*0.05))).length();
			mDetailsPanel->setParamValue(14, Ogre::StringConverter::toString(pathdist));
		}
		//for start point.
		int j=0;
		if(!gazeflag[j])
		{
			gazeflag[j]=true;
			Ogre::Vector3 negz = Ogre::Vector3(0,0,-1);
			Ogre::Vector3 negx = Ogre::Vector3(-1,0,0);
			Ogre::Vector3 tempvec;			
			if(!import)tempvec = convert_persp(sp->interpolate(SPEED_M/pathdist))-convert_persp(sp->interpolate(0));
			else tempvec = convert_persp(clicked_points[1])-convert_persp(clicked_points[0]);

			if(negz.dotProduct(tempvec)>=0)
			{
				if(negx.dotProduct(tempvec)>=0)
				{
					gazeAtp[j]= fmod(tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
				}
				else gazeAtp[j]= fmod(2*180 - tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
			}
			else
			{
				if(negx.dotProduct(tempvec)>=0)
				{
					gazeAtp[j]= fmod(tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
				}
				else gazeAtp[j]= fmod(2*180 - tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
			}
		}
		if(!speedflag[j])
		{
			speedflag[j]=true;
			speedAtp[j]=SPEED_M;
		}
		//at a point we have stop->move(if there is stopflag).

		//for last point.
		j=clicked_points.size()-1;
		if(!gazeflag[j])
		{
			gazeflag[j]=true;
			Ogre::Vector3 negz = Ogre::Vector3(0,0,-1);
			Ogre::Vector3 negx = Ogre::Vector3(-1,0,0);
			Ogre::Vector3 tempvec;
			if(!import)tempvec = (convert_persp(sp->interpolate(1.0))-convert_persp(sp->interpolate(((pathdist)-SPEED_M)/pathdist)));
			else tempvec = convert_persp(clicked_points[clicked_points.size()-1]) - convert_persp(clicked_points[clicked_points.size()-2]);

			if(negz.dotProduct(tempvec)>=0)
			{
				if(negx.dotProduct(tempvec)>=0)
				{
					gazeAtp[j]= fmod(tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
				}
				else gazeAtp[j]= fmod(2*180 - tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
			}
			else
			{
				if(negx.dotProduct(tempvec)>=0)
				{
					gazeAtp[j]= fmod(tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);

				}
				else gazeAtp[j]= fmod(2*180 - tempvec.angleBetween(Ogre::Vector3(0,0,-1)).valueDegrees(),360);
			}
		}
		if(!speedflag[j])
		{
			speedflag[j]=true;
			speedAtp[j]=SPEED_M;
		}

		float t,v1,v2;
		for(j=1;j<(clicked_points.size()-1);j++)
		{
			if(!gazeflag[j])
			{
				gazeflag[j]=true;
				for(int k=j+1;k<clicked_points.size();k++)
				{
					if(gazeflag[k])
					{
						t=(dists[j]-dists[j-1])/(dists[k]-dists[j-1]);
						//v1 = fmod(gazeAtp[j-1],360);
						//v2 = fmod(gazeAtp[k],360);
						//if(abs(v2-v1)<=180)
						gazeAtp[j]=fmod(gazeAtp[k]*t+gazeAtp[j-1]*(1-t),360);
						//else gazeAtp[j]=fmod((v2-180)*t+(v1+180)*(1-t)+180,360);
						break;
					}
				}
			}

			if(!speedflag[j])
			{
				speedflag[j]=true;
				for(int k=j+1;k<clicked_points.size();k++)
				{
					if(speedflag[k])
					{
						t=(dists[j]-dists[j-1])/(dists[k]-dists[j-1]);
						speedAtp[j]=speedAtp[k]*t+speedAtp[j-1]*(1-t);
						break;
					}
				}
			}
		}

		mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(gazeAtp.size()));
		mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(dists.size()));
		
		//showing gazedir arrow in animation.
		arrow = mSceneMgr->createEntity( Ogre::String("gazedir"), "arrow.mesh");
		arrowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::String("gazedirnode"));
		arrowNode->attachObject(arrow);
		arrowNode->scale(50,1,100);
		arrowNode->setPosition(circleNode->getPosition());

		stopflagtemp.clear();
		stopAtptemp.clear();
		for(int i=0;i<stopflag.size();i++)
		{
			//copy
			stopflagtemp.push_back(stopflag[i]);
			stopAtptemp.push_back(stopAtp[i]);
		}
	}
	else if (arg.key == OIS::KC_L)//palce viewer to ortho view.
	{
		
		circleNode->setPosition(convert(clicked_points[0]));
		mSceneMgr->getSceneNode("gazedirnode")->setPosition(circleNode->getPosition());
		mCamera2->setPosition(convert_persp(clicked_points[0]));
		if(!import)mCamera2->lookAt(convert_persp(sp->interpolate(0.1)));
		else mCamera2->lookAt(convert_persp(clicked_points[1]));
	}
	else if (arg.key == OIS::KC_B)//start walk.
	{
		walk = !walk;
	}
	
	///LOD RELATED KEY PRESS
	else if (arg.key == OIS::KC_O)   // disable LOD
	{
      mbLOD = !mbLOD;
	}


	else if (arg.key == OIS::KC_A)//aganin start the walk.
	{
		//M
		stopflagtemp.clear();
		stopAtptemp.clear();
		for(int i=0;i<stopflag.size();i++)
		{
			//copy
			stopflagtemp.push_back(stopflag[i]);
			stopAtptemp.push_back(stopAtp[i]);
		}

		//L
		circleNode->setPosition(convert(clicked_points[0]));
		mSceneMgr->getSceneNode("gazedirnode")->setPosition(circleNode->getPosition());
		mCamera2->setPosition(convert_persp(clicked_points[0]));
		if(!import)mCamera2->lookAt(convert_persp(sp->interpolate(0.1)));
		else mCamera2->lookAt(convert_persp(clicked_points[1]));
		
		prev_index_for_serach=0;

		//B
		disttrav = 0;
		walk = true;
		mOldCamPos.x = 100000; mOldCamPos.y = 100000; mOldCamPos.z = 1000000;

	}
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }
	else if (arg.key == OIS::KC_E)
    {
        char *file = "C:/Users/root/Desktop/IDH_HAMPI/graphics_project/OgreProject/OgreProject/paths/path_3.txt";
		savepath(file);
    }
	else if (arg.key == OIS::KC_I)
    {
        char *file = "C:/Users/root/Desktop/IDH_HAMPI/graphics_project/OgreProject/OgreProject/paths/path_3.txt";
		loadpath(file,true);
		import = true;
    }
    return true;
}

Ogre::Vector3 BaseApplication::convert(Ogre::Vector3 pos)
{
	Ogre::Vector3 dim = b.getSize();
	float x = (pos.x-((float)winsize.x/2.0))*(dim.x/(float)winsize.x);
	float z = -((((float)winsize.y/2.0)-pos.y)*(dim.z/(float)winsize.y));
	float y = 4000;
	return Ogre::Vector3(x,y,z);
}

Ogre::Vector3 BaseApplication::convert_persp(Ogre::Vector3 pos)
{
	Ogre::Vector3 dim = b.getSize();
	float x = (pos.x*dim.x/(float)winsize.x)-(dim.x/2.0);
	float z = (pos.y*dim.z/(float)winsize.y)-(dim.z/2.0);
	float y = (-dim.y/2)+1000;//1000 units up from floor.
	return Ogre::Vector3(x,y,z);
}

Ogre::Vector3 BaseApplication::inverse_convert_persp(Ogre::Vector3 pos)
{
	Ogre::Vector3 dim = b.getSize();
	float x = (pos.x+(dim.x/2.0))*((float)winsize.x/(float)dim.x);
	float y = (pos.y+(dim.z/2.0))*((float)winsize.y/(float)dim.z);
	return Ogre::Vector3(x,y,0);
}

bool BaseApplication::keyReleased( const OIS::KeyEvent &arg )
{
    //mCameraMan->injectKeyUp(arg);
    return true;
}

bool BaseApplication::mouseMoved( const OIS::MouseEvent &arg )
{
	if (arg.state.buttonDown(OIS::MB_Left))
	{
		if(rclick_elemid>=0)// && (((!import)&&rclick_elemid<clicked_points.size())||((import)&&rclick_label<indices.size())))
		{
			if(!import)mSceneMgr->getSceneNode(Ogre::String("arrownode").append( Ogre::StringConverter::toString(rclick_elemid+1)))->yaw(Ogre::Degree(arg.state.X.rel));
			else mSceneMgr->getSceneNode(Ogre::String("arrownode_import").append( Ogre::StringConverter::toString(rclick_label)))->yaw(Ogre::Degree(arg.state.X.rel));
			gazeflag[rclick_elemid] = true;
			gazeAtp[rclick_elemid]+=arg.state.X.rel;
			gazeAtp[rclick_elemid] = fmod(gazeAtp[rclick_elemid],360);
			
		}
	}

	//if (mTrayMgr->injectMouseMove(arg)) return true;
	//mCameraMan->injectMouseMove(arg);
	
	return true;
}

bool BaseApplication::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //if (mTrayMgr->injectMouseDown(arg, id)) return true;
    //mCameraMan->injectMouseDown(arg, id);
	Ogre::Entity* arrow;
	Ogre::SceneNode* arrowNode;
	Ogre::Vector3 temp; 
	switch (id)
	{
	case OIS::MB_Left:
		
		//stop panel.
		if(arg.state.X.abs>=(winsize.x-100-20)&&arg.state.X.abs<=(winsize.x-100-20+8)&&arg.state.Y.abs>=(winsize.y-25)&&arg.state.Y.abs<=(winsize.y-25+8) && rclick_elemid!=-1)
		{
			panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("stopstatus"));
			if(panel->getMaterialName().length()==14)
			{
				panel->setMaterialName( "Template/Red1" );//stop mode.
				panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("sbar"));
				if(stopflag[rclick_elemid])
				{
					panel->setDimensions( (stopAtp[rclick_elemid] - STOP_COUNTER_L)*50/(STOP_COUNTER_H-STOP_COUNTER_L), 7);
				}
				else
				{
					panel->setDimensions(25, 7);
				}
			}
			else 
			{
				panel->setMaterialName( "Template/Green" );//speed mode.
				panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("sbar"));
				if(speedflag[rclick_elemid])
				{
					panel->setDimensions(  (speedAtp[rclick_elemid] - SPEED_L)*50/(SPEED_H-SPEED_L), 7);
				}
				else
				{
					panel->setDimensions(25, 7);
				}
			}
		}
		//speed panel.
		else if(arg.state.X.abs>=(winsize.x-100)&&arg.state.X.abs<=(winsize.x-100+50)&&arg.state.Y.abs>=(winsize.y-25)&&arg.state.Y.abs<=(winsize.y-25+8) && rclick_elemid!=-1)
		{
			panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("sbar"));
			panel->setDimensions(arg.state.X.abs-(winsize.x-100),7);
			panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("stopstatus"));
			if(panel->getMaterialName().length()==13)
			{
				stopflag[rclick_elemid]=true;
				stopAtp[rclick_elemid]=STOP_COUNTER_L +(((arg.state.X.abs-(winsize.x-100))/50.0)*(STOP_COUNTER_H-STOP_COUNTER_L));
			}
			else
			{
				speedflag[rclick_elemid]=true;
				speedAtp[rclick_elemid]=SPEED_L +(((arg.state.X.abs-(winsize.x-100))/50.0)*(SPEED_H-SPEED_L));
			}
		}
		else if(rclick_elemid!=-1)
		{
			break;
		}
		else
		{
			bool valid = false;//used for selecting a point on imported path.
			if(!import)
			{
				gazeflag.push_back(false);
				gazeAtp.push_back(0.0);
				speedflag.push_back(false);
				speedAtp.push_back(0.0);
				stopflag.push_back(false);
				stopAtp.push_back(0.0);
				clicked_points.push_back(Ogre::Vector3(arg.state.X.abs,arg.state.Y.abs,0));
				lastselectedpointindex = clicked_points.size()-1;
				mDetailsPanel->setParamValue(11, Ogre::StringConverter::toString(clicked_points.size()));
				mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(arg.state.X.abs));
				mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(arg.state.Y.abs));
		
				panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", Ogre::String("PanelName").append( Ogre::StringConverter::toString(clicked_points.size()))));
				panel->setMetricsMode(Ogre::GMM_PIXELS);
				panel->setPosition(arg.state.X.abs-3, arg.state.Y.abs -3);
				panel->setDimensions( 7, 7 );
				panel->setMaterialName( "BaseWhite" );
				overlay->add2D( panel );

				arrow = mSceneMgr->createEntity( Ogre::String("arrow").append( Ogre::StringConverter::toString(clicked_points.size())), "arrow.mesh");
				arrowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::String("arrownode").append( Ogre::StringConverter::toString(clicked_points.size())));
				arrowNode->attachObject(arrow);
				arrowNode->setPosition(convert(Ogre::Vector3(arg.state.X.abs, arg.state.Y.abs,0)));
				arrowNode->scale(50,1,100);

			}
			else
			{	
				int index=0;
				Ogre::Vector3 temp;
				for(int i=0;i<clicked_points.size();i++)
				{
					temp = clicked_points[i]-Ogre::Vector3(arg.state.X.abs,arg.state.Y.abs,0);
					if(temp.dotProduct(temp) <= 100)
					{
						index = i;
						valid=true;
						break;
					}
				}
		
				if(valid)
				{
					totalselectedpoints+=1;
					indices.push_back(index);
					lastselectedpointindex = index;

					panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", Ogre::String("PanelName_import").append( Ogre::StringConverter::toString(totalselectedpoints))));
					panel->setMetricsMode(Ogre::GMM_PIXELS);
					panel->setPosition(clicked_points[index].x-3, clicked_points[index].y -3);
					panel->setDimensions( 7, 7 );
					panel->setMaterialName( "BaseWhite" );
					overlay->add2D( panel );

					arrow = mSceneMgr->createEntity( Ogre::String("arrow").append( Ogre::StringConverter::toString(totalselectedpoints)), "arrow.mesh");
					arrowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::String("arrownode_import").append( Ogre::StringConverter::toString(totalselectedpoints)));
					arrowNode->attachObject(arrow);
					arrowNode->setPosition(convert(Ogre::Vector3(clicked_points[index].x, clicked_points[index].y,0)));
					arrowNode->scale(50,1,100);
				}
			}
			/*
			if(clicked_points.size()==1||(import && (totalselectedpoints==1) && valid))//show bottom bar.
			{
				showBottomPanel();
			}
			if(clicked_points.size()>0||(import && valid))
			{
				resetBottomPanel();
			}
			*/
		}
		break;
	case OIS::MB_Right:
		
		if(rclick_elemid!=-1)
		{
			//deselect a selected point.
			if(import)overlayManager->getOverlayElement(Ogre::String("PanelName_import").append( Ogre::StringConverter::toString(rclick_label)))->setMaterialName("BaseWhite");
			else overlayManager->getOverlayElement(Ogre::String("PanelName").append( Ogre::StringConverter::toString(rclick_elemid+1)))->setMaterialName("BaseWhite");
			rclick_elemid = -1;
			resetBottomPanel();
		}
		/*
		rclick_elemid=-1;
		temp = clicked_points[lastselectedpointindex]-Ogre::Vector3(arg.state.X.abs,arg.state.Y.abs,0);
		if(temp.dotProduct(temp) <= 100)
		{
			rclick_elemid = lastselectedpointindex;
			gazeflag[rclick_elemid]=true;
		}
		*/
		else
		{
			if(!import)
			{
				for(int i=0;i<clicked_points.size();i++)
				{
					Ogre::Vector3 temp = clicked_points[i]-Ogre::Vector3(arg.state.X.abs,arg.state.Y.abs,0);
					if(temp.dotProduct(temp) <= 100)
					{
						rclick_elemid = i;
						overlayManager->getOverlayElement(Ogre::String("PanelName").append( Ogre::StringConverter::toString(rclick_elemid+1)))->setMaterialName("Template/Green");
						showBottomPanel();
						break;
					}
				}
			}
			else
			{
				for(int i=0;i<indices.size();i++)
				{
					Ogre::Vector3 temp = clicked_points[indices[i]]-Ogre::Vector3(arg.state.X.abs,arg.state.Y.abs,0);
					if(temp.dotProduct(temp) <= 100)
					{
						rclick_elemid = indices[i];
						rclick_label = i+1;
						overlayManager->getOverlayElement(Ogre::String("PanelName_import").append( Ogre::StringConverter::toString(rclick_label)))->setMaterialName("Template/Green");
						showBottomPanel();
						break;
					}
				}
			}
		}
		
		break;
	default:
		break;
	}
    return true;
}

bool BaseApplication::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    //if (mTrayMgr->injectMouseUp(arg, id)) return true;
    //mCameraMan->injectMouseUp(arg, id);
    return true;
}

//Adjust mouse clipping area
void BaseApplication::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void BaseApplication::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

void BaseApplication::DestroyAllAttachedMovableObjects( Ogre::SceneNode* i_pSceneNode )
{
   if ( !i_pSceneNode )
   {
      return;
   }

   // Destroy all the attached objects
   Ogre::SceneNode::ObjectIterator itObject = i_pSceneNode->getAttachedObjectIterator();

   while ( itObject.hasMoreElements() )
   {
      Ogre::MovableObject* pObject = static_cast<Ogre::MovableObject*>(itObject.getNext());
      i_pSceneNode->getCreator()->destroyMovableObject( pObject );
   }

   // Recurse to child SceneNodes
   Ogre::SceneNode::ChildNodeIterator itChild = i_pSceneNode->getChildIterator();

   while ( itChild.hasMoreElements() )
   {
      Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
      DestroyAllAttachedMovableObjects( pChildNode );
   }
}

void BaseApplication::savepath(char *filename)
{
	ofstream myfile;
	myfile.open (filename);
	Ogre::Vector3 pos;
	//myfile << b.getCenter().x << "," << b.getCenter().y << "," << b.getCenter().z << "\n";
	//myfile << b.getSize().x << "," << b.getSize().y << ","<< b.getSize().z << "\n";
	pos = clicked_points[0];
	pos.x = (pos.x*2/winsize.x)-1;
	pos.y = (pos.y*2/winsize.y)-1;
	myfile << (pos.x) << " " << (pos.y) << "\n";
	for(int j=0;j<(clicked_points.size()-1);j++)
	{
		for(int i=1;i<=20;i++)//sample spline at 19 intermediate points.
		{
			pos = sp->interpolate(j,i*0.05);
			pos.x = (pos.x*2/winsize.x)-1;
			pos.y = (pos.y*2/winsize.y)-1;
			myfile << (pos.x) << " " << (pos.y) << "\n";//positions in coordinate system of model.
		}
	}
	myfile.close();
}

void BaseApplication::loadpath(char *filename,bool richa_path)
{
	string line;
	string token;
	ifstream myfile(filename);
	Ogre::Vector3 pos;
	if (myfile.is_open())
	{
	clicked_points.clear();
	dists.clear();
	pathdist=0;

	float path_scale = 25854.8;
	Ogre::Vector3 path_t = Ogre::Vector3(16842.1,92107.8,0);

	while ( myfile.good() )
	{
		getline(myfile,line);
		istringstream ss(line);
		if(line.compare("")!=0)
		{
			std::getline(ss, token, ' ');
			istringstream(token) >> pos.x;
			std::getline(ss, token, ' ');
			istringstream(token) >> pos.y;
			pos.z = 0;
			if(richa_path)
			{
				pos.x = pos.x*path_scale + path_t.x;
				pos.x = (pos.x-754.151f)/(32930.1f-754.151f)*(30934.7f-(-1241.26f))+(-1241.26f);
				pos.x = pos.x - b.getCenter().x;
			
				pos.y = pos.y*path_scale + path_t.y;
				pos.y = ((pos.y-66511.6f)/(117704.0f-66511.6f))*(107652.0f-56453.5f)+56453.5f;
				pos.y = (-pos.y - b.getCenter().z);
				pos = inverse_convert_persp(pos);
			}
			else
			{
				//for path generated using this program.
				pos.x = (pos.x-(-1))*winsize.x/2.0;
				pos.y = (pos.y-(-1))*winsize.y/2.0;
			}
			
			clicked_points.push_back(pos);
			gazeflag.push_back(false);
			gazeAtp.push_back(0.0);
			speedflag.push_back(false);
			speedAtp.push_back(0.0);
			stopflag.push_back(false);
			stopAtp.push_back(0.0);
		}
	}
	myfile.close();
	}

	else cout << "Unable to open file";

	mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(clicked_points[0].x));
	mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(clicked_points[0].y));
	mDetailsPanel->setParamValue(11, Ogre::StringConverter::toString(clicked_points.size()));

	//draw path.
	Ogre::Vector3 prev;
	pos = clicked_points[0];	
	/*
	panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( 
			"Panel", Ogre::String("Splinepoint_import").append( Ogre::StringConverter::toString(0))));
	panel->setMetricsMode(Ogre::GMM_PIXELS);
	panel->setPosition(pos.x,pos.y);
	panel->setDimensions( 2, 2 );
	panel->setMaterialName( "Template/Red1" );
	overlay->add2D( panel );
	*/
	Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1"); 
	Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node"); 
	myManualObject->begin("Template/Blue", Ogre::RenderOperation::OT_LINE_STRIP); 
	myManualObject->position(convert(pos));

	dists.push_back(0.0);

	for(int i=1;i<clicked_points.size();i++)
	{
		prev = clicked_points[i-1];
		pos = clicked_points[i];
		myManualObject->position(convert(pos));
		/*
		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( 
			"Panel", Ogre::String("Splinepoint_import").append( Ogre::StringConverter::toString(i))));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(pos.x,pos.y);
		panel->setDimensions( 2, 2 );
		panel->setMaterialName( "Template/Red1" );
		overlay->add2D( panel );
		*/
		pathdist+=(convert(pos) - convert(prev)).length();
		dists.push_back(pathdist);
	}
	myManualObject->end(); 
	myManualObjectNode->attachObject(myManualObject);
}

void BaseApplication::showBottomPanel()
{
	panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "s1"));
	panel->setMetricsMode(Ogre::GMM_PIXELS);
	panel->setPosition(winsize.x-100,winsize.y-25);
	panel->setDimensions( 1, 8 );
	panel->setMaterialName( "Template/Black" );
	overlay->add2D( panel );

	panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "s2"));
	panel->setMetricsMode(Ogre::GMM_PIXELS);
	panel->setPosition(winsize.x-100,winsize.y-25);
	panel->setDimensions( 50, 1 );
	panel->setMaterialName( "Template/Black" );
	overlay->add2D( panel );

	panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "s3"));
	panel->setMetricsMode(Ogre::GMM_PIXELS);
	panel->setPosition(winsize.x-100+50,winsize.y-25);
	panel->setDimensions( 1, 8 );
	panel->setMaterialName( "Template/Black" );
	overlay->add2D( panel );

	panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "s4"));
	panel->setMetricsMode(Ogre::GMM_PIXELS);
	panel->setPosition(winsize.x-100,winsize.y-25+8);
	panel->setDimensions( 50, 1 );
	panel->setMaterialName( "Template/Black" );
	overlay->add2D( panel );

	if(!speedflag[rclick_elemid] && !stopflag[rclick_elemid])
	{
		//default
		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "sbar"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100+1,winsize.y-25+1);
		panel->setDimensions( 25, 7);
		panel->setMaterialName( "Template/Green" );
		overlay->add2D( panel );

		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "stopstatus"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100-20,winsize.y-25);
		panel->setDimensions( 9, 9 );
		panel->setMaterialName( "Template/Green" );
		overlay->add2D( panel );
	}
	else if(speedflag[rclick_elemid])
	{
		//show speed
		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "sbar"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100+1,winsize.y-25+1);
		panel->setDimensions( (speedAtp[rclick_elemid] - SPEED_L)*50/(SPEED_H-SPEED_L), 7);
		panel->setMaterialName( "Template/Green" );
		overlay->add2D( panel );

		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "stopstatus"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100-20,winsize.y-25);
		panel->setDimensions( 9, 9 );
		panel->setMaterialName( "Template/Green" );
		overlay->add2D( panel );
	}
	else if(stopflag[rclick_elemid])
	{
		//show stop time.
		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "sbar"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100+1,winsize.y-25+1);
		panel->setDimensions( (stopAtp[rclick_elemid] - STOP_COUNTER_L)*50/(STOP_COUNTER_H-STOP_COUNTER_L), 7);
		panel->setMaterialName( "Template/Green" );
		overlay->add2D( panel );

		panel = static_cast<Ogre::OverlayContainer*>( overlayManager->createOverlayElement( "Panel", "stopstatus"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(winsize.x-100-20,winsize.y-25);
		panel->setDimensions( 9, 9 );
		panel->setMaterialName( "Template/Red1" );
		overlay->add2D( panel );
	}
}
void BaseApplication::resetBottomPanel()
{
	/*
	panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("stopstatus"));
	panel->setMaterialName( "Template/Green" );	
	panel = static_cast<Ogre::OverlayContainer*>(overlayManager->getOverlayElement("sbar"));
	panel->setDimensions( 25, 7);
	*/
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s1"));
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s2"));
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s3"));
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("s4"));
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("sbar"));
	overlayManager->destroyOverlayElement(overlayManager->getOverlayElement("stopstatus"));
}


bool BaseApplication::GetLodLevel(Ogre::Real& reductionValue)
{
   //Ogre::Entity* chariotEnt = mSceneMgr->getEntity("Chariot");
   Ogre::Entity* chariotEnt = mLODEntity[0];
   Ogre::AxisAlignedBox abbChariot = chariotEnt->getBoundingBox();
 
   Ogre::Entity* templeEnt = mSceneMgr->getEntity("Head");
   Ogre::AxisAlignedBox abbTemple = templeEnt->getBoundingBox();


   //if (mPerspectiveCam->isVisible(abbChariot) == false)
   //   return false;


   //If the camera position is changed by a threshold (different from old pos)
   Ogre::Vector3 camPos = mCamera2->getDerivedPosition();

   Ogre::Real templeSize = abbTemple.getCenter().distance(abbTemple.getMaximum());
   if (camPos.distance(mOldCamPos) > (templeSize*0.01))//camera movement is 25% of temple size
   {
      Ogre::Vector3 chariotPos = mLODSceneNode[0]->getPosition();
      Ogre::Vector3 chariotPosAbb = abbChariot.getCenter();
      Ogre::Real maxDist = chariotPosAbb.distance( abbChariot.getMaximum());

      Ogre::Real dist = camPos.distance(chariotPos);
      float x = dist - chariotEnt->getBoundingRadius();


      if (x < 0)
         x = 0;

      float x2 = (x )/ templeSize;
      reductionValue = (- exp(-3 * pow(x2, 2)) + 1);

      char str[200];
      sprintf(str, "%f, x = %f, x2 = %f \n", reductionValue, x, x2);
      ::OutputDebugString(str);

      mOldCamPos = camPos;
      return true;
   }
   return false;
}

/*void BaseApplication::updateLod(bool bThreaded)
{
   Ogre::Real reductionValue = 100;
   if (GetLodLevel(reductionValue) == false)
      return;

   Ogre::Entity* chariotEnt = mSceneMgr->getEntity("Chariot");
   Ogre::LodConfig lodConfig;
   lodConfig.levels.clear();
   lodConfig.mesh = chariotEnt->getMesh();;
   lodConfig.strategy = Ogre::DistanceLodStrategy::getSingletonPtr();
   Ogre::LodLevel lodLevel;
   lodLevel.reductionMethod = Ogre::LodLevel::VRM_PROPORTIONAL;
   lodLevel.distance = 1;
   lodLevel.reductionValue = reductionValue;
   lodConfig.levels.push_back(lodLevel);

   if(bThreaded)
   {
      clearLodQueue();
      QueuedProgressiveMeshGenerator pm;
      pm.generateLodLevels(lodConfig);
   } 
   else 
   {
      ProgressiveMeshGenerator pm;
      pm.generateLodLevels(lodConfig);
   }
}
*/

/*void BaseApplication::clearLodQueue()
{
   // Remove outdated Lod requests to reduce delay.
   PMWorker::getSingleton().clearPendingLodRequests();
}*/





void BaseApplication::updateStaticLod()
{
   Ogre::Real reductionValue = 100;
   if (GetLodLevel(reductionValue) == false)
      return;

   if (reductionValue > 0.99) reductionValue = 0.99;

   for (int i = 0; i < 5; i++)
   {
      mLODSceneNode[i]->setVisible(false);
   }

   reductionValue = reductionValue * 5;
   int i = floorf(reductionValue);
   mLODSceneNode[i]->setVisible(true);
   char str[200];
   sprintf(str, "LOD LEVEL%d\n", i);
   ::OutputDebugString(str);
}
