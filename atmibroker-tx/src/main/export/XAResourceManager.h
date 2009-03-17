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
#ifndef XARESOURCEMANAGERFACTORY_H
#define XARESOURCEMANAGERFACTORY_H

#include "atmiBrokerTxMacro.h"
#include "log4cxx/logger.h"
#include "CorbaConnection.h"
#include "RMException.h"
#include "XAResourceAdaptorImpl.h"

#include <map>

#include <tao/PortableServer/PortableServer.h>
#include "CosTransactionsS.h"
#include "xa.h"

class XAResourceAdaptorImpl;

extern log4cxx::LoggerPtr xaResourceLogger;

class ATMIBROKER_TX_DLL XAResourceManager
{
public:
        XAResourceManager(CORBA_CONNECTION *, const char *, const char *, const char *, CORBA::Long, struct xa_switch_t *)
                throw (RMException);
        virtual ~XAResourceManager();

	int xa_start (XID *, int, long);
	int xa_end (XID *, int, long);

        // return the resource id
        CORBA::Long rmid(void) {return rmid_;};
	void notifyError(XID *, int, bool);
	void setComplete(XID*);
	const char * name() {return name_;}

private:
	typedef std::map<XID *, XAResourceAdaptorImpl *> XABranchMap;
	XABranchMap branches_;

        PortableServer::POA_ptr poa_;
        CORBA_CONNECTION* connection_;
        const char *name_;
        const char *openString_;
        const char *closeString_;
        CORBA::Long rmid_;
        struct xa_switch_t * xa_switch_;

	void createPOA();
        int createServant(XID *);
	XAResourceAdaptorImpl * locateBranch(XID *);

	void show_branches(const char *, XID *);
};
#endif // XARESOURCEMANAGERFACTORY_H
