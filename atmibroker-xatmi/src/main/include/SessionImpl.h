/*
 * JBoss, Home of Professional Open Source
 * Copyright 2008, Red Hat Middleware LLC, and others contributors as indicated
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
#ifndef SessionImpl_H_
#define SessionImpl_H_

#include "log4cxx/logger.h"

#include "Session.h"
#include "Sender.h"
#include "Receiver.h"

class SessionImpl: public virtual Session {
public:
	SessionImpl(void* connection_poa, void* connection_orb, int id);

	virtual ~SessionImpl();

	void setSendTo(char* replyTo);

	Receiver* getReceiver();

	Sender* getSender();
private:
	static log4cxx::LoggerPtr logger;
	int id;
	void* connection_orb;
	Receiver* queueReceiver;
	Sender* queueSender;
};

#endif
