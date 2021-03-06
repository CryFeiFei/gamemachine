﻿#ifndef __PHYSICSOBJECT_H__
#define __PHYSICSOBJECT_H__
#include <gmcommon.h>
#include <map>
#include <linearmath.h>
#include "gmbulletforward.h"
#include <gmassets.h>

BEGIN_NS

GM_ALIGNED_STRUCT(GMPhysicsMoveArgs)
{
	GMPhysicsMoveArgs() = default;
	GMPhysicsMoveArgs(const GMVec3& _lookAt, const GMVec3& _direction, const GMVec3& _speed, const GMVec3& _rate)
		: lookAt(_lookAt)
		, direction(_direction)
		, speed(_speed)
		, rate(_rate)
	{
	}

	GMVec3 lookAt; //!< 物体朝向
	GMVec3 direction; //!< 物体移动方向，坐标系相对于物体朝向，采用左手坐标系，与朝向坐标系一致。
	GMVec3 speed;
	GMVec3 rate;
};

class GMRigidPhysicsObject;
GM_ALIGNED_STRUCT(GMPhysicsRayTestResult)
{
	GMVec3 rayFromWorld = Zero<GMVec3>();
	GMVec3 rayToWorld = Zero<GMVec3>();
	GMVec3 hitPointWorld = Zero<GMVec3>();
	GMVec3 hitNormalWorld = Zero<GMVec3>();
	GMRigidPhysicsObject* hitObject = nullptr;
	bool hit = false;
};

GM_ALIGNED_STRUCT(GMMotionStates)
{
	GMMat4 transform = Identity<GMMat4>();
	GMVec3 linearVelocity = Zero<GMVec3>();
};

GM_PRIVATE_CLASS(GMPhysicsObject);
//! 表示一个物理对象
/*!
  物理对象拥有一些物理性质，如边界大小等。
  \sa GMPhysicsWorld
*/
class GM_EXPORT GMPhysicsObject : public GMObject
{
	GM_DECLARE_PRIVATE(GMPhysicsObject)
	GM_FRIEND_CLASS(GMPhysicsWorld);
	GM_FRIEND_CLASS(GMGameObject);

public:
	GMPhysicsObject();
	~GMPhysicsObject();

public:
	//! 获取物理对象的运动状态。
	/*!
	  物理对象运动状态包括全局变换、线速度等。
	  \return 物理对象的运动状态。
	*/
	virtual const GMMotionStates& getMotionStates();

public:
	void setMotionStates(const GMMotionStates& motionStates);

private:
	//! 将此物理对象与一个GMGameObject绑定起来
	/*!
	  在调用GMGameObject::setPhysicsObject时，此方法将会被GMGameObject调用，因此设置为私有。此对象生命周期由所绑定的GMGameObject管理。
	  \sa GMGameObject::setPhysicsObject()
	*/
	void setGameObject(GMGameObject* gameObject);

public:
	//! 获取所绑定的GMGameObject对象。
	/*!
	  本对象的生命周期由所绑定的GMGameObject对象所管理。
	  \return 本对象所绑定的GMGameObject对象。
	*/
	GMGameObject* getGameObject();
};

enum class GMPhysicsActivationState
{
	ActiveTag = 1,
	IslandSleeping,
	WantsDeactivation,
	DisableDeactivation,
	DisableSimulation,
};

class GMPhysicsShape;

GM_PRIVATE_CLASS(GMRigidPhysicsObject);
class GM_EXPORT GMRigidPhysicsObject : public GMPhysicsObject
{
	GM_DECLARE_PRIVATE(GMRigidPhysicsObject)
	GM_DECLARE_BASE(GMPhysicsObject)

	friend class GMDiscreteDynamicsWorld;

public:
	GMRigidPhysicsObject();
	~GMRigidPhysicsObject();

public:
	//! 将一个物理形状与此物理对象相关联。
	/*!
	  可以用同一个物理形状关联多个物理对象。此方法将构造出物理对象的所有属性，因此，应该在其调用之前设置好所有的物理参数，并且与一个GMGameObject绑定。
	  \param shape GMAssetType为PhysicsShape的物理形状资产。
	  \sa GMAsset, GMGameObject::setPhysicsObject()
	*/
	void setShape(GMAsset shape);
	void setMass(GMfloat mass);
	btRigidBody* getRigidBody();
	GMPhysicsShapeAsset getShape();

	bool isStaticObject() const;
	bool isKinematicObject() const;
	bool isStaticOrKinematicObject() const;
	bool hasContactResponse() const;
	GMPhysicsActivationState getActivationState();
	void setActivationState(GMPhysicsActivationState state, bool force = false);
	void activate(bool force = false);
	GMMat4 getCenterOfMassTransform();
	GMMat4 getCenterOfMassTransformInversed();

public:
	//! 获取物理对象的运动状态。
	/*!
	  物理对象运动状态包括全局变换、线速度等。在本类的实现中，它先会检查运动状态是否更新，如果有更新，则将更新后的值缓存到类的内部，如果没有更新，则返回原值。
	  \return 物理对象的运动状态。
	*/
	virtual const GMMotionStates& getMotionStates() override;

private:
	void initRigidBody(GMfloat mass, const btTransform& startTransform, const GMVec3& color);

private:
	//! 解除对一个bullet刚体的生命周期管理。
	/*!
	  在一个bullet刚体被创建时，它的生命周期由此类的实例管理。此类实例析构时，bullet刚体被释放。<br>
	  但是，如果这个bullet刚体被添加到了物理世界中，则调用此方法解除对此刚体生命周期的管理，此bullet刚体生命周期由物理世界管理。
	  \sa GMDiscreteDynamicsWorld::addRigidObject()
	*/
	void detachRigidBody();
};

END_NS
#endif
