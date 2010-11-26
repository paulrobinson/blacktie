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

#include <string.h>
#include <exception>

#include "apr_strings.h"

#include "malloc.h"
#include "HybridSessionImpl.h"
#include "HybridCorbaEndpointQueue.h"
#include "HybridStompEndpointQueue.h"
#include "txx.h"

#include "ThreadLocalStorage.h"
#include "AtmiBrokerEnv.h"

#include "BufferConverterImpl.h"

#include <time.h>

log4cxx::LoggerPtr HybridSessionImpl::logger(log4cxx::Logger::getLogger(
		"HybridSessionImpl"));

static int receiptCounter = 0;

HybridSessionImpl::HybridSessionImpl(bool isConv, char* connectionName,
		CORBA_CONNECTION* connection, apr_pool_t* pool, int id,
		char* serviceName, void(*messagesAvailableCallback)(int, bool)) {
	LOG4CXX_TRACE(logger, (char*) "constructor service");
	this->isConv = isConv;
	long ttl;

	switch (txx_ttl(&ttl)) {
	case -1: // no txn bound to thread so use XATMI timeouts
		ttl = mqConfig.requestTimeout;
		break;
	default: /*FALLTHRU - txx_ttl can only return -1, 0 or 1*/
	case 0: // will create a non-blocking socket that times out after ttl seconds
		break;
	case 1:
		ttl = 0; // will create a blocking socket
		break;
	}

	this->id = id;
	this->corbaConnection = connection;
	this->temporaryQueueName = NULL;
	serviceInvokation = true;

	stompConnection = NULL;
	stompConnection = HybridConnectionImpl::connect(pool, ttl);
	this->pool = pool;

	// XATMI_SERVICE_NAME_LENGTH is in xatmi.h and therefore not accessible
	int XATMI_SERVICE_NAME_LENGTH = 15;
	int queueNameLength = 14 + 15 + 1;
	this->sendTo  = (char*) ::malloc(queueNameLength);
	memset(this->sendTo, '\0', queueNameLength);
	if (isConv) {
		strcpy(this->sendTo, "/queue/BTC_");
	} else {
		strcpy(this->sendTo, "/queue/BTR_");
	}
	strncat(this->sendTo, serviceName, XATMI_SERVICE_NAME_LENGTH);

	remoteEndpoint = NULL;

	this->canSend = true;
	this->canRecv = true;

	char * poaName = (char*) malloc(30);
	::sprintf(poaName, "%s%d", connectionName, id);
	this->temporaryQueue = new HybridCorbaEndpointQueue(this, corbaConnection,
			poaName, id, messagesAvailableCallback);
	this->replyTo = temporaryQueue->getName();
	this->lastEvent = 0;
	this->lastRCode = 0;
	this->serviceName = serviceName;
	LOG4CXX_TRACE(logger, "OK service session created");
}

HybridSessionImpl::HybridSessionImpl(bool isConv, char* connectionName,
		CORBA_CONNECTION* connection, apr_pool_t* pool, int id,
		const char* temporaryQueueName, void(*messagesAvailableCallback)(int,
				bool)) {
	LOG4CXX_DEBUG(logger, (char*) "constructor corba");
	this->isConv = isConv;
	this->id = id;
	this->corbaConnection = connection;
	this->temporaryQueueName = temporaryQueueName;
	serviceInvokation = false;

	stompConnection = NULL;
	this->pool = pool;
	this->sendTo = NULL;

	LOG4CXX_DEBUG(logger, (char*) "RemoteEndpoint: " << temporaryQueueName);
	CORBA::Object_var tmp_ref = corbaConnection->orbRef->string_to_object(
			temporaryQueueName);
	remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
	LOG4CXX_DEBUG(logger, (char*) "RemoteEndpoint: " << temporaryQueueName << " ENDPONT: " << remoteEndpoint);

	this->canSend = true;
	this->canRecv = true;

	char * poaName = (char*) malloc(30);
	::sprintf(poaName, "%s%d", connectionName, id);
	this->temporaryQueue = new HybridCorbaEndpointQueue(this, corbaConnection,
			poaName, id, messagesAvailableCallback);
	this->replyTo = temporaryQueue->getName();
	this->lastEvent = 0;
	this->lastRCode = 0;
	this->serviceName = 0;
	LOG4CXX_DEBUG(logger, (char*) "constructor corba done");
}

HybridSessionImpl::~HybridSessionImpl() {
	setSendTo( NULL);
	LOG4CXX_TRACE(logger, (char*) "TQ Closed: " << replyTo);
	delete temporaryQueue;
	temporaryQueue = NULL;

	if (stompConnection) {
		LOG4CXX_TRACE(logger, (char*) "destroying");
		HybridConnectionImpl::disconnect(stompConnection, pool);
		LOG4CXX_TRACE(logger, (char*) "destroyed");
		stompConnection = NULL;
	}
	if (temporaryQueueName != NULL) {
		LOG4CXX_TRACE(logger, (char*) "TQ Disconnected: "
				<< temporaryQueueName);
		temporaryQueueName = NULL;
	}
}

void HybridSessionImpl::setSendTo(const char* destinationName) {
	if (this->sendTo != NULL && (destinationName == NULL || strcmp(
			destinationName, this->sendTo) != 0)) {
		::free(this->sendTo);
		remoteEndpoint = NULL;
		this->sendTo = NULL;
	}

	if (destinationName != NULL && strcmp(destinationName, "") != 0
			&& this->sendTo == NULL) {
		LOG4CXX_DEBUG(logger, (char*) "RemoteEndpoint: " << destinationName);
		CORBA::Object_var tmp_ref = corbaConnection->orbRef->string_to_object(
				destinationName);
		remoteEndpoint = AtmiBroker::EndpointQueue::_narrow(tmp_ref);
		LOG4CXX_DEBUG(logger, (char*) "RemoteEndpoint: " << remoteEndpoint);
		this->sendTo = strdup((char*) destinationName);
	}
}

MESSAGE HybridSessionImpl::receive(long time) {
	MESSAGE message = temporaryQueue->receive(time);
	if (message.replyto != NULL && strcmp(message.replyto, "") != 0) {
		setSendTo(message.replyto);
	} else {
		setSendTo( NULL);
	}
	return message;
}

bool HybridSessionImpl::send(MESSAGE message) {
	LOG4CXX_DEBUG(logger, "HybridSessionImpl::send");

	char* data_togo = BufferConverterImpl::convertToWireFormat(message.type,
			message.subtype, message.data, &message.len);

	bool toReturn = false;
	if (serviceInvokation) {
		stomp_frame frame;
		frame.command = (char*) "SEND";
		frame.headers = apr_hash_make(pool);
		apr_hash_set(frame.headers, "destination", APR_HASH_KEY_STRING, sendTo);

		frame.body_length = message.len;
		frame.body = data_togo;
		if (message.replyto && strcmp(message.replyto, "") != 0) {
			LOG4CXX_TRACE(logger, "send set messagereplyto: "
					<< message.replyto);
			apr_hash_set(frame.headers, "messagereplyto", APR_HASH_KEY_STRING,
					message.replyto);
		} else {
			LOG4CXX_TRACE(logger, "send not set messagereplyto");
		}
		char * correlationId = apr_itoa(pool, message.correlationId);
		char * flags = apr_itoa(pool, message.flags);
		char * rval = apr_itoa(pool, message.rval);
		char * rcode = apr_itoa(pool, message.rcode);
		char * priority = apr_itoa(pool, message.priority);

		apr_hash_set(frame.headers, "messagecorrelationId",
				APR_HASH_KEY_STRING, correlationId);
		apr_hash_set(frame.headers, "priority", APR_HASH_KEY_STRING, priority);
		LOG4CXX_TRACE(logger, "Set the corrlationId: " << correlationId << " and priority: "
			<< priority);
		apr_hash_set(frame.headers, "messageflags", APR_HASH_KEY_STRING, flags);
		apr_hash_set(frame.headers, "messagerval", APR_HASH_KEY_STRING, rval);
		apr_hash_set(frame.headers, "messagercode", APR_HASH_KEY_STRING, rcode);
		if (serviceName != NULL) {
			apr_hash_set(frame.headers, "servicename", APR_HASH_KEY_STRING,
					serviceName);
		}
		apr_hash_set(frame.headers, "messagetype", APR_HASH_KEY_STRING,
				message.type);
		apr_hash_set(frame.headers, "messagesubtype", APR_HASH_KEY_STRING,
				message.subtype);

		if (message.control) {
			LOG4CXX_TRACE(logger, "Sending serialized control: "
					<< message.control);
			apr_hash_set(frame.headers, "messagecontrol", APR_HASH_KEY_STRING,
					message.control);
		}

		LOG4CXX_DEBUG(logger, "Send to: " << sendTo << " Command: "
				<< frame.command << " Size: " << frame.body_length);

		apr_status_t rc;
		/*
		 * Determine whether the caller has requested non-blocking semantics. Unfortunately
		 * the current module cannot depend on the xatmi module so we cannot pull in the
		 * flag from xatmi.h that defines TPNOBLOCK
		 * TODO figure out how we can access TPNOBLOCK without needing to depend on xatmi
		 * Note that we had a similar issue with XATMI_SERVICE_NAME_LENGTH in the constructor
		 */
		bool isNonBlocking = (message.flags & 0x00000001); // TODO find a way to pull in TPNOBLOCK

		if (stompConnection == NULL) {
			 rc = APR_ENOSOCKET;
		} else {
			if (isNonBlocking) {
				LOG4CXX_TRACE(logger, "Setting socket_opt to non-blocking for send");
#if 1
				apr_socket_timeout_set(stompConnection->socket, 1);
#else
				apr_socket_timeout_set(stompConnection->socket, 0);
#endif
				// Note: sockets are created on a per request basis so there is no
				// need to clear the socket opt after sending the frame.
			}

			// Check to set the ttl
			char* ttl = NULL;
			if (message.ttl > 0) {
				long long epoch = time(NULL) * (long long)1000;
				long long longTTL = epoch + message.ttl;
				ttl = (char*) malloc(32); // #   define ULLONG_MAX	18446744073709551615ULL from /usr/include/limits.h
				sprintf(ttl, "%lld", longTTL);
				apr_hash_set(frame.headers, "expires", APR_HASH_KEY_STRING, ttl);
				LOG4CXX_TRACE(logger, "Set the expires ttl: " << ttl);
			}

			// Create a receipt ID to send
			char * receiptId = (char*) malloc(10);
			::sprintf(receiptId, "send-%d", receiptCounter);
			apr_hash_set(frame.headers, "receipt", APR_HASH_KEY_STRING, receiptId);
	
			// Send the data
			rc = stomp_write(stompConnection, &frame, pool);
			
			// Free temporary storage
			free(receiptId);
			if (ttl != NULL) {
				free(ttl);	
			}
		}

		if (isNonBlocking && rc == APR_TIMEUP) {
			LOG4CXX_DEBUG(logger, "Could not send frame due to blocking condition on socket");
			setSpecific(TPE_KEY, TSS_TPEBLOCK);
		} else if (rc != APR_SUCCESS) {
			LOG4CXX_ERROR(logger, "Could not send frame");
			char errbuf[256];
			apr_strerror(rc, errbuf, sizeof(errbuf));
			LOG4CXX_ERROR(logger, (char*) "APR Error was: " << rc << ": "
					<< errbuf);
			setSpecific(TPE_KEY, TSS_TPENOENT); // TODO - clean up session
			//			free(errbuf);
		} else {
			LOG4CXX_TRACE(logger, "Sent frame");
			stomp_frame *framed;
			// now read back the acknowledgent that the message was received but first change the timeout on the socket
			if (isNonBlocking) {
				// enable blocking-with-timeout for 1 second to provide time for the response:
				LOG4CXX_TRACE(logger, "Setting socket_opt to blocking for at most 1 second for ack");
				apr_socket_opt_set(stompConnection->socket, APR_SO_NONBLOCK, 0);
				apr_socket_timeout_set(stompConnection->socket, 1);
			}

			rc = stomp_read(stompConnection, &framed, pool);
			if (isNonBlocking && rc == APR_TIMEUP) {
				LOG4CXX_DEBUG(logger, "Could not send frame due to blocking condition on socket");
				setSpecific(TPE_KEY, TSS_TPEBLOCK);
			} else if (rc != APR_SUCCESS) {
				LOG4CXX_ERROR(logger, "Could not send frame");
				char errbuf[256];
				apr_strerror(rc, errbuf, sizeof(errbuf));
				LOG4CXX_ERROR(logger, (char*) "APR Error was: " << rc << ": "
						<< errbuf);
				setSpecific(TPE_KEY, TSS_TPENOENT); // TODO - clean up session
				//				free(errbuf);
			} else if (strcmp(framed->command, (const char*) "ERROR") == 0) {
				LOG4CXX_DEBUG(logger, (char*) "Got an error: " << framed->body);
				setSpecific(TPE_KEY, TSS_TPENOENT); // TODO - clean up session
			} else if (strcmp(framed->command, (const char*) "RECEIPT") == 0) {
				LOG4CXX_DEBUG(logger, (char*) "SEND RECEIPT: "
						<< (char*) apr_hash_get(framed->headers, "receipt-id",
								APR_HASH_KEY_STRING));
				LOG4CXX_DEBUG(logger, "Sent to: " << sendTo << " Command: "
						<< frame.command << " Size: " << frame.body_length);
				toReturn = true;
			} else {
				LOG4CXX_ERROR(logger, "Didn't get a receipt: "
						<< framed->command << ", " << framed->body);
				LOG4CXX_DEBUG(logger, "Sent: " << sendTo << " Command: "
						<< frame.command << " Size: " << frame.body_length);
				setSpecific(TPE_KEY, TSS_TPESYSTEM); // TODO - clean up session
			}
			delete[] data_togo;
			LOG4CXX_DEBUG(logger, "Will return: " << toReturn);
		}
		serviceInvokation = false;
	} else {
		if (remoteEndpoint != NULL) {
			LOG4CXX_DEBUG(logger, (char*) "Sending to RemoteEndpoint: "
					<< remoteEndpoint << " : " << replyTo);
			AtmiBroker::octetSeq_var aOctetSeq = new AtmiBroker::octetSeq(
					message.len, message.len, (unsigned char*) data_togo, true);
			try {
				remoteEndpoint->send(message.replyto, message.rval,
						message.rcode, aOctetSeq, message.len,
						message.correlationId, message.flags, message.type,
						message.subtype);
				aOctetSeq = NULL;
				LOG4CXX_DEBUG(logger, (char*) "Called back ");
				toReturn = true;
			} catch (const CORBA::SystemException& ex) {
				LOG4CXX_WARN(logger, (char*) "Caught SystemException: "
						<< ex._name());
			} catch (CORBA::Exception& e) {
				LOG4CXX_WARN(logger, (char*) "Caught exception: "
						<< e._name());
			} catch (...) {
				LOG4CXX_ERROR(logger,
						(char*) "UNEXPECTED EXCEPTION RETURNING RESPONSE");
			}
		} else {
			LOG4CXX_WARN(logger, (char*) "No remote endpoint to send to");
		}
	}
	LOG4CXX_DEBUG(logger, "HybridSessionImpl::sent");
	return toReturn;
}

void HybridSessionImpl::disconnect() {
	temporaryQueue->disconnect();
}

const char* HybridSessionImpl::getReplyTo() {
	return replyTo;
}

int HybridSessionImpl::getId() {
	return id;
}
