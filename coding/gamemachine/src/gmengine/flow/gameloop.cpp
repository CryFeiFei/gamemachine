﻿#include "stdafx.h"
#include <cstdio>
#include "gameloop.h"
#include "GL/freeglut.h"
#include "utilities/assert.h"

static void renderLoop(int i)
{
	if (i == 0)
	{
		GameLoop* loop = GameLoop::getInstance();
		loop->drawFrame();
		if (!loop->isTerminated())
			glutTimerFunc(1, renderLoop, 0);
		else
			loop->exit();
	}
}

GameLoop* GameLoop::getInstance()
{
	static GameLoop s_gameLoop;
	return &s_gameLoop;
}

GameLoop::GameLoop()
	: m_running(false)
	, m_terminate(false)
	, m_timeElapsed(1.f / 60.f)
	, m_handler(nullptr)
{
}

GameLoop::~GameLoop()
{

}

void GameLoop::init(const GameLoopSettings& settings, IGameHandler* handler)
{
	m_settings = settings;
	m_handler = handler;
	updateSettings();
}

void GameLoop::drawFrame()
{
	m_handler->logicalFrame(m_timeElapsed);

	m_drawStopwatch.start();
	m_handler->mouse();
	m_handler->keyboard();
	m_handler->render();
	m_drawStopwatch.stop();
	m_timeElapsed = (m_drawStopwatch.getMillisecond()) / 1000 + (1.f / 60.f);
}

void GameLoop::start()
{
	glutTimerFunc(1, renderLoop, 0);
}

void GameLoop::terminate()
{
	m_terminate = true;
}

bool GameLoop::isTerminated()
{
	return m_terminate;
}

void GameLoop::updateSettings()
{
	m_eachFrameElapse = 1000.0f / m_settings.fps;
}

void GameLoop::exit()
{
	::exit(0);
}