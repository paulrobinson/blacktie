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
package org.jboss.blacktie.jatmibroker.core.transport.corba;

import java.util.Properties;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.transport.OrbManagement;
import org.jboss.blacktie.jatmibroker.core.transport.Transport;
import org.jboss.blacktie.jatmibroker.core.transport.TransportFactory;
import org.jboss.blacktie.jatmibroker.xatmi.ConnectionException;

public class TransportFactoryImpl extends TransportFactory {

	private static final Logger log = LogManager
			.getLogger(TransportFactoryImpl.class);
	private OrbManagement orbManagement;

	protected void setProperties(Properties properties)
			throws ConfigurationException {
		log.debug("Creating OrbManagement");
		try {
			orbManagement = new OrbManagement(properties, false);
		} catch (Throwable t) {
			throw new ConfigurationException(
					"Could not create the orb management function", t);
		}
		log.debug("Created OrbManagement");
	}

	public Transport createTransport() throws ConnectionException {
		log.debug("Creating");
		TransportImpl instance = null;
		try {
			instance = new TransportImpl(orbManagement);
		} catch (Throwable t) {
			throw new ConnectionException(-1, "Could not connect to server", t);
		}
		log.debug("Created");
		return instance;
	}
}