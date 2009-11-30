/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat, Inc., and others contributors as indicated
 * by the @authors tag. All rights reserved.
 * See the copyright.txt in the distribution for a
 * full listing of individual contributors.
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License, v. 2.1.
 * This program is distributed in the hope that it will be useful, but WITHOUT A
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public License,
 * v.2.1 along with this distribution; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 */
#include <cppunit/extensions/HelperMacros.h>
#include "TestAdmin.h"

void TestAdmin::setUp() {
	userlogc((char*) "TestAdmin::setUp");
	BaseAdminTest::setUp();
}

void TestAdmin::tearDown() {
	userlogc((char*) "TestAdmin::tearDown");
	BaseAdminTest::tearDown();
}

long TestAdmin::getBARCounter() {
	long n = 0;
	int cd;

	cd = callADMIN((char*)"counter,BAR,", '1', 0, &n);	
	CPPUNIT_ASSERT(cd == 0);

	return n;
}
void TestAdmin::testStatus() {
	int cd;

	cd = callADMIN((char*)"status", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);
	cd = callADMIN((char*)"status,BAR,", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);
}

void TestAdmin::testMessageCounter() {
	CPPUNIT_ASSERT(getBARCounter() == 0);
	CPPUNIT_ASSERT(callBAR(0) == 0);
	CPPUNIT_ASSERT(getBARCounter() == 1);
}

void TestAdmin::testServerdone() {
	int cd;

	cd = callADMIN((char*)"serverdone", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);
}

void TestAdmin::testServerPauseAndResume() {
	int cd;

	CPPUNIT_ASSERT(callBAR(0) == 0);
	userlogc((char*)"call BAR OK");

	cd = callADMIN((char*)"pause", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);
	userlogc((char*)"pause server OK");

	userlogc((char*)"unadvertise on pause server should OK");
	cd = callADMIN((char*)"unadvertise,BAR,", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);

	userlogc((char*)"advertise on pause server should OK, but service is still pause");
	cd = callADMIN((char*)"advertise,BAR,", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);

	userlogc((char*)"call BAR should time out after 20 seconds");
	CPPUNIT_ASSERT(callBAR(TPETIME) == -1);

	cd = callADMIN((char*)"resume", '1', 0, NULL);
	CPPUNIT_ASSERT(cd == 0);
	userlogc((char*)"resume server OK");

	CPPUNIT_ASSERT(callBAR(0) == 0);
	userlogc((char*)"call BAR OK");
}
