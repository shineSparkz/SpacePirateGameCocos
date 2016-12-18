#include "AppDelegate.h"
//#include "GameScene.h"
#include "TitleScene.h"

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(480, 320);

static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate() 
{
}

bool AppDelegate::applicationDidFinishLaunching()
{
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
	if (!glview)
	{		
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
			designResolutionSize = cocos2d::Size(1025, 768);
			glview = GLViewImpl::createWithRect("GameApp", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
	#else
			glview = GLViewImpl::create("GameApp");
	#endif
			director->setOpenGLView(glview);
	}

    //director->setDisplayStats(true);
    // set FPS. the default value is 1.0/60 if you don't call this
    //director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    
	auto frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }

	float sf = director->getContentScaleFactor();
	//std::string msg = "Scale Factor: " + ToString(sf);
	//MessageBox(msg.c_str(), "Alert");

    //register_all_packages();
	
	auto scene = TitleScene::createScene();

	// Run with should be used for first only
    director->runWithScene(scene);

	// Kill current and replace with this
	//Director::getInstance()->replaceScene(myScene)

	// pushScene - suspends the execution of the running scene, pushing it on the stack of suspended scenes. Only call this if there is a running scene.
	//Director::getInstance()->pushScene(myScene);

	//popScene - This scene will replace the running one. The running scene will be deleted. Only call this if there is a running scene.
	//Director::getInstance()->popScene(myScene);
    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
	// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be paused
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

void AppDelegate::applicationWillEnterForeground()
{
	// This function will be called when the app is active again
    Director::getInstance()->startAnimation();

    // If you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
