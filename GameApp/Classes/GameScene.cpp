#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "AppDelegate.h"

#include "PauseScene.h"
#include "GameOverScene.h"

USING_NS_CC;

#define SK_DEGREES_TO_RADIANS(__ANGLE__) ((__ANGLE__) * 0.01745329252f) // PI / 180
#define SK_RADIANS_TO_DEGREES(__ANGLE__) ((__ANGLE__) * 57.29577951f) // PI * 180

static const int32_t playerProjectileLayer = 0x1 << 0;
static const int32_t monsterLayer = 0x1 << 1;
static const int32_t asteroid_layer = 0x1 << 2;
static const int32_t playerLayer = 0x1 << 4;

static const int32_t contact_bitmask = 0xffffffff;

const float MaxPlayerAccel = 400.0f;
const float MaxPlayerSpeed = 200.0f;

const float SHIP_SLOW_SPEED = 0.75f;
const float SHIP_NORMAL_SPEED = 1.8f;
const float SHIP_FAST_SPEED = 3.2f;

Scene* GameScene::createScene()
{
	auto scene = Scene::createWithPhysics();
	auto layer = GameScene::create();

	scene->addChild(layer);

	return scene;
}

bool GameScene::init()
{
	if (!Layer::init())
	{
		return false;
	}

	// Adds contact event listener
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(GameScene::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);

	this->createGameObjects();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	
	// Set up mouse events
	auto listener = EventListenerMouse::create();
	listener->onMouseDown = CC_CALLBACK_1(GameScene::onMouseDown, this);
	listener->onMouseMove = CC_CALLBACK_1(GameScene::onMouseMove, this);
	listener->onMouseUp = CC_CALLBACK_1(GameScene::onMouseUp, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
	
	// Setup Keyboard for pc port
	auto klistener = EventListenerKeyboard::create();
	klistener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
	klistener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(klistener, this);

#else
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(GameScene::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(GameScene::onTouchEnded, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(GameScene::onTouchMoved, this);
	touchListener->onTouchCancelled = CC_CALLBACK_2(GameScene::onTouchCancelled, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	Device::setAccelerometerEnabled(true);
	auto listener = EventListenerAcceleration::create(CC_CALLBACK_2(
		GameScene::onAcceleration, this));

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif

	this->scheduleUpdate();
	return true;
}

// ---------- Initialisation -----------------
void GameScene::createGameObjects()
{
	auto dir = Director::getInstance();
	auto winsz = dir->getWinSize();
	float scale_factor = dir->getContentScaleFactor();

	// Set Variables
	m_WinWidth = winsz.width;
	m_WinHeight = winsz.height;
	m_LevelBounds = Rect(-5000, -5000, 10000, 10000);
	m_LeftKey = m_RightKey = m_MouseDown = false;
	m_PlayerAngle = 0;

	// ---- Create UI Text Labels ----
	m_DebugLabel = Label::createWithTTF("", "fonts/arcade.ttf", 24 / scale_factor);
	m_MoneyLabel = Label::createWithTTF("Money " + AppDelegate::ToString(m_Money), "fonts/arcade.ttf", 24 / scale_factor);
	m_RenownLabel = Label::createWithTTF("Renown " + AppDelegate::ToString(m_Renown), "fonts/arcade.ttf", 24 / scale_factor);


	m_DebugLabel->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_DebugLabel->setPosition(32, 12 * scale_factor);

	m_MoneyLabel->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_MoneyLabel->setPosition(32, m_WinHeight - m_MoneyLabel->getBoundingBox().size.height - (12 * scale_factor));
	m_MoneyLabel->setColor(Color3B(255, 0, 0)); 

	m_RenownLabel->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_RenownLabel->setPosition(32, m_WinHeight - m_MoneyLabel->getBoundingBox().size.height - (12 * scale_factor) - m_RenownLabel->getBoundingBox().size.height);
	m_RenownLabel->setColor(Color3B(255, 0, 0));
 
	// ---- Create Background ----
	m_Background = Sprite::create("background.jpg");
	m_Background->setPosition(winsz.width/2, winsz.height/2);
	m_Background->setScale(1);
	m_Background->setTextureRect(Rect(0, 0, winsz.width, winsz.height));
	
	cocos2d::Texture2D::TexParams tp;
	tp.minFilter = GL_LINEAR;
	tp.magFilter = GL_LINEAR;
	tp.wrapS = GL_REPEAT;
	tp.wrapT = GL_REPEAT;
	m_Background->getTexture()->setTexParameters(tp);
	m_Background->setAnchorPoint(cocos2d::Vec2(0.5, 0.5));

	// ---- Player ----
	m_Player = Sprite::create("spaceship.png");
	m_Player->setAnchorPoint(cocos2d::Vec2(0.5, 0.5));
	m_Player->setPosition(winsz.width / 2, winsz.height / 2);
	m_Player->setScale(0.5f);
	m_Player->setTag(playerLayer);
	
	auto physicsBody = PhysicsBody::createBox(Size(54, 54), PhysicsMaterial(0.1f, 1.0f, 0.0f));
	physicsBody->setDynamic(false);
	physicsBody->setContactTestBitmask(contact_bitmask);
	physicsBody->setCategoryBitmask(playerLayer);
	physicsBody->setCollisionBitmask(monsterLayer | asteroid_layer);
	m_Player->addComponent(physicsBody);

	// ---- Create other nodes ----
	m_WorldNode = Node::create();
	this->addChild(m_WorldNode);

	m_CameraNode = Node::create();
	m_CameraNode->setAnchorPoint(Vec2(0.5f, 0.5f));
	m_WorldNode->addChild(m_CameraNode);

	// First BG
	m_CameraNode->addChild(m_Background);

	// Next stars
	m_Stars.resize(50);
	for (unsigned i = 0; i < 50; ++i)
	{
		auto star = Sprite::create("star.png");
		star->setPosition(Vec2(cocos2d::random(0.0f, m_WinWidth), cocos2d::random(0.0f, m_WinHeight)));
		m_CameraNode->addChild(star);
		m_Stars[i] = star;
	}

	generateAsteroids();
	
	// Then player, and finally UI
	m_CameraNode->addChild(m_Player);
	m_CameraNode->addChild(m_DebugLabel, 1);
	m_CameraNode->addChild(m_MoneyLabel, 1);
	m_CameraNode->addChild(m_RenownLabel, 1);

	// ---- UI ----
	auto left = Sprite::create("left_arrow.jpg");
	auto left_size = left->getTexture()->getContentSize();
	left->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_CameraNode->addChild(left);

	m_LeftKeyBounds = Rect(16, 32, left_size.width, left_size.height);
	left->setPosition(m_LeftKeyBounds.getMinX(), m_LeftKeyBounds.getMinY());

	auto right = Sprite::create("right_arrow.jpg");
	right->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_CameraNode->addChild(right);
	m_RightKeyBounds = Rect(16 + 4 + left_size.width, 32, left_size.width, left_size.height);
	right->setPosition(m_RightKeyBounds.getMinX(), m_RightKeyBounds.getMinY());

	auto pause = Sprite::create("pause.png");
	auto pause_size = pause->getTexture()->getContentSizeInPixels();
	pause->setScale(0.5f);
	pause->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	m_CameraNode->addChild(pause);
	m_PauseKeyBounds = Rect(m_WinWidth - (pause_size.width * 0.5f) - 4 , m_WinHeight - (pause_size.height * 0.5f) - 4, pause_size.width * 0.5f, pause_size.height * 0.5f);
	pause->setPosition(m_PauseKeyBounds.getMinX(), m_PauseKeyBounds.getMinY());
}

void GameScene::generateAsteroids()
{
	std::vector<Sprite*> asteroids;

	float minX = m_WinWidth;
	float maxX = m_LevelBounds.size.width - m_WinWidth;
	float minY = m_WinHeight;
	float maxY = m_LevelBounds.size.height - m_WinHeight;

	// TODO : have some other calculation for how many should produce
	for (int i = 0; i < 90; ++i)
	{
		bool validPosition = false;

		while (!validPosition)
		{
			float randX = cocos2d::RandomHelper::random_int((int)minX, (int)maxX);
			float randY = cocos2d::RandomHelper::random_int((int)minY, (int)maxY);
			randX -= (m_LevelBounds.size.width * 0.5f);
			randY -= (m_LevelBounds.size.height * 0.5f);

			if (asteroids.empty())
			{
				asteroids.push_back(createAsteroid(randX, randY));
				validPosition = true;
				break;
			}

			bool free_space = true;
			for (size_t j = 0; j < asteroids.size(); ++j)
			{
				if (asteroids[j]->getBoundingBox().containsPoint(Vec2(randX, randY)))
				{
					free_space = false;
					//cocos2d::log("Invalid POSITION");
					break;
				}
			}

			if (free_space)
			{
				//cocos2d::log(cocos2d::StringUtils::format("Created Asteroid : %f,%f", randX, randY).c_str());
				validPosition = true;
				asteroids.push_back(createAsteroid(randX, randY));
				break;
			}
		}
	}
}

cocos2d::Sprite* GameScene::createAsteroid(float x, float y)
{
	Sprite* asteroid = Sprite::create("asteroid.png");
	asteroid->setAnchorPoint(cocos2d::Vec2(0.5, 0.5));
	asteroid->setPosition(x, y);
	asteroid->setScale(1.0f);
	asteroid->setTag(asteroid_layer);

	auto physicsBody = PhysicsBody::createBox(Size(90, 90), PhysicsMaterial(0.1f, 1.0f, 0.0f));
	physicsBody->setDynamic(false);

	physicsBody->setCategoryBitmask(asteroid_layer);
	physicsBody->setCollisionBitmask(playerProjectileLayer | playerLayer);
	physicsBody->setContactTestBitmask(contact_bitmask);
	asteroid->addComponent(physicsBody);

	m_WorldNode->addChild(asteroid);
	return asteroid;
}

void GameScene::createProjectile()
{
	auto director = Director::getInstance();
	float sf = director->getContentScaleFactor();

	// Set up initial location of projectile
	Sprite* projectile = Sprite::create("projectile.png", Rect(0, 0, 20 / sf, 20 / sf));
	projectile->setPosition(Vec2(m_Player->getPositionX(), m_Player->getPositionY()));
	projectile->setTag(playerProjectileLayer);

	auto physicsBody = PhysicsBody::createBox(Size(20, 20), PhysicsMaterial(0.1f, 1.0f, 0.0f));
	physicsBody->setDynamic(true);
	
	physicsBody->setCategoryBitmask(playerProjectileLayer);
	physicsBody->setCollisionBitmask(monsterLayer | asteroid_layer);
	physicsBody->setContactTestBitmask(contact_bitmask);
	projectile->addComponent(physicsBody);

	m_CameraNode->addChild(projectile);

	float realMoveDuration = 0.75f;

	// Get Direction of space ship and normalize
	float angle = SK_DEGREES_TO_RADIANS(m_Player->getRotation());
	Vec2 dir(sinf(angle), cosf(angle));
	dir.normalize();

	// Move projectile to actual endpoint
	projectile->runAction(CCSequence::create(
		MoveBy::create(realMoveDuration, dir * 800),
		CallFuncN::create(this,
			callfuncN_selector(GameScene::spriteMoveFinished)),NULL));
}

void GameScene::createRandomEnemy(float x, float y)
{
	//135x264 -- 3x4
	auto director = Director::getInstance();
	float sf = director->getContentScaleFactor();

	float frame_width =  (135.0f / 3.0f) / sf;
	float frame_height = (264.0f / 4.0f) / sf;

	auto monster = Sprite::create("monster2.png", Rect(0,0, frame_width, frame_height));
	monster->setAnchorPoint(cocos2d::Vec2(0.5, 0.5));
	monster->setPosition(x, y);
	monster->setScale(1.0f);
	monster->setColor(Color3B(255, 255, 255));
	monster->setTag(monsterLayer);

	auto physicsBody = PhysicsBody::createBox(Size(64, 64),
		PhysicsMaterial(0.1f, 1.0f, 0.0f));
	physicsBody->setDynamic(!false);
	physicsBody->setContactTestBitmask(contact_bitmask);
	physicsBody->setCategoryBitmask(monsterLayer);
	physicsBody->setCollisionBitmask(playerProjectileLayer | playerLayer);
	physicsBody->setGravityEnable(false);

	monster->addComponent(physicsBody);

	// Animate
	Vector<SpriteFrame*> animFrames;
	animFrames.reserve(3);
	// Left facing
	animFrames.pushBack(SpriteFrame::create("monster2.png", Rect(0, 0, frame_width, frame_height)));
	animFrames.pushBack(SpriteFrame::create("monster2.png", Rect(1*frame_width, 0, frame_width, frame_height)));
	animFrames.pushBack(SpriteFrame::create("monster2.png", Rect(2*frame_width, 0, frame_width, frame_height)));

	// create the animation out of the frames
	Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
	Animate* animate = Animate::create(animation);
	monster->runAction(RepeatForever::create(animate));

	// create a MoveBy Action to where we want the sprite to drop from.
	auto move = MoveBy::create(5, Vec2(-m_WinWidth, 0.0f));
	auto move_back = move->reverse();

	// create a delay that is run in between sequence events
	auto delay = DelayTime::create(0.5f);

	// create the sequence of actions, in the order we want to run them
	auto seq1 = Sequence::create(move, delay, move_back, delay->clone(), nullptr);

	// run the sequence and repeat forever.
	monster->runAction(RepeatForever::create(seq1));

	this->addChild(monster);
}


void GameScene::update(float delta)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	
	// Rotate Right
	if (m_RightKey || (m_MouseDown && m_RightKeyBounds.containsPoint(m_FirstTouch)))
	{
		m_Player->setRotation(m_PlayerAngle + 4.0f);
		m_PlayerAngle = m_Player->getRotation();
	}

	// Rotate Left
	else if (m_LeftKey || (m_MouseDown && m_LeftKeyBounds.containsPoint(m_FirstTouch)))
	{
		m_Player->setRotation(m_PlayerAngle - 4.0f);
		m_PlayerAngle = m_Player->getRotation();
	}

	if (m_MouseDown && m_PauseKeyBounds.containsPoint(m_FirstTouch))
	{
		m_MouseDown = false;
		auto d = Director::getInstance();
		auto scene = PauseScene::createScene();
		d->pushScene(scene);
	}

	// Control Speed
	if (m_BrakesKeyDown)
	{
		m_ShipVelocity = SHIP_SLOW_SPEED;
	}
	else if(m_FastKeyDown)
	{
		m_ShipVelocity = SHIP_FAST_SPEED;
	}
	else
	{
		m_ShipVelocity = SHIP_NORMAL_SPEED;
	}

#else
	// Rotate Right
	if (m_RightKey)
	{
		m_Player->setRotation(m_PlayerAngle + 4.0f);
		m_PlayerAngle = m_Player->getRotation();
	}

	// Rotate Left
	else if (m_LeftKey)
	{
		m_Player->setRotation(m_PlayerAngle - 4.0f);
		m_PlayerAngle = m_Player->getRotation();
	}

	// Control speed through accelerometer
	const float FilteringFactor = 0.75f;
	float ax = (m_AccelX * FilteringFactor + m_AccelX * (1.0f - FilteringFactor));
	//float ay = (m_AccelY * FilteringFactor + m_AccelY * (1.0f - FilteringFactor));
	
	/*
	// Did the angle flip from +Pi to -Pi, or -Pi to +Pi?
	if (ax > 3.0f)
	{
		m_ShipVelocity = SHIP_FAST_SPEED;
	}
	else if (ax < 3.0f)
	{
		m_ShipVelocity = SHIP_SLOW_SPEED;
	}
	else
	{
		m_ShipVelocity = SHIP_NORMAL_SPEED;
	}
	*/

	m_ShipVelocity = SHIP_NORMAL_SPEED;

#endif

	if (m_InBattle)
	{
		m_EnemySpawnTimer += delta;
		if (m_EnemySpawnTimer >= 3.5f)
		{
			if (m_NumEnemiesSpawned < 60)
			{
				for (int i = 0; i < 5; ++i)
				{
					// Gen Random
					float minY = m_CameraNode->getPositionY() - (m_WinHeight + 32);
					float maxY = m_CameraNode->getPositionY() + (m_WinHeight - 32);
					float actualY = cocos2d::random(minY, maxY);

					this->createRandomEnemy(m_CameraNode->getPositionX() + m_WinWidth, actualY);

					if (++m_NumEnemiesSpawned >= 60)
						break;
				}
			}

			m_EnemySpawnTimer = 0.0f;
		}
	}

	// --- Move Camera in direction of player ----
	float angle = SK_DEGREES_TO_RADIANS(m_Player->getRotation());
	Vec2 dir(sinf(angle), cosf(angle));
	dir.normalize();
	m_CameraNode->setPosition(m_CameraNode->getPosition() + (dir * m_ShipVelocity));

	if (m_CameraNode->getPositionX() < m_LevelBounds.getMinX())
	{
		m_CameraNode->setPositionX(m_LevelBounds.getMaxX());
	}
	else if (m_CameraNode->getPositionX() >= m_LevelBounds.getMaxX())
	{
		m_CameraNode->setPositionX(m_LevelBounds.getMinX());
	}

	if (m_CameraNode->getPositionY() < m_LevelBounds.getMinY())
	{
		m_CameraNode->setPositionY(m_LevelBounds.getMaxY());
	}
	else if (m_CameraNode->getPositionY() >= m_LevelBounds.getMaxY())
	{
		m_CameraNode->setPositionY(m_LevelBounds.getMinY());
	}

	m_DebugLabel->setString(cocos2d::StringUtils::format("Speed %f, Num Allocs %d", m_ShipVelocity, (int)this->getChildrenCount() + (int)m_Background->getChildrenCount() + (int)m_WorldNode->getChildrenCount()));

	// In SK this is the centre on camera bit
	auto cam = Camera::getDefaultCamera();
	cam->setPosition(m_CameraNode->getPosition() + Vec2(m_WinWidth * 0.5f, m_WinHeight * 0.5f));

	// Move Stars in opposite direction to player
	for (auto i = m_Stars.begin(); i != m_Stars.end(); ++i)
	{
		(*i)->setPosition((*i)->getPosition() - (dir * m_ShipVelocity));

		if ((*i)->getPositionX() < 0.0f)
		{
			(*i)->setPositionX(m_WinWidth);
		}
		else if ((*i)->getPositionX() > m_WinWidth)
		{
			(*i)->setPositionX(0.0f);
		}

		if ((*i)->getPositionY() < 0.0f)
		{
			(*i)->setPositionY(m_WinHeight);
		}
		else if ((*i)->getPositionY() > m_WinHeight)
		{
			(*i)->setPositionY(0.0f);
		}
	}
}


// ---------- Collision -----------------
bool GameScene::onContactBegin(cocos2d::PhysicsContact& contact)
{
	PhysicsBody* firstBody;
	PhysicsBody* secondBody;

	bool dead = false;

	if (contact.getShapeA()->getBody()->getCategoryBitmask() < contact.getShapeB()->getBody()->getCategoryBitmask())
	{
		firstBody = contact.getShapeA()->getBody();
		secondBody = contact.getShapeB()->getBody();
	}
	else
	{
		firstBody = contact.getShapeB()->getBody();
		secondBody = contact.getShapeA()->getBody();
	}

	// Check for projectile Collisions
	if ((firstBody->getCategoryBitmask() & playerProjectileLayer) != 0 &&
	   (secondBody->getCategoryBitmask() & monsterLayer) != 0)
	{
		projectileHit(firstBody->getNode(), secondBody->getNode());
		--m_NumEnemiesSpawned;
		m_RenownLabel->setString("Renown " + AppDelegate::ToString(++m_Renown));
	} 
	else if ((firstBody->getCategoryBitmask() & playerProjectileLayer) != 0 &&
		(secondBody->getCategoryBitmask() & asteroid_layer) != 0)
	{
		projectileHit(firstBody->getNode(), secondBody->getNode());
		m_MoneyLabel->setString("Money " + AppDelegate::ToString(++m_Money));
	}
	// Player Hit monster
	else if ((firstBody->getCategoryBitmask() & monsterLayer) != 0 && (secondBody->getCategoryBitmask() & playerLayer) != 0)
	{
		dead = true;
	}
	else if ((firstBody->getCategoryBitmask() & asteroid_layer) != 0 && (secondBody->getCategoryBitmask() & playerLayer) != 0)
	{
		dead = true;
	}

	if (dead)
	{
		auto director = Director::getInstance();
		auto scene = GameOverScene::createScene();
		director->replaceScene(scene);
	}
	
	return true;
}

void GameScene::projectileHit(cocos2d::Node* s1, cocos2d::Node* s2)
{
	if(s1)
		s1->removeFromParentAndCleanup(true);
	
	if(s2)
		s2->removeFromParentAndCleanup(true);
}

void GameScene::spriteMoveFinished(cocos2d::Node* sender)
{
	Sprite *sprite = (Sprite *)sender;
	if (sprite)
	{
		if(sprite->getParent())
			sprite->getParent()->removeChild(sprite, true);
	}
}


// ---------- Mouse -----------------
void GameScene::onMouseDown(cocos2d::Event* ev)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
	EventMouse* mouseEvent = (EventMouse*)(ev);
	//mouseEvent->getMouseButton();
	
	m_FirstTouch = cocos2d::Vec2(mouseEvent->getCursorX(), mouseEvent->getCursorY());
	m_MouseDown = true;
#endif
}

void GameScene::onMouseMove(cocos2d::Event* ev)
{
	/*
	if (m_MouseDown)
	{
		EventMouse* mouseEvent = (EventMouse*)(ev);
		cocos2d::Vec2 pos(mouseEvent->getCursorX(), mouseEvent->getCursorY());

		Vec2 dist(m_FirstTouch.x - m_Player->getPositionX(), m_FirstTouch.y - m_Player->getPositionY());
		Vec2 newpos(pos.x - dist.x, pos.y - dist.y);
		m_FirstTouch = pos;

		auto sz = m_Player->getSpriteFrame()->getOriginalSizeInPixels() * m_Player->getScale();
		auto dir = Director::getInstance();
		auto winsz = dir->getWinSize();

		float plr_spr_w = sz.width * 0.5;
		float plr_spr_h = sz.height * 0.5;
		float view_w = winsz.width;
		float view_h = winsz.height;

		// Cap
		if (newpos.x <= (plr_spr_w))
		{
			newpos.x = (plr_spr_w);
		}
		else if (newpos.x >= (view_w - plr_spr_w))
		{
			newpos.x = (view_w - plr_spr_w);
		}

		if (newpos.y <= (plr_spr_h))
		{
			newpos.y = (plr_spr_h);
		}
		else if (newpos.y >= (view_h - plr_spr_h))
		{
			newpos.y = (view_h - plr_spr_h);
		}

		// Finally set the relative capped position
		m_Player->setPosition(newpos);
	}
	*/
}

void GameScene::onMouseUp(cocos2d::Event* ev)
{
	m_MouseDown = false;
}


// ---------- Key Board -----------------
void GameScene::onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW || keyCode == EventKeyboard::KeyCode::KEY_D)
	{
		m_RightKey = true;
	}
	
	if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW || keyCode == EventKeyboard::KeyCode::KEY_A)
	{
		m_LeftKey = true;
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_SPACE)
	{
		createProjectile();
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_W)
	{
		m_FastKeyDown = true;
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_S)
	{
		m_BrakesKeyDown = true;
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_B)
	{
		m_InBattle = !m_InBattle;
	}

	// Pause
	if (keyCode == EventKeyboard::KeyCode::KEY_ENTER )
	{
		auto d = Director::getInstance();
		auto scene = PauseScene::createScene();
		d->pushScene(scene);
	}

	// TODO : Esc = quit

}

void GameScene::onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_RIGHT_ARROW || keyCode == EventKeyboard::KeyCode::KEY_D)
	{
		m_RightKey = false;
	}
	
	if (keyCode == EventKeyboard::KeyCode::KEY_LEFT_ARROW || keyCode == EventKeyboard::KeyCode::KEY_A)
	{
		m_LeftKey = false;
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_W)
	{
		m_FastKeyDown = false;
	}

	if (keyCode == EventKeyboard::KeyCode::KEY_S)
	{
		m_BrakesKeyDown = false;
	}
}


// ---------- Touch Screen Phone -----------------
bool GameScene::onTouchBegan(Touch* touch, Event* event)
{
	m_FirstTouch = touch->getLocation();
	
	//m_MouseDown = true;
	//m_ShipVelocity = SHIP_SLOW_SPEED;
	
	//if(!m_LeftKeyBounds.containsPoint(m_FirstTouch) && !m_RightKeyBounds.containsPoint(m_FirstTouch))
	//	this->createProjectile();

	if (m_LeftKeyBounds.containsPoint(m_FirstTouch))
	{
		m_LeftKey = true;
	}
	else if (m_RightKeyBounds.containsPoint(m_FirstTouch))
	{
		m_RightKey = true;
	}
	else if (m_PauseKeyBounds.containsPoint(m_FirstTouch))
	{
		m_LeftKey = m_RightKey = false;

		// Pause
		auto d = Director::getInstance();
		auto scene = PauseScene::createScene();
		d->pushScene(scene);
	}
	else
	{
		this->createProjectile();
	}
	
	return true;
}

void GameScene::onTouchEnded(Touch* touch, Event* event)
{
	m_LeftKey = m_RightKey = false;
	//m_MouseDown = false;
	//m_ShipVelocity = SHIP_NORMAL_SPEED;
}

void GameScene::onTouchMoved(Touch* touch, Event* event)
{
	/*
	cocos2d::Vec2 pos = touch->getLocation();

	Vec2 dist(m_FirstTouch.x - m_Player->getPositionX(), m_FirstTouch.y - m_Player->getPositionY());
	Vec2 newpos(pos.x - dist.x, pos.y - dist.y);
	m_FirstTouch = pos;

	auto sz = m_Player->getSpriteFrame()->getOriginalSizeInPixels() * m_Player->getScale();
	auto dir = Director::getInstance();
	auto winsz = dir->getWinSize();

	float plr_spr_w = sz.width * 0.5;
	float plr_spr_h = sz.height * 0.5;
	float view_w = winsz.width;
	float view_h = winsz.height;

	// Cap
	if (newpos.x <= (plr_spr_w))
	{
		newpos.x = (plr_spr_w);
	}
	else if (newpos.x >= (view_w - plr_spr_w))
	{
		newpos.x = (view_w - plr_spr_w);
	}

	if (newpos.y <= (plr_spr_h))
	{
		newpos.y = (plr_spr_h);
	}
	else if (newpos.y >= (view_h - plr_spr_h))
	{
		newpos.y = (view_h - plr_spr_h);
	}

	// Finally set the relative capped position
	m_Player->setPosition(newpos);
	*/
}

void GameScene::onTouchCancelled(Touch* touch, Event* event)
{
	cocos2d::log("touch cancelled");
}

void GameScene::onAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* ev)
{
	m_AccelX = acc->y;
	m_AccelY = acc->x;
}



















// Only if we are on phone
/*
const float FilteringFactor = 0.75f;
float ax = (m_AccelX * FilteringFactor + m_AccelX * (1.0f - FilteringFactor));
float ay = (m_AccelY * FilteringFactor + m_AccelY * (1.0f - FilteringFactor));

float min_angle = 0.05f;

if (ay > min_angle)
{
m_PlayerAccelX = -MaxPlayerAccel;
}
else if (ay < -min_angle)
{
m_PlayerAccelX = MaxPlayerAccel;
}
if (ax < -min_angle)
{
m_PlayerAccelY = -MaxPlayerAccel;
}
else if (ax > min_angle)
{
m_PlayerAccelY = MaxPlayerAccel;
}

m_PlayerSpeedX += m_PlayerAccelX * delta;
m_PlayerSpeedY += m_PlayerAccelY * delta;

m_PlayerSpeedX = fmaxf(fminf(m_PlayerSpeedX, MaxPlayerSpeed), -MaxPlayerSpeed);
m_PlayerSpeedY = fmaxf(fminf(m_PlayerSpeedY, MaxPlayerSpeed), -MaxPlayerSpeed);

float newX = m_Player->getPositionX() + m_PlayerSpeedX * delta;
float newY = m_Player->getPositionY() + m_PlayerSpeedY * delta;
//newX = fminf(m_WinWidth, fmaxf(newX, 0));
//newY = fminf(m_WinHeight, fmaxf(newY, 0));

//m_Player->setPosition(newX, newY);

if (m_PlayerSpeedX < -MaxPlayerSpeed)
{
m_PlayerSpeedX = -MaxPlayerSpeed;
}
else if (m_PlayerSpeedX > MaxPlayerSpeed)
{
m_PlayerSpeedX = MaxPlayerSpeed;
}

float speed = sqrtf(m_PlayerSpeedX * m_PlayerSpeedX + m_PlayerSpeedY * m_PlayerSpeedY);
if (speed > 40.0f)
{
float angle = atan2f(m_PlayerSpeedY, m_PlayerSpeedX);

// Did the angle flip from +Pi to -Pi, or -Pi to +Pi?
if (m_LastAngle < -3.0f && angle > 3.0f)
{
m_PlayerAngle += M_PI * 2.0f;
}
else if (m_LastAngle > 3.0f && angle < -3.0f)
{
m_PlayerAngle -= M_PI * 2.0f;
}

m_LastAngle = angle;
const float RotationBlendFactor = 0.2f;
m_PlayerAngle = angle * RotationBlendFactor + m_PlayerAngle * (1.0f - RotationBlendFactor);
}

m_Player->setRotation(SK_RADIANS_TO_DEGREES(m_PlayerAngle) - 45.0f);

m_ProjectileTimer += delta;
if (m_ProjectileTimer > 0.7f)
{
m_ProjectileTimer = 0.0f;
//this->createProjectile();
}
*/