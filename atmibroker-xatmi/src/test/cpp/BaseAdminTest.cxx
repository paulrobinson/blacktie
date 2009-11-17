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

#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "xatmi.h"
#include "userlogc.h"
#include "BaseAdminTest.h"

void BaseAdminTest::setUp() {
	BaseServerTest::setUp();
}

void BaseAdminTest::tearDown() {
	BaseServerTest::tearDown();
}

int BaseAdminTest::callADMIN(char* command, char expect, int r, long* n) {
	long  sendlen = strlen(command) + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, command);

	char* recvbuf = tpalloc((char*) "X_OCTET", NULL, 1);
	long  recvlen = 1;

	int cd = ::tpcall((char*) "default_ADMIN_1", (char *) sendbuf, sendlen, (char**)&recvbuf, &recvlen, TPNOTRAN);
	CPPUNIT_ASSERT(recvbuf[0] == expect);
	CPPUNIT_ASSERT(r == tperrno);

	if(ACE_OS::strncmp(command, "counter", 7) == 0) {
		*n = atol(&recvbuf[1]);
	} else if(ACE_OS::strncmp(command, "status", 6) == 0) {
		userlogc((char*) "len is %d, service status: %s", recvlen, &recvbuf[1]);
	}

	tpfree(sendbuf);
	tpfree(recvbuf);

	return cd;
}

int BaseAdminTest::callBAR(int r) {
	long  sendlen = strlen((char*)"test") + 1;
	char* sendbuf = tpalloc((char*) "X_OCTET", NULL, sendlen);
	strcpy(sendbuf, (char*) "test");

	int cd = ::tpacall((char*) "BAR", (char *) sendbuf, sendlen, TPNOREPLY);
	CPPUNIT_ASSERT(r == tperrno);
	tpfree(sendbuf);

	return cd;
}
