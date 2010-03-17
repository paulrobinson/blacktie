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

import javax.management.MalformedObjectNameException;

import junit.framework.TestCase;

import org.jboss.blacktie.btadmin.server.RunServer;

public class StartupTest extends TestCase {
	private RunServer runServer = new RunServer();

	private CommandHandler commandHandler;

	public void setUp() throws Exception {
		this.commandHandler = new CommandHandler();
	}

	public void tearDown() {
	}

	public void testStartup() throws IOException, MalformedObjectNameException,
			NullPointerException, InstantiationException,
			IllegalAccessException, ClassNotFoundException {
		String command = "startup";
		if (commandHandler.handleCommand(command.split(" ")) == -1) {
			fail("Command was unsuccessful");
		}
		
		// TODO CHECK THE OUTPUT BEFORE RETURNING
		try {
			Thread.currentThread().sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		command = "shutdown default";
		if (commandHandler.handleCommand(command.split(" ")) == -1) {
			fail("Command was unsuccessful");
		}
		
		// TODO SHUTDOWN SHOULD RETURN WHEN THERE ARE NO MORE CONSUMERS?
		try {
			Thread.currentThread().sleep(5000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}