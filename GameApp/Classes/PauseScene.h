#ifndef __PAUSE_SCENE_H__
#define __PAUSE_SCENE_H__

#include "cocos2d.h"

class PauseScene : public cocos2d::Layer
{
public:
	static cocos2d::Scene* createScene();
	virtual bool init();

	// Touch Callbacks
	bool onTouchBegan(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchMoved(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchEnded(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchCancelled(cocos2d::Touch*, cocos2d::Event*) override;

	// Mouse Callbacks
	void onMouseDown(cocos2d::Event*);
	//.....

	void update(float) override;

	CREATE_FUNC(PauseScene);

private:
	float m_WinWidth;
	float m_WinHeight;
};

#endif // __HELLOWORLD_SCENE_H__
