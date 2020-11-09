/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"

USING_NS_CC;

Scene* HelloWorld::createScene()
{
    //支持物理引擎的Scene
    auto scene = Scene::createWithPhysics();
    //显示物理引擎调试界面 --- 有红框
    //scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

    auto layer = HelloWorld::create();

    scene->addChild(layer);

    return scene;

    //return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // 背景
    auto background = DrawNode::create();
    background->drawSolidRect(origin, visibleSize, cocos2d::Color4F(1, 1, 1, 1.0));
    this->addChild(background);
    auto ground = DrawNode::create();
    ground->drawSolidRect(Vec2(0, (visibleSize.height/2-15)), cocos2d::Size(visibleSize.width, 0), cocos2d::Color4F(0,0,0,1.0));
    this->addChild(ground);

    // 碰撞检测
    auto contactListener = EventListenerPhysicsContact::create();
    contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegin, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);


    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    //初始化人物角色
    player_init();

    // 每隔1秒调用一次定时事件
    srand((unsigned int)time(nullptr));
    this->schedule(CC_SCHEDULE_SELECTOR(HelloWorld::add_monster), 1);

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}

void HelloWorld::add_monster(float dt)
{
    if ((rand() % 100) > 70) {
        return;
    }
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    auto monster = Sprite::create("monster.png");
    // Add monster
    monster->setPosition(Vec2(visibleSize.width, visibleSize.height / 2 + origin.y));
    this->addChild(monster);
    auto monst_phy = PhysicsBody::createBox(monster->getContentSize(), PhysicsMaterial(0.0f, 0.0f, 0.0f));
    monst_phy->setDynamic(false);
    monst_phy->setContactTestBitmask(0xFFFFFFFF);
    // play_phy->setTag(10); //设置tag
    //monst_phy->setDynamic(false);
    // monst_phy->setContactTestBitmask(1);
    monster->setPhysicsBody(monst_phy);
    // 定义移动的object
    auto actionMove = MoveTo::create(3, Vec2(0, visibleSize.height / 2 + origin.y));
    // 定义消除的Object。怪物移出屏幕后被消除，释放资源。
    auto actionRemove = RemoveSelf::create();
    monster->runAction(Sequence::create(actionMove, actionRemove, nullptr));
}

/***
 *  碰撞检测
 */
bool HelloWorld::onContactBegin(cocos2d::PhysicsContact& contact)
{
    auto nodeA = contact.getShapeA()->getBody()->getNode();
    auto nodeB = contact.getShapeB()->getBody()->getNode();

    if (nodeA && nodeB) {
        // if (nodeA->getPhysicsBody()->getContactTestBitmask() == 1 && nodeB->getPhysicsBody()->getContactTestBitmask() == 1) {
        if (nodeA->getTag() == 10 || nodeB->getTag() == 10) {
            HWND hwnd = NULL;

            //获取当前窗口句柄
            EnumThreadWindows(
                GetCurrentThreadId(),
                (WNDENUMPROC)[](HWND h, LPARAM l)->BOOL {
                    *(HWND*)l = h;
                    return FALSE;
                },
                (LPARAM)&hwnd
            );
            
            MessageBox(hwnd, L"你被撞倒了", L"Game Over", MB_OK);
            Director::getInstance()->end();
            return true;
        }
    }
    return true;
}

/***
 *  初始化player
 */

bool HelloWorld::player_init()
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    // 添加一个人物精灵
    _player = Sprite::create("player.png");
    if (_player == nullptr) {
        problemLoading("player.png");
        return false;
    }
    else {
        _player->setPosition(Vec2(visibleSize.width * 0.2, visibleSize.height / 2 + origin.y));
        // 碰撞实体
        auto play_phy = PhysicsBody::createBox(_player->getContentSize(), PhysicsMaterial(0.0f, 0.0f, 0.0f));
        // play_phy->setContactTestBitmask(1);
        play_phy->setDynamic(false);
        play_phy->setContactTestBitmask(0xFFFFFFFF);
        _player->setTag(10); //设置tag
        _player->setPhysicsBody(play_phy);
        _player_jump = false;
        this->addChild(_player, 1);
    }

    // 添加触摸事件
    auto listener1 = EventListenerTouchOneByOne::create();
    listener1->onTouchBegan = [&](Touch* touch, Event* event) {
        if (_player_jump) {
            return false;
        }
        _player_jump = true;
        auto callfunction = [&]() {
            _player_jump = false;
        };
        auto jump = JumpBy::create(0.5f, Vec2(0, 0), 100, 1.0f);
        CallFunc* callFunc = CallFunc::create(callfunction);
        _player->runAction(Sequence::create(jump, callFunc, nullptr));
        return true;
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener1, this);

    return true;
}
