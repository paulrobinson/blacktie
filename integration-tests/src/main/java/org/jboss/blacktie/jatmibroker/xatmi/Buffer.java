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

import java.io.Serializable;

/**
 * This class encapsulates the response from the remote service and the return
 * code
 */
public class Buffer implements Serializable {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * The response from the server
	 */
	private byte[] data;

	private String type;

	private String subtype;

	/**
	 * Create a new buffer
	 * 
	 * @param data
	 * @param len
	 * @param type
	 * @param subtype
	 */
	public Buffer(String type, String subtype) {
		this.type = type;
		this.subtype = subtype;
	}

	/**
	 * Get the return value
	 * 
	 * @return The return value
	 */
	public String getType() {
		return type;
	}

	/**
	 * Get the return code
	 * 
	 * @return The return code
	 */
	public String getSubtype() {
		return subtype;
	}

	/**
	 * Get the data
	 * 
	 * @return The data
	 */
	public byte[] getData() {
		return data;
	}

	/**
	 * Set the data to send.
	 * 
	 * @param data
	 *            The data
	 */
	public void setData(byte[] data) {
		this.data = data;
	}
}