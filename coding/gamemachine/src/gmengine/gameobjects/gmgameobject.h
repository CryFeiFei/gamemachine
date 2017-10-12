﻿#ifndef __GAMEOBJECT_H__
#define __GAMEOBJECT_H__
#include <gmcommon.h>
#include <utilities.h>
#include <gmmodel.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMGameObject)
{
	GMuint id = 0;
	GMGameWorld* world = nullptr;
	GMModel* model = nullptr;
	linear_math::Matrix4x4 scaling = linear_math::Matrix4x4::identity();
	linear_math::Matrix4x4 translation = linear_math::Matrix4x4::identity();
	linear_math::Quaternion rotation = linear_math::Quaternion::identity();
	linear_math::Matrix4x4 transformMatrix = linear_math::Matrix4x4::identity();

	bool canDeferredRendering = true;
};

enum class GMGameObjectType
{
	Static,
	Entity, // 可响应实体
	Sprite, // 精灵
	Particles, // 粒子
	Custom,
};

struct GMAsset;
class GMGameObject : public GMObject
{
	DECLARE_PRIVATE(GMGameObject)

public:
	GMGameObject();
	GMGameObject(GMAsset* asset);

public:
	void setModel(GMAsset* asset);
	GMModel* getModel();

	virtual void setWorld(GMGameWorld* world);
	GMGameWorld* getWorld();
	virtual void onAppendingObjectToWorld() {}
	virtual void onRemovingObjectFromWorld() {}
	virtual GMGameObjectType getType() { return GMGameObjectType::Static; }
	virtual void draw();
	virtual void updateAfterSimulate() {}
	virtual void simulate() {}
	virtual bool canDeferredRendering();

private:
	inline void updateMatrix() { D(d); d->transformMatrix = d->scaling * d->rotation.toMatrix() * d->translation; }

public:
	inline void setScaling(const linear_math::Matrix4x4& scaling) { D(d); updateMatrix(); d->scaling = scaling; }
	inline void setTranslate(const linear_math::Matrix4x4& translation) { D(d); updateMatrix(); d->translation = translation; }
	inline void setRotation(const linear_math::Quaternion& rotation) { D(d); updateMatrix(); d->rotation = rotation; }
	inline const linear_math::Matrix4x4& getTransform() { D(d); return d->transformMatrix; }

	// events
private:
	static void onShaderSetBlend(GMObject* sender, GMObject* receiver);
};

//GMEntityObject
enum { EntityPlaneNum = 6 };

GM_PRIVATE_OBJECT(GMEntityObject)
{
	linear_math::Vector3 mins, maxs;
	GMPlane planes[EntityPlaneNum];
};

class GMEntityObject : public GMGameObject
{
	DECLARE_PRIVATE(GMEntityObject)

public:
	GMEntityObject(GMAsset* asset);

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Entity; }

	GMPlane* getPlanes();
	void getBounds(REF linear_math::Vector3& mins, REF linear_math::Vector3& maxs);

private:
	void calc();
	void makePlanes();
};

// GMSkyObject
GM_PRIVATE_OBJECT(GMSkyGameObject)
{
	linear_math::Vector3 min;
	linear_math::Vector3 max;
	Shader shader;
};

class GMSkyGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMSkyGameObject)

public:
	GMSkyGameObject(const Shader& shader, const linear_math::Vector3& min, const linear_math::Vector3& max);
	~GMSkyGameObject();

private:
	void createSkyBox(OUT GMModel** obj);
};

END_NS
#endif