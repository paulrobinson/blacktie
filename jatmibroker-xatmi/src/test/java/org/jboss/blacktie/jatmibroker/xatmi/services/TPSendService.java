package org.jboss.blacktie.jatmibroker.xatmi.services;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.jboss.blacktie.jatmibroker.xatmi.Response;
import org.jboss.blacktie.jatmibroker.xatmi.Service;
import org.jboss.blacktie.jatmibroker.xatmi.TPSVCINFO;

public class TPSendService implements Service {
	private static final Logger log = LogManager.getLogger(TPSendService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpsend_service");
		return null;
	}
}
