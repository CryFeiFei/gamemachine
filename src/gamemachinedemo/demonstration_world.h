﻿#ifndef __DEMOSTRATION_H__
#define __DEMOSTRATION_H__

#include <gamemachine.h>
#include <gm2dgameobject.h>
#include <gmgl.h>
#include <gmdemogameworld.h>

class DemostrationWorld;
GM_PRIVATE_OBJECT(DemoHandler)
{
	gm::GMRenderConfig renderConfig;
	gm::GMDebugConfig debugConfig;
	DemostrationWorld* parentDemonstrationWorld = nullptr;
	bool inited = false;
	bool activating = false;
	gm::GMGameWorld* demoWorld = nullptr;
};

class DemoHandler : public gm::GMObject
{
	DECLARE_PRIVATE(DemoHandler)

public:
	DemoHandler(DemostrationWorld* parentDemonstrationWorld);
	~DemoHandler();

public:
	virtual void init();
	virtual bool isInited();
	virtual void onActivate();
	virtual void onDeactivate();
	virtual void event(gm::GameMachineEvent evt);

protected:
	virtual void setLookAt();
	virtual void setDefaultLights();

protected:
	void backToEntrance();
	bool isActivating();
	void switchNormal();

	inline gm::GMGameWorld*& getDemoWorldReference()
	{
		D(d);
		return d->demoWorld;
	}
};

typedef Pair<gm::GMString, DemoHandler*> GameHandlerItem;
typedef Vector<GameHandlerItem> DemoHandlers;

GM_PRIVATE_OBJECT(DemostrationWorld)
{
	DemoHandlers demos;
	DemoHandler* currentDemo = nullptr;
	DemoHandler* nextDemo = nullptr;
};

class DemostrationWorld : public gm::GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(DemostrationWorld, gm::GMGameWorld)

public:
	DemostrationWorld() = default;
	~DemostrationWorld();

public:
	inline DemoHandler* getCurrentDemo() { D(d); return d->currentDemo; }
	void setCurrentDemo(DemoHandler* demo) { D(d); d->currentDemo = demo; }
	void setCurrentDemo(gm::GMint index) { D(d); d->currentDemo = d->demos[0].second; }

public:
	void addDemo(const gm::GMString& name, AUTORELEASE DemoHandler* demo);
	void init();
	void renderScene();
	void switchDemo();
	void resetProjectionAndEye();
};

GM_PRIVATE_OBJECT(DemostrationEntrance)
{
	DemostrationWorld* world = nullptr;
	gm::GMDebugConfig debugConfig;
	gm::GMRenderConfig renderConfig;
};

class DemostrationEntrance : public gm::IGameHandler, public gm::IShaderLoadCallback
{
	DECLARE_PRIVATE_NGO(DemostrationEntrance)

public:
	DemostrationEntrance() = default;
	~DemostrationEntrance();

public:
	inline DemostrationWorld* getWorld() { D(d); return d->world; }

	// IShaderLoadCallback
private:
	void onLoadShaders(gm::IGraphicEngine* engine);

private:
	void initLoadEffectsShader(gm::GMGLShaderProgram* effectsShaderProgram);
	void initLoadShaderProgram(gm::GMGLShaderProgram* forwardShaderProgram, gm::GMGLShaderProgram* deferredShaderProgram[2]);

	// IGameHandler
private:
	virtual void init() override;
	virtual void start() override;
	virtual void event(gm::GameMachineEvent evt) override;
};

inline gm::GMDemoGameWorld* asDemoGameWorld(gm::GMGameWorld* world)
{
	return gm::gm_cast<gm::GMDemoGameWorld*>(world);
}

#endif