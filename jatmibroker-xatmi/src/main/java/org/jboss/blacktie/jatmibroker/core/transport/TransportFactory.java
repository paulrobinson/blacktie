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
package org.jboss.blacktie.jatmibroker.core.transport;

import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public abstract class TransportFactory {

	private static final Logger log = LogManager
			.getLogger(TransportFactory.class);
	private static Map<String, TransportFactory> transportFactories = new HashMap<String, TransportFactory>();

	private boolean closed;

	public static synchronized TransportFactory getTransportFactory(
			String serviceName, Properties properties)
			throws ConfigurationException {
		log.debug("Loading transport for: " + serviceName);
		String transportLibrary = (String) properties.getProperty("blacktie."
				+ serviceName + ".transportLib");
		log.debug("Transport library was: " + transportLibrary);
		if (transportLibrary == null) {
			throw new ConfigurationException("TransportLibrary was not defined");
		}
		// Determine the transport class to load
		String className = null;
		if (transportLibrary.contains("hybrid")) {
			className = org.jboss.blacktie.jatmibroker.core.transport.hybrid.TransportFactoryImpl.class
					.getName();
		}
		log.debug("Transport class was: " + className);

		if (!transportFactories.containsKey(className)) {
			try {
				Class clazz = Class.forName(className);
				TransportFactory newInstance = (TransportFactory) clazz
						.newInstance();
				newInstance.initialize(properties);
				transportFactories.put(className, newInstance);
				log.debug("TransportFactory was prepared");
			} catch (Throwable t) {
				throw new ConfigurationException(
						"Could not load the connection factory", t);
			}
		}
		return transportFactories.get(className);
	}

	protected abstract void initialize(Properties properties)
			throws ConfigurationException;

	public abstract Transport createTransport() throws ConnectionException;

	protected abstract void closeFactory();

	/**
	 * Make sure that the
	 */
	public synchronized final void close() {
		log.debug("Closing factory: " + getClass().getName());
		if (!closed) {
			log.debug("Going into shutdown");
			closeFactory();
			transportFactories.remove(getClass().getName());
			closed = true;
		}
		log.debug("Closed factory: " + getClass().getName());
	}
}
