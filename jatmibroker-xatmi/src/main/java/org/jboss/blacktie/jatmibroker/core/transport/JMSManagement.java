package org.jboss.blacktie.jatmibroker.core.transport;

import java.util.Properties;

import javax.jms.Connection;
import javax.jms.ConnectionFactory;
import javax.jms.Destination;
import javax.jms.JMSException;
import javax.jms.Session;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

/**
 * This class could be extended to support connection reconnection.
 */
public class JMSManagement {
	private static final Logger log = LogManager.getLogger(JMSManagement.class);

	private Properties props = new Properties();

	private Connection connection;

	private InitialContext context;

	public JMSManagement(Properties properties) throws NamingException,
			JMSException {
		log.debug("Creating JMSManagement");
		props.setProperty("java.naming.factory.initial",
				"org.jnp.interfaces.NamingContextFactory");
		props.setProperty("java.naming.factory.url.pkgs",
				"org.jboss.naming:org.jnp.interfaces");
		props.setProperty("java.naming.provider.url", (String) properties
				.get("java.naming.provider.url"));
		props.putAll(properties);
		context = new InitialContext(props);
		ConnectionFactory factory = (ConnectionFactory) context
				.lookup("ConnectionFactory");

		String username = (String) props.get("StompConnectUsr");
		String password = (String) props.get("StompConnectPwd");

		if (username != null) {
			connection = factory.createConnection(username, password);
		} else {
			connection = factory.createConnection();
		}
		connection.start();
		log.debug("Created JMSManagement");
	}

	public Session createSession() throws JMSException {
		return connection.createSession(false, Session.AUTO_ACKNOWLEDGE);
	}

	public void close() {
		try {
			connection.close();
		} catch (JMSException e) {
			log.error("Could not close the connection to the JMS server: "
					+ e.getMessage(), e);
		}
	}

	public Destination lookup(String serviceName) throws NamingException {
		return (Destination) context.lookup("/queue/" + serviceName);
	}
}