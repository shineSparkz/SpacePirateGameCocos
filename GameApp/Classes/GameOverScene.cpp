#include "GameOverScene.h"

#include "SimpleAudioEngine.h"

#include "TitleScene.h"
USING_NS_CC;

Scene* GameOverScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();
	auto layer = GameOverScene::create();

	scene->addChild(layer);

	return scene;
}

bool GameOverScene::init()
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

	auto title = Label::createWithTTF("GAME OVER", "fonts/arcade.ttf", 90);
	title->setPosition(winsz.width * 0.5f, winsz.height * 0.5f + title->getBoundingBox().size.height);
	title->setColor(Color3B(255, 0, 0));
	this->addChild(title);

	auto info = Label::createWithTTF("Tap Anywhere", "fonts/arcade.ttf", 36);// ("player.png", cocos2d::Rect(0, 0, 64 / scale_factor, 64 / scale_factor));
	info->setAnchorPoint(Vec2(0.5f, 0.5f));
	info->setPosition(winsz.width * 0.5f, winsz.height * 0.5f);
	this->addChild(info);

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	auto listener = EventListenerMouse::create();
	// Set up mouse event lambdas
	listener->onMouseDown = CC_CALLBACK_1(GameOverScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

#else
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(GameOverScene::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(GameOverScene::onTouchEnded, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(GameOverScene::onTouchMoved, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(GameOverScene::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
#endif

	this->scheduleUpdate();
	return true;
}

void GameOverScene::update(float delta)
{
}

void GameOverScene::onMouseDown(cocos2d::Event* ev)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	EventMouse* mouseEvent = dynamic_cast<EventMouse*>(ev);
	mouseEvent->getMouseButton();

	auto bounds = ev->getCurrentTarget()->getBoundingBox();
	if (bounds.containsPoint(Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY())))
	{
		auto director = Director::getInstance();
		auto scene = TitleScene::createScene();
		director->replaceScene(scene);
	}
#endif
}

bool GameOverScene::onTouchBegan(Touch* touch, Event* event)
{
	auto bounds = event->getCurrentTarget()->getBoundingBox();

	if (bounds.containsPoint(touch->getLocation()))
	{
		auto director = Director::getInstance();
		auto scene = TitleScene::createScene();
		director->replaceScene(scene);
	}
	return true;
}

void GameOverScene::onTouchEnded(Touch* touch, Event* event)
{
}

void GameOverScene::onTouchMoved(Touch* touch, Event* event)
{
}

void GameOverScene::onTouchCancelled(Touch* touch, Event* event)
{
}