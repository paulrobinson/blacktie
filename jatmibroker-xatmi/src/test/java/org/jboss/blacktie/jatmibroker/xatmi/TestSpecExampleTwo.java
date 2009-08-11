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
import org.jboss.blacktie.jatmibroker.core.conf.ConfigurationException;
import org.jboss.blacktie.jatmibroker.core.server.AtmiBrokerServer;

public class TestSpecExampleTwo extends TestCase {
	private static final Logger log = LogManager
			.getLogger(TestSpecExampleTwo.class);
	private AtmiBrokerServer server;
	private Connection connection;

	public void setUp() throws ConnectionException, ConfigurationException {
		this.server = new AtmiBrokerServer("standalone-server", null);
		this.server.tpadvertise("TestOne", TestSpecExampleTwoService.class
				.getName());

		ConnectionFactory connectionFactory = ConnectionFactory
				.getConnectionFactory();
		connection = connectionFactory.getConnection();
	}

	public void tearDown() throws ConnectionException, ConfigurationException {
		connection.close();
		server.close();
	}

	public void test() throws ConnectionException {
		log.info("TestSpecExampleTwo::test_specexampletwo");
		long len = 0;
		// TODO long event = 0; /* X_C_TYPE and subtype inq_buf. The structure
		// */
		Session cd; /* contains a character array named input and an */
		/* array of integers named output. */
		/* allocate typed buffer */
		Buffer ptr = new Buffer("X_C_TYPE", "inq_buf");
		ptr.format(new String[] { "input", "output", "failTest" }, new Class[] {
				char[].class, int.class, int.class }, new int[] { 100, 0, 0 });

		/* populate typed buffer with input data */
		ptr.setString("input",
				"retrieve all accounts with balances less than 0");
		// TODO tx_begin(); /* start global transaction */
		/* connect to conversational service, send input data, & yield control */
		cd = connection.tpconnect("TestOne", ptr, 0, Connection.TPRECVONLY
				| Connection.TPSIGRSTRT);
		do {

			try {
				/* receive 10 account records at a time */
				ptr = cd.tprecv(Connection.TPSIGRSTRT);
				/*
				 * Format & display in AP-specific manner the accounts returned.
				 */
			} catch (ConnectionException e) {
				if (e.getTperrno() == Connection.TPEEVENT
						&& e.getEvent() == Connection.TPEV_SVCSUCC) {
					break;
				} else {
					fail("Got unexpected error: " + e.getTperrno()
							+ " with event: " + e.getEvent());
				}
			}

		} while (true);
		// TODO
		// if (event == TPEV_SVCSUCC)
		// tx_commit(); /* commit global transaction */
		// else
		// tx_rollback(); /* rollback global transaction */

	}
}