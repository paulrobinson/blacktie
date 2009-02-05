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
/*
 * BREAKTHRUIT PROPRIETARY - NOT TO BE DISCLOSED OUTSIDE BREAKTHRUIT, LLC.
 */
// copyright 2006, 2008 BreakThruIT

//
// Servant which implements the AtmiBroker::ServiceFactory interface.
//
#ifdef TAO_COMP
#include <tao/ORB.h>
#include "tao/ORB_Core.h"
#include "AtmiBrokerC.h"
#elif ORBIX_COMP
#include <omg/orb.hh>
#endif
#ifdef VBC_COMP
#include <orb.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "ServiceQueue.h"
#include "ThreadLocalStorage.h"

#include "AtmiBrokerServer.h"
#include "AtmiBroker_ServiceImpl.h"
#include "AtmiBrokerServiceXml.h"

#include "xatmi.h"
#include "userlog.h"

#include "log4cxx/logger.h"

log4cxx::LoggerPtr loggerServiceQueue(log4cxx::Logger::getLogger("ServiceQueue"));

// Constants
int MAX_SERVICE_CACHE_SIZE = 1;

// ServiceQueue constructor
//
// Note: since we use virtual inheritance, we must include an
// initialiser for all the virtual base class constructors that
// require arguments, even those that we inherit indirectly.
//
ServiceQueue::ServiceQueue(void* thePoa, char *serviceName, void(*func)(TPSVCINFO *)) :
	thePoa(thePoa), serviceName(serviceName), m_shutdown(false), lock(SynchronizableObject::create(false)) {
	// Intentionally empty.
	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "constructor() ");

	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "getDescriptorData() ");
	serviceInfo.poolSize = MAX_SERVICE_CACHE_SIZE;
	char* serviceConfigFilename = (char*) malloc(sizeof(char) * (XATMI_SERVICE_NAME_LENGTH + 5));
	strcpy(serviceConfigFilename, serviceName);
	strcat(serviceConfigFilename, ".xml");
	AtmiBrokerServiceXml aAtmiBrokerServiceXml;
	aAtmiBrokerServiceXml.parseXmlDescriptor(&serviceInfo, serviceConfigFilename);
	free(serviceConfigFilename);

	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "createPool");
	for (int i = 0; i < MAX_SERVICE_CACHE_SIZE; i++) {
		AtmiBroker_ServiceImpl * tmp_servant = new AtmiBroker_ServiceImpl(serviceName, func);
		ServiceDispatcher* dispatcher = new ServiceDispatcher(this, tmp_servant);
		if (dispatcher->activate(THR_NEW_LWP| THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, 0, 0, 0, 0, 0, 0) != 0) {
			delete dispatcher;
			LOG4CXX_ERROR(loggerServiceQueue, (char*) "Could not start thread pool");
		} else {
			servantVector.push_back(dispatcher);
		}
	}
	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "createPool done ");
}

// ~ServiceQueue destructor.
//
ServiceQueue::~ServiceQueue() {

	lock->lock();
	m_shutdown = true;
	lock->unlock();

	for (std::vector<ServiceDispatcher*>::iterator i = servantVector.begin(); i != servantVector.end(); i++) {
		ServiceDispatcher* dispatcher = (*i);
		dispatcher->shutdown();
	}

	// TODO NOTIFY ALL REQUIRED HERE
	for (std::vector<ServiceDispatcher*>::iterator i = servantVector.begin(); i != servantVector.end(); i++) {
		lock->lock();
		lock->notify();
		lock->unlock();
	}

	std::vector<ServiceDispatcher*>::iterator i = servantVector.begin();
	while (i != servantVector.end()) {
		ServiceDispatcher* dispatcher = (*i);
		i = servantVector.erase(i);
		dispatcher->wait();
		delete dispatcher;
	}
}

void ServiceQueue::send(const char* replyto_ior, CORBA::Short rval, CORBA::Long rcode, const AtmiBroker::octetSeq& idata, CORBA::Long ilen, CORBA::Long correlationId, CORBA::Long flags) throw (CORBA::SystemException ) {
	MESSAGE message;
	message.replyto = replyto_ior;
	message.data = (char*) malloc(sizeof(char*) * ilen);
	memcpy(message.data, (char*) idata.get_buffer(), ilen);
	message.len = ilen;
	message.correlationId = correlationId;
	message.flags = flags;
	message.control = getSpecific(TSS_KEY);

	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "start_conversation()");

	lock->lock();
	messageQueue.push(message);
	lock->notify();
	lock->unlock();
}

void ServiceQueue::disconnect() {

}

MESSAGE ServiceQueue::receive(long flags) {
	MESSAGE message;
	message.data = NULL;
	lock->lock();
	if (!m_shutdown) {
		if (messageQueue.size() == 0) {
			lock->wait(0);
		}
		if (messageQueue.size() > 0) {
			message = messageQueue.front();
			messageQueue.pop();
		}
	}
	lock->unlock();
	return message;
}

const char * ServiceQueue::getName() {
	return NULL;
}

SVCINFO ServiceQueue::get_service_info() {
	userlog(log4cxx::Level::getDebug(), loggerServiceQueue, (char*) "get_service_info()");
	SVCINFO svcinfo;
	svcinfo.serviceName = strdup(serviceName);
	svcinfo.poolSize = serviceInfo.poolSize;
	svcinfo.securityType = strdup("");
	return svcinfo;
}

void* ServiceQueue::getPoa() {
	return thePoa;
}

