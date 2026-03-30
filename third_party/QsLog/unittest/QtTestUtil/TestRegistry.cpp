/*
 * Copyright (C) 2008  Remko Troncon
 * Licensed under the MIT license.
 * See COPYING for license details.
 */

#include "QtTestUtil/TestRegistry.h"

#include <QtTest/QtTest>

namespace QtTestUtil {

TestRegistry* TestRegistry::getInstance() {
	static TestRegistry registry;
	return &registry;
}

void TestRegistry::registerTest(QObject* test) {
	tests_ += test;
}

int TestRegistry::runTests(int argc, char* argv[]) {
	int result = 0;
    const auto &tests = tests_;
    for (auto *test : tests) {
        result |= QTest::qExec(test, argc, argv);
    }
	return result;
}

}
