#pragma once


class GameObject;
class Component
{
public:
  Component() = default;
  virtual ~Component() = default;


  virtual void init() {}
  virtual void lateInit() {}
  virtual void update(float delta) {} 
  virtual void render() {}
  virtual void shutdown() noexcept {} 

  const GameObject& getGameObject() const noexcept { return *mGameObject; }
  GameObject& getGameObject()  noexcept { return *mGameObject; }
  void setGameObject(GameObject* go)  noexcept { mGameObject = go; }
protected:
  GameObject* mGameObject;
};