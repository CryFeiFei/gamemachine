﻿#include <stdafx.h>
#include "cases/hook.h"
#include "cases/string.h"
#include "cases/scanner.h"
#include "cases/thread.h"
#include "cases/linearmath.h"
#include "cases/variant.h"
#include "cases/lua.h"
#include "cases/base64.h"

int main(int argc, char* argv[])
{
	UnitTest unitTest;

	UnitTestCase* caseArray[] = {
		new cases::Hook(),
		new cases::String(),
		new cases::Scanner(),
		new cases::LinearMath(),
		new cases::Variant(),
		new cases::Lua(),
		new cases::Base64(),
		new cases::Thread()
	};

	for (auto& c : caseArray)
	{
		c->addToUnitTest(unitTest);
	}
	unitTest.run();

	for (auto& c : caseArray)
	{
		delete c;
	}

	getchar();
	return 0;
}