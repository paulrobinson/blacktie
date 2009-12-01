package org.jboss.blacktie.jatmibroker.xatmi;

import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;

public class TestTPDisconService implements BlacktieService {
	private static final Logger log = LogManager
			.getLogger(TestTPDisconService.class);

	public Response tpservice(TPSVCINFO svcinfo) {
		log.info("testtpdiscon_service");
		try {
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			log.error("Was interrupted");
		}
		return null;
	}
}
