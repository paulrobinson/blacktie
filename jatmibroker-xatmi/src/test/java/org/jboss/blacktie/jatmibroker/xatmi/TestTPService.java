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

import junit.framework.TestCase;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.RunServer;
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;

public class TestTPService extends TestCase {
	private static final Logger log = LogManager.getLogger(TestTPService.class);
	private RunServer server = new RunServer();
	private Connection connection;
	private int sendlen;
	private Buffer sendbuf;

	public void setUp() throws ConnectionException, ConfigurationException {
		server.serverinit();
		server.tpadvertiseTestTPService();

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();

		sendlen = "TestTPService".length() + 1;
		sendbuf = new X_OCTET();
		sendbuf.setData("TestTPService".getBytes());
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.serverdone();
	}

	public void test_tpservice_notpreturn() {
		log.info("test_tpservice_notpreturn");
		try {
			connection.tpcall(server.getServiceNameTestTPService(), sendbuf,
					sendlen, 0);
			fail("Managed call");
		} catch (ConnectionException e) {
			assertTrue("Error was: " + e.getTperrno(),
					e.getTperrno() == Connection.TPESVCERR);
		}
	}
}
