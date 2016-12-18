#ifndef __TITLE_SCENE_H__
#define __TITLE_SCENE_H__

#include "cocos2d.h"

class TitleScene : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
    virtual bool init();
	void update(float) override;

	// Touch Callbacks
	bool onTouchBegan(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchMoved(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchEnded(cocos2d::Touch*, cocos2d::Event*)override;
	void onTouchCancelled(cocos2d::Touch*, cocos2d::Event*) override;
    
	// Mouse Callbacks
	void onMouseDown(cocos2d::Event*);
	
	CREATE_FUNC(TitleScene);

private:
	std::vector<cocos2d::Sprite*> m_Stars;
	float m_WinWidth;
	float m_WinHeight;
};

#endif // __TITLE_SCENE_H__
