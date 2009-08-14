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
#ifndef _TXCLIENT_H
#define _TXCLIENT_H

#include "tx.h"
#include "xa.h"

#ifndef WIN32
#define FTRACE(logger, message) { \
		LOG4CXX_TRACE(logger, __FUNCTION__ << (char *) ":" << __LINE__ << (char *) ":" << message); }

#else
#define FTRACE(logger, message) { \
		LOG4CXX_TRACE(logger, (char *) << message); }
#endif

/**
 * Modify the transaction associated with the target thread such that the only
 * possible outcome of the transaction is to roll back the transaction
 */
extern BLACKTIE_TX_DLL int txx_rollback_only();

/**
 * start an orb for making transactional service calls
 * (see orbInit in OrbManagement.h for implementation)
 */
extern BLACKTIE_TX_DLL void * txx_start(char *);

/**
 * stop the transaction manager proxy
 */
extern BLACKTIE_TX_DLL void txx_stop(void);

/**
 * associate a transaction with the current thread
 * (also resumes all Resource Managers linked into the running applications)
 */
extern BLACKTIE_TX_DLL int txx_bind(void *);

/**
 * Associate an OTS control with the current thread. The second parameter should
 * correspond to an (ACE) thread id
 */
extern BLACKTIE_TX_DLL int txx_bind(void *, int);

/**
 * Associate a transaction with the current thread:
 * - input parameter 1 is the name of the orb that serialized the IOR passed in the second parameter.
 *   If it is NULL then any orb will be used (and if no orb exists or the orb id is invalid
 *   then a negative return code will be returned)
 * - input parameter 2 is a serialized transaction (ie an IOR)
 *
 * Return a non-negative value on success
 */
extern BLACKTIE_TX_DLL int txx_associate_serialized(char *, char *);

/**
 * Convert the transaction associated with the calling thread into a string.
 * - input parameter 1 is the name of the orb that will perform the serialization.
 *   If it is NULL then any orb will be used (and if no orb exists or the orb id is invalid
 *   then a negative return code will be returned)
 *
 * Return a non-negative value on success
 */
extern BLACKTIE_TX_DLL char* txx_serialize(char *);

/**
 * disassociate a transaction from the current thread
 * (also suspends all Resource Managers linked into the running applications)
 * returns the transaction that was previously associated
 *
 * Returns the OTS control associated with the current thread. The caller
 * is responsible for calling release_control on the returned value.
 */
extern BLACKTIE_TX_DLL void * txx_unbind(void);

/**
 * Disassociate any transaction associated with the current thread only
 * if the caller did not start the thread.
 *
 * Returns the OTS control associated with the current thread. The caller
 * is responsible for calling release_control on the returned value.
 */
extern BLACKTIE_TX_DLL void * txx_unbind_if_not_owner(void);

/**
 * Return the OTS control associated with the current thread
 * The caller is responsible for calling release_control on the
 * returned control.
 */
extern BLACKTIE_TX_DLL void * txx_get_control();

/**
 * Release an OTS control returned by:
 * get_control
 * disassociate_tx_if_not_owner
 */
extern BLACKTIE_TX_DLL void txx_release_control(void *);

#endif //_TXCLIENT_H
