#ifndef __GAMEWORLD_SCENE_H__
#define __GAMEWORLD_SCENE_H__

#include "cocos2d.h"

class GameScene : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init();
	void update(float) override;
	bool onContactBegin(cocos2d::PhysicsContact& contact);

	// Game Specific
	cocos2d::Sprite* createAsteroid(float x, float y);
	void createGameObjects();
	void createProjectile();
	void createRandomEnemy(float x, float y);
	void projectileHit(cocos2d::Node* s1, cocos2d::Node* s2);
	void generateAsteroids();

	void spriteMoveFinished(cocos2d::Node*);

	// Touch Callbacks
	bool onTouchBegan(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchMoved(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchEnded(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchCancelled(cocos2d::Touch*, cocos2d::Event*) override;

	// Mouse Callbacks
	void onMouseDown(cocos2d::Event*);
	void onMouseMove(cocos2d::Event*);
	void onMouseUp(cocos2d::Event*);

	// Accelerometer
	void onAcceleration(cocos2d::Acceleration* acc, cocos2d::Event* ev);
	
	// Key Callbacks
	void onKeyPressed(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* e) override;
	void onKeyReleased(cocos2d::EventKeyboard::KeyCode keyCode, cocos2d::Event* e)override;

	CREATE_FUNC(GameScene);

private:
	cocos2d::Node* m_WorldNode;
	cocos2d::Node* m_CameraNode;
	cocos2d::Sprite* m_Player;
	cocos2d::Sprite* m_Background;

	cocos2d::Label* m_DebugLabel;
	cocos2d::Label* m_RenownLabel;
	cocos2d::Label* m_MoneyLabel;


	// Player stuff
	cocos2d::Vec2 m_FirstTouch;
	float m_PlayerAngle = 0.0f;

	float m_PlayerAccelX;
	float m_PlayerAccelY;
	float m_PlayerSpeedX;
	float m_PlayerSpeedY;
	float m_LastAngle;
	float m_AccelX;
	float m_AccelY;

	bool m_RightKey = false;
	bool m_LeftKey = false;
	bool m_MouseDown = false;
	bool m_BrakesKeyDown = false;
	bool m_FastKeyDown = false;

	float m_WinWidth;
	float m_WinHeight;
	float m_ShipVelocity = 1.8f;

	bool m_InBattle = true;
	float m_ProjectileTimer = 0.0f;	
	float m_EnemySpawnTimer = 0.0f;
	int m_NumEnemiesSpawned = 0;
	
	int m_Renown = 0;
	int m_Money = 0;

	cocos2d::Rect m_LevelBounds;
	cocos2d::Rect m_LeftKeyBounds;
	cocos2d::Rect m_RightKeyBounds;
	cocos2d::Rect m_PauseKeyBounds;

	std::vector<cocos2d::Sprite*> m_Stars;
};

#endif // __HELLOWORLD_SCENE_H__

