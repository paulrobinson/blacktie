#include "log4cxx/logger.h"
#include "AtmiBrokerAdmin.h"
#include "AtmiBrokerServerControl.h"
#include "xatmi.h"
#include "userlog.h"
#include "string.h"

log4cxx::LoggerPtr loggerAtmiBrokerAdmin(log4cxx::Logger::getLogger(
			"AtmiBrokerAdmin"));

#ifdef __cplusplus
extern "C" {
#endif

void ADMIN(TPSVCINFO* svcinfo) {
	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerAdmin,
			(char*) "get request");

	char* req = svcinfo->data;
	char* toReturn = NULL;
	int   len = 1;
	toReturn = tpalloc((char*) "X_OCTET", NULL, len);
	toReturn[0] = '0';

	if(strcmp(req, "serverdone") == 0) {
		LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "get serverdone command");
		toReturn[0] = '1';
	} else if(strncmp(req, "advertise", 9) == 0) {
		LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "get advertise command");
		strtok(req, ",");
		char* svc = strtok(NULL, ",");
		LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "advertise service " << svc);
		toReturn[0] = '1';
	} else if(strncmp(req, "unadvertise", 11) == 0) {
		LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "get unadvertise command");
		strtok(req, ",");
		char* svc = strtok(NULL, ",");
		if (tpunadvertise(svc) == 0) {
			toReturn[0] = '1';
			LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "unadvertise service " << svc << " OK");
		} else {
			LOG4CXX_INFO(loggerAtmiBrokerAdmin, (char*) "unadvertise service " << svc << " FAIL");
		}
	}

	userlog(log4cxx::Level::getDebug(), loggerAtmiBrokerAdmin,
			(char*) "service done");
	tpreturn(TPSUCCESS, 0, toReturn, len, 0);
}

#ifdef __cplusplus
}
#endif
