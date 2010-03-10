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
package org.jboss.blacktie.btadmin;

import java.io.IOException;

import javax.management.MBeanServerConnection;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

import junit.framework.TestCase;

public class ShutdownTest extends TestCase {
	public void testShutdownWithoutArgs() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown";
		if (handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithoutId() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown default";
		if (handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithNonIntId() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown default one";
		if (handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	public void testShutdownWithoutServerName() throws IOException,
			MalformedObjectNameException, NullPointerException {
		String command = "shutdown 1";
		if (handleCommand(command.split(" "))) {
			fail("Command was successful");
		}
	}

	private boolean handleCommand(String[] command) throws IOException,
			MalformedObjectNameException, NullPointerException {
		String url = "service:jmx:rmi:///jndi/rmi://localhost:1090/jmxconnector";
		String mbeanName = "jboss.blacktie:service=Admin";
		// Initialize the connection to the mbean server
		JMXServiceURL u = new JMXServiceURL(url);
		JMXConnector c = JMXConnectorFactory.connect(u);
		MBeanServerConnection beanServerConnection = c
				.getMBeanServerConnection();
		ObjectName blacktieAdmin = new ObjectName(mbeanName);

		CommandHandler commandHandler = new CommandHandler(
				beanServerConnection, blacktieAdmin);
		return commandHandler.handleCommand(command);
	}

}
