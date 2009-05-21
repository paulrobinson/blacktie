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
package org.jboss.blacktie.jatmibroker.jab;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Buffer;
import org.jboss.blacktie.jatmibroker.xatmi.Connection;
import org.jboss.blacktie.jatmibroker.xatmi.Response;

/**
 * Create an invoker for a remote service.
 */
public class JABRemoteService {
	private static final Logger log = LogManager
			.getLogger(JABRemoteService.class);
	private Connection connection;
	private String serviceName;
	private String bufferType;
	private byte[] data;
	private int length;
	private Response response;

	public JABRemoteService(JABSession aJABSession, String aServiceName)
			throws JABException {
		log.debug("JABService constructor");

		connection = aJABSession.getConnection();
		serviceName = aServiceName;
	}

	public void call(JABTransaction aJABTransaction) throws JABException {
		log.debug("JABService call");

		try {
			// TODO HANDLE TRANSACTION
			Buffer buffer = new Buffer(bufferType, null);
			buffer.setData(data);
			response = connection.tpcall(serviceName, buffer, length, 0);
			log.debug("service_request responsed");
		} catch (Exception e) {
			throw new JABException(e);
		}
	}

	public void clear() {
		log.debug("JABService clear");
		data = null;
		response = null;
	}

	public void setBuffer(String name, byte[] data, int length) {
		log.debug("JABService set buffer");
		this.bufferType = name;
		this.data = data;
		this.length = length;
	}

	public byte[] getResponseData() {
		return response.getBuffer().getData();
	}
}
