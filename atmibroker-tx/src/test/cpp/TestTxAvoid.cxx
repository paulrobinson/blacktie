/*
 * JBoss, Home of Professional Open Source
 * Copyright 2009, Red Hat, Inc., and others contributors as indicated
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
#include "testTxAvoid.h"
#include "OrbManagement.h"
#include "XAResourceAdaptorImpl.h"
#include "ace/OS_NS_unistd.h"
#include "TxManager.h"
#include "userlogc.h"

void doOne() {
try {
		XARecoveryLog log("test_recovery_log");

		XID gid = {1L, 1L, 0L};
		XID xid = XAResourceManager::gen_xid(200, gid);
		XID xid2 = XAResourceManager::gen_xid(201, gid);
		int rv, cnt = 0;
		char* ior = (char *) "IOR:1";

		// add a record
		rv = log.add_rec(xid, ior);
		CPPUNIT_ASSERT(rv == 0);

		// delete it by XID
		rv = log.del_rec(xid);
		CPPUNIT_ASSERT(rv == 0);

		// add a record
		rv = log.add_rec(xid, ior);
		CPPUNIT_ASSERT(rv == 0);

		// find it by xid
		ior = log.find_ior(xid);
		CPPUNIT_ASSERT(ior != 0);

		// add another record
		CPPUNIT_ASSERT(log.add_rec(xid2, (char *) "IOR:2") == 0);

		// use an iterator to check that there are two records
		for (rrec_t* rr = log.find_next(0); rr; rr = log.find_next(rr)) {
			cnt += 1;
			log.del_rec(rr->xid);
		}

		CPPUNIT_ASSERT(cnt >= 2);
	} catch (RMException e) {
		std::string s = "Error creating recovery log: ";
		s += e.what();
		CPPUNIT_FAIL(s.c_str());
	}
}

void doTwo() {
userlogc_debug( (char*) "TestTransactions::test_basic begin");
	atmibroker::tx::TxManager *txm = atmibroker::tx::TxManager::get_instance();
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->open());
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->begin());
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->suspend(123));
	CPPUNIT_ASSERT_EQUAL(true, txm->isCdTransactional(123));
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->resume(123));
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->commit());
	CPPUNIT_ASSERT_EQUAL(TX_OK, txm->close());
	atmibroker::tx::TxManager::discard_instance();
	userlogc( (char*) "TestTransactions::test_basic pass");
}

 
void doThree(long delay) {
(void) ACE_OS::sleep(delay);
}

void doFour() {
CPPUNIT_ASSERT_EQUAL(TX_OK, txx_rollback_only());
}

static int fn1(char *a, int i, long l) { return 0; }
static int fn2(XID *x, int i, long l) { return 0; }
static int fn3(XID *, long l1, int i, long l2) { return 0; }
static int fn4(int *ip1, int *ip2, int i, long l) { return 0; }


static struct xa_switch_t real_resource = { "DummyRM", 0L, 0, fn1, fn1, /* open and close */
	fn2, fn2, fn2, fn2, fn2, /*start, end, rollback, prepare, commit */
	fn3, /* recover */
	fn2, /* forget */
	fn4 /* complete */
};
// manufacture a dummy RM transaction id
static XID xid = {
		1L, /* long formatID */
		0L, /* long gtrid_length */
		0L, /* long bqual_length */
		{0} /* char data[XIDDATASIZE]; */
};



void* doFive() {
	XARecoveryLog log;
	CosTransactions::Control_ptr curr = (CosTransactions::Control_ptr) txx_get_control();
	// there should be a transaction running
	CPPUNIT_ASSERT_MESSAGE("curr is nil", !CORBA::is_nil(curr));
	CosTransactions::Coordinator_ptr c = curr->get_coordinator(); // will leak curr if exception
	txx_release_control(curr);
	// and it should have a coordinator
	CPPUNIT_ASSERT_MESSAGE("coordinator is nil", !CORBA::is_nil(c));

	// a side effect of starting a transaction is to start an orb
	CORBA::ORB_ptr orb = find_orb("ots");
	CPPUNIT_ASSERT_MESSAGE("orb is nil", !CORBA::is_nil(orb));
	// get a handle on a poa
	CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

	PortableServer::POAManager_var mgr = poa->the_POAManager();
	mgr->activate();

	// now for the real test:
	// - create a CosTransactions::Resource ...
	//XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(findConnection("ots"), "Dummy", "", "", 123L, &real_resource, log);
	XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(NULL, xid, xid, 123L, &real_resource, log);
	//XAResourceAdaptorImpl * ra = new XAResourceAdaptorImpl(123L, &real_resource, log);
	CORBA::Object_ptr ref = poa->servant_to_reference(ra);
	CosTransactions::Resource_var v = CosTransactions::Resource::_narrow(ref);

	try {
		// ... and enlist it with the transaction
		CosTransactions::RecoveryCoordinator_ptr rc = c->register_resource(v);
		CPPUNIT_ASSERT_MESSAGE("recovery coordinator is nil", !CORBA::is_nil(rc));
	} catch (CosTransactions::Inactive&) {
		CPPUNIT_FAIL("corba resource registration error - inactive");
	} catch (const CORBA::SystemException& ex) {
		ex._tao_print_exception("Resource registration error: ");
		CPPUNIT_FAIL("corba resource registration error - system ex");
	}
	return ra;
}

void doSix(long delay) {
(void) ACE_OS::sleep(delay);
}
void doSeven(void* rad) {
	XAResourceAdaptorImpl * ra = (XAResourceAdaptorImpl *) rad;
	// the resource should have been committed
	CPPUNIT_ASSERT_MESSAGE("resource did not complete", ra->is_complete());

}