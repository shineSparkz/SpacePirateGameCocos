#include "PauseScene.h"

#include "SimpleAudioEngine.h"

#include "Test.h"
USING_NS_CC;

Scene* PauseScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();
	auto layer = PauseScene::create();

	scene->addChild(layer);

	return scene;
}

bool PauseScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto director = Director::getInstance();
	float scale_factor = director->getContentScaleFactor();
	auto winsz = director->getWinSize();

	m_WinWidth = winsz.width;
	m_WinHeight = winsz.height;

	auto title = Label::createWithTTF("Paused", "fonts/arcade.ttf", 90);
	title->setPosition(winsz.width * 0.5f, winsz.height * 0.5f + title->getBoundingBox().size.height);
	title->setColor(Color3B(0, 255, 0));
	this->addChild(title);

	auto info = Label::createWithTTF("Tap Anywhere", "fonts/arcade.ttf", 36);// ("player.png", cocos2d::Rect(0, 0, 64 / scale_factor, 64 / scale_factor));
	info->setAnchorPoint(Vec2(0.5f, 0.5f));
	info->setPosition(winsz.width * 0.5f, winsz.height * 0.5f);
	this->addChild(info);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	auto listener = EventListenerMouse::create();
	// Set up mouse event lambdas
	listener->onMouseDown = CC_CALLBACK_1(PauseScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

#else
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(PauseScene::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(PauseScene::onTouchEnded, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(PauseScene::onTouchMoved, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(PauseScene::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
#endif

	this->scheduleUpdate();
	return true;
}

void PauseScene::update(float delta)
{
}

void PauseScene::onMouseDown(cocos2d::Event* ev)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	EventMouse* mouseEvent = dynamic_cast<EventMouse*>(ev);
	mouseEvent->getMouseButton();

	auto bounds = ev->getCurrentTarget()->getBoundingBox();
	if (bounds.containsPoint(Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY())))
	{
		auto director = Director::getInstance();
		// Pop this and resume game
		director->popScene();
	}
#endif
}

bool PauseScene::onTouchBegan(Touch* touch, Event* event)
{
	auto bounds = event->getCurrentTarget()->getBoundingBox();

	if (bounds.containsPoint(touch->getLocation()))
	{
		auto director = Director::getInstance();
		// Pop this and resume game
		director->popScene();
	}
	return true;
}

void PauseScene::onTouchEnded(Touch* touch, Event* event)
{
}

void PauseScene::onTouchMoved(Touch* touch, Event* event)
{
}

void PauseScene::onTouchCancelled(Touch* touch, Event* event)
{
}