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
package org.jboss.blacktie.jatmibroker.xatmi;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.transport.Message;
import org.jboss.blacktie.jatmibroker.transport.Receiver;
import org.jboss.blacktie.jatmibroker.transport.Transport;
import org.jboss.blacktie.jatmibroker.transport.TransportFactory;

/**
 * This is the connection to remote Blacktie services.
 */
public class Connection {

	private static final Logger log = LogManager.getLogger(Connection.class);

	// AVAILABLE FLAGS
	int TPNOBLOCK = 0x00000001;
	int TPSIGRSTRT = 0x00000002;
	int TPNOREPLY = 0x00000004;
	int TPNOTRAN = 0x00000008;
	int TPTRAN = 0x00000010;
	int TPNOTIME = 0x00000020;
	int TPGETANY = 0x00000080;
	int TPNOCHANGE = 0x00000100;
	int TPCONV = 0x00000400;
	int TPSENDONLY = 0x00000800;
	int TPRECVONLY = 0x00001000;

	// ERROR CONDITIONS
	public int TPEBADDESC = 2;
	public int TPEBLOCK = 3;
	public int TPEINVAL = 4;
	public int TPELIMIT = 5;
	public int TPENOENT = 6;
	public int TPEOS = 7;
	public int TPEPROTO = 9;
	public int TPESVCERR = 10;
	public int TPESVCFAIL = 11;
	public int TPESYSTEM = 12;
	public int TPETIME = 13;
	public int TPETRAN = 14;
	public int TPGOTSIG = 15;
	public int TPEITYPE = 17;
	public int TPEOTYPE = 18;
	public int TPEEVENT = 22;
	public int TPEMATCH = 23;

	private static int nextId;

	private Map<String, Transport> transports = new HashMap<String, Transport>();

	/**
	 * Any local temporary queues created in this connection
	 */
	private Map<java.lang.Integer, Receiver> temporaryQueues = new HashMap<java.lang.Integer, Receiver>();

	private Properties properties;

	/**
	 * The connection
	 * 
	 * @param properties
	 * @param username
	 * @param password
	 * @throws ConnectionException
	 */
	Connection(Properties properties) {
		this.properties = properties;
	}

	/**
	 * Synchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The returned buffer
	 */
	public Response tpcall(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		int cd = tpacall(svc, buffer, len, flags);
		return tpgetrply(cd, flags);
	}

	/**
	 * Asynchronous call
	 * 
	 * @param svc
	 *            The name of the service to call
	 * @param idata
	 *            The inbound data
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	public int tpacall(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		int correlationId = nextId++;
		Receiver endpoint = null;
		try {
			endpoint = getTransport(svc).createReceiver();
			temporaryQueues.put(correlationId, endpoint);
		} catch (Throwable t) {
			throw new ConnectionException(-1,
					"Could not create a temporary queue", t);
		}
		// TODO HANDLE TRANSACTION
		getTransport(svc).getSender(svc).send(endpoint.getReplyTo(), (short) 0,
				0, buffer.getData(), len, correlationId, flags);
		return correlationId;
	}

	/**
	 * Cancel the outstanding asynchronous call.
	 * 
	 * @param cd
	 *            The connection descriptor
	 * @param flags
	 *            The flags to use
	 */
	public void tpcancel(int cd, int flags) throws ConnectionException {
		Receiver endpoint = temporaryQueues.get(cd);
		endpoint.close();
	}

	/**
	 * Get the reply from the server
	 * 
	 * @param cd
	 *            The connection descriptor to use
	 * @param flags
	 *            The flags to use
	 * @return The response from the server
	 */
	public Response tpgetrply(int cd, int flags) throws ConnectionException {
		Receiver endpoint = temporaryQueues.get(cd);
		Message m = endpoint.receive(flags);
		// TODO WE SHOULD BE SENDING THE TYPE, SUBTYPE AND CONNECTION ID?
		Buffer received = new Buffer(null, null);
		received.setData(m.data);
		return new Response(m.rval, m.rcode, received, m.len, m.flags);
	}

	/**
	 * Handle the initiation of a conversation with the server
	 * 
	 * @param svc
	 *            The name of the service
	 * @param idata
	 *            The outbound buffer
	 * @param flags
	 *            The flags to use
	 * @return The connection descriptor
	 */
	public Session tpconnect(String svc, Buffer buffer, int len, int flags)
			throws ConnectionException {
		// Initiate the session
		int cd = tpacall(svc, buffer, len, flags);
		Receiver endpoint = temporaryQueues.get(cd);
		// Return a handle to allow the connection to send/receive data on
		return new Session(getTransport(svc), cd, endpoint);
	}

	/**
	 * Close any resources associated with this connection
	 * 
	 * @throws ConnectionException
	 */
	public void close() throws ConnectionException {
		Iterator<Receiver> receivers = temporaryQueues.values().iterator();
		while (receivers.hasNext()) {
			Receiver receiver = receivers.next();
			receiver.close();
		}
		temporaryQueues.clear();
		Iterator<Transport> transports = this.transports.values().iterator();
		while (transports.hasNext()) {
			Transport transport = transports.next();
			transport.close();
		}
		this.transports.clear();
	}

	private Transport getTransport(String serviceName)
			throws ConnectionException {
		Transport toReturn = transports.get(serviceName);
		if (toReturn == null) {
			try {
				toReturn = TransportFactory.loadTransportFactory(serviceName,
						properties).createTransport();
			} catch (ConfigurationException e) {
				throw new ConnectionException(-1, "Could not load properties",
						e);
			}
			transports.put(serviceName, toReturn);
		}
		return toReturn;
	}
}
