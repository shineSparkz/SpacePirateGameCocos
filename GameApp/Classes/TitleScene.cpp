#include "TitleScene.h"
#include "SimpleAudioEngine.h"

#include "GameScene.h"

USING_NS_CC;

Scene* TitleScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create(); 
    auto layer = TitleScene::create();

    scene->addChild(layer);

    return scene;
}

bool TitleScene::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
	auto director = Director::getInstance();
	float scale_factor = director->getContentScaleFactor();
	auto winsz = director->getWinSize();

	m_WinWidth = winsz.width;
	m_WinHeight = winsz.height;

	// Real size of frame * scale
	auto title = Label::createWithTTF("Space Pirate", "fonts/arcade.ttf", 120 / scale_factor);
	title->setPosition(winsz.width * 0.5f, winsz.height * 0.5f + title->getBoundingBox().size.height);
	title->setAnchorPoint(Vec2(0.5f, 0.5f));
	title->setColor(Color3B(0, 255, 0));
	this->addChild(title);

	auto info = Label::createWithTTF("Tap Anywhere", "fonts/arcade.ttf", 36 / scale_factor);// ("player.png", cocos2d::Rect(0, 0, 64 / scale_factor, 64 / scale_factor));
	info->setAnchorPoint(Vec2(0.5f, 0.5f));
	info->setPosition(winsz.width * 0.5f, winsz.height * 0.5f);
	this->addChild(info);

	auto name = Label::createWithTTF("Alex Spellman", "fonts/arcade.ttf", 28 / scale_factor);// ("player.png", cocos2d::Rect(0, 0, 64 / scale_factor, 64 / scale_factor));
	name->setAnchorPoint(Vec2(0.5f, 0.5f));
	name->setPosition(winsz.width * 0.5f, 64);
	this->addChild(name);

	// Next stars
	m_Stars.resize(200);
	for (unsigned i = 0; i < 200; ++i)
	{
		auto star = Sprite::create("star.png");
		star->setPosition3D(Vec3(cocos2d::random(0.0f, m_WinWidth), cocos2d::random(0.0f, m_WinHeight), cocos2d::random(0.0f, 499.0f)));
		this->addChild(star);
		m_Stars[i] = star;
	}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	auto listener = EventListenerMouse::create();

	// Set up mouse event lambdas
	listener->onMouseDown = CC_CALLBACK_1(TitleScene::onMouseDown, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#else
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(TitleScene::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(TitleScene::onTouchEnded, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(TitleScene::onTouchMoved, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(TitleScene::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
#endif

	this->scheduleUpdate();

    return true;
}

void TitleScene::update(float delta)
{
	//render the 3D stars behind everything
	for (auto i = m_Stars.begin(); i != m_Stars.end(); ++i)
	{
		float x = (*i)->getPositionX();
		float y = (*i)->getPositionY();
		float z = (*i)->getPositionZ();

		(*i)->setPositionZ(--z);

		if ((*i)->getPositionZ() <= 0)
		{
			(*i)->setPosition3D(Vec3(x, y, 499));//loop round
		}
	}
}

void TitleScene::onMouseDown(cocos2d::Event* ev)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	EventMouse* mouseEvent = dynamic_cast<EventMouse*>(ev);
	mouseEvent->getMouseButton();
	
	auto bounds = ev->getCurrentTarget()->getBoundingBox();
	if (bounds.containsPoint(Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY())))
	{
		auto director = Director::getInstance();
		auto game_scene = GameScene::createScene();
		director->replaceScene(game_scene);
	}

#endif
}

bool TitleScene::onTouchBegan(Touch* touch, Event* event)
{
	auto bounds = event->getCurrentTarget()->getBoundingBox();

	if (bounds.containsPoint(touch->getLocation()))
	{
		auto director = Director::getInstance();
		auto game_scene = GameScene::createScene();
		director->replaceScene(game_scene);
	}

	return true;
}

void TitleScene::onTouchEnded(Touch* touch, Event* event)
{
}

void TitleScene::onTouchMoved(Touch* touch, Event* event)
{
}

void TitleScene::onTouchCancelled(Touch* touch, Event* event)
{
}