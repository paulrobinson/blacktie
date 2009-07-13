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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <iostream>

#include "AtmiBrokerEnv.h"
#include "AtmiBrokerEnvXml.h"
#include "log4cxx/logger.h"
#include "ace/OS_NS_stdlib.h"
#include "ace/OS_NS_stdio.h"
#include "ace/OS_NS_string.h"
#include "ace/Default_Constants.h"

log4cxx::LoggerPtr loggerAtmiBrokerEnv(log4cxx::Logger::getLogger(
		"AtmiBrokerEnv"));

int AtmiBrokerEnv::ENV_VARIABLE_SIZE = 30;
char *AtmiBrokerEnv::ENVIRONMENT_FILE = (char*) "Environment.xml";
char *AtmiBrokerEnv::ENVIRONMENT_DIR = NULL;
AtmiBrokerEnv *AtmiBrokerEnv::ptrAtmiBrokerEnv = NULL;

AtmiBrokerEnv *
AtmiBrokerEnv::get_instance() {
	if (ptrAtmiBrokerEnv == NULL)
		ptrAtmiBrokerEnv = new AtmiBrokerEnv();
	return ptrAtmiBrokerEnv;
}

void AtmiBrokerEnv::discard_instance() {
	if (ptrAtmiBrokerEnv != NULL) {
		delete ptrAtmiBrokerEnv;
		ptrAtmiBrokerEnv = NULL;
		ENVIRONMENT_DIR = NULL;
	}
}

void AtmiBrokerEnv::set_environment_dir(const char* dir) {
	if(dir == NULL) {
		ENVIRONMENT_DIR = NULL;
	} else {
		ENVIRONMENT_DIR = strdup(dir);
	}
}

AtmiBrokerEnv::AtmiBrokerEnv() {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "constructor");
	readEnvironment = false;

	char* ptrDir = NULL;
	ptrDir = ACE_OS::getenv("BLACKTIE_CONFIGURATION_DIR");

	if(ptrDir != NULL && ENVIRONMENT_DIR == NULL){
		AtmiBrokerEnv::set_environment_dir(ptrDir);
	}

	if(ENVIRONMENT_DIR) {
		char aEnvFileName[256];

		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "read env from dir: " << ENVIRONMENT_DIR);
		ACE_OS::snprintf(aEnvFileName, 256, "%s"ACE_DIRECTORY_SEPARATOR_STR_A"%s",
						 ENVIRONMENT_DIR,
						 ENVIRONMENT_FILE);
		readenv(aEnvFileName, NULL);
	} else {
		readenv(NULL, NULL);
	}
}

AtmiBrokerEnv::~AtmiBrokerEnv() {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "destructor");
	for (std::vector<envVar_t>::iterator i = envVariableInfoSeq.begin(); i
			!= envVariableInfoSeq.end(); i++) {
		free((*i).name);
		free((*i).value);
	}

	if(ENVIRONMENT_DIR) {
		free(ENVIRONMENT_DIR);
	}
	envVariableInfoSeq.clear();

	//free(namingServiceId);
	//free(transFactoryId);

	if (xarmp)
		xarmp = xarmp->head;

	while (xarmp) {
		xarm_config_t * next = xarmp->next;

		free(xarmp->resourceName);
		free(xarmp->openString);
		free(xarmp->closeString);
		free(xarmp->xasw);
		free(xarmp->xalib);

		free(xarmp);

		xarmp = next;
	} 
	xarmp = 0;

	if (servers.size() != 0) {
		for (ServersInfo::iterator server = servers.begin(); server != servers.end(); server++) {
			free((*server)->serverName);

			std::vector<ServiceInfo>* services = &(*server)->serviceVector;
			for(std::vector<ServiceInfo>::iterator i = services->begin(); i != services->end(); i++) {
				free((*i).serviceName);
				free((*i).transportLib);
				free((*i).function_name);
				free((*i).library_name);
			}
			services->clear();

			free(*server);
		}
		servers.clear();
	}

	readEnvironment = false;
}

char*
AtmiBrokerEnv::getTransportLibrary(char* serviceName) {
	if (servers.size() != 0) {
		for (ServersInfo::iterator server = servers.begin(); server != servers.end(); server++) {
			std::vector<ServiceInfo>* services = &(*server)->serviceVector;
			for(std::vector<ServiceInfo>::iterator i = services->begin(); i != services->end(); i++) {
				if(ACE_OS::strncmp((*i).serviceName, serviceName, 15) == 0){
					return (*i).transportLib;
				}
			}
		}
	}

	return NULL;
}

char*
AtmiBrokerEnv::getenv(char* anEnvName) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "getenv %s" << anEnvName);

	char *envValue = ::getenv(anEnvName);
	if (envValue != NULL) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "getenv env is %s"
				<< envValue);
		return envValue;
	}

	for (std::vector<envVar_t>::iterator i = envVariableInfoSeq.begin(); i
			!= envVariableInfoSeq.end(); i++) {
		if (strcmp(anEnvName, (*i).name) == 0) {
			LOG4CXX_DEBUG(loggerAtmiBrokerEnv,
					(char*) "getenv found env name '%s'" << (*i).value);
			return (*i).value;
		}
	}
	return NULL;
}

int AtmiBrokerEnv::putenv(char* anEnvNameValue) {
	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "putenv %s" << anEnvNameValue);

	::putenv(anEnvNameValue);

	int size = strlen(anEnvNameValue);

	char * name = (char*) malloc(sizeof(char) * size);
	memset(name, '\0', size);
	char * value = (char*) malloc(sizeof(char) * size);
	memset(value, '\0', size);

	int i = 0;
	for (; i < size; i++) {
		if (anEnvNameValue[i] == '=')
			break;
		else
			name[i] = anEnvNameValue[i];
	}
	int j = 0;
	i++;
	for (; i < size; i++, j++) {
		value[j] = anEnvNameValue[i];
	}

	LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "putenv name '%s' value '%s'"
			<< name << value);

	envVar_t envVar;
	envVar.name = strdup(name);
	envVar.value = strdup(value);
	envVariableInfoSeq.push_back(envVar);

	free(name);
	free(value);

	return 1;
}

int AtmiBrokerEnv::readenv(char* aEnvFileName, char* label) {
	if (!readEnvironment) {
		LOG4CXX_DEBUG(loggerAtmiBrokerEnv,
				(char*) "readenv ignores label variable");
		char* descPath;

		if (aEnvFileName != NULL) {
			descPath = aEnvFileName;
		} else {
			descPath = ENVIRONMENT_FILE; 
		}

		LOG4CXX_DEBUG(loggerAtmiBrokerEnv, (char*) "readenv file: " << descPath);

		AtmiBrokerEnvXml aAtmiBrokerEnvXml;
		if (aAtmiBrokerEnvXml.parseXmlDescriptor(&envVariableInfoSeq, descPath)) {
			readEnvironment = true;
		} else {
			LOG4CXX_ERROR(loggerAtmiBrokerEnv, (char*) "can not read " << descPath);
			throw std::exception();
			//abort();
			//return -1;
		}
	}
	return 1;
}

std::vector<envVar_t>& AtmiBrokerEnv::getEnvVariableInfoSeq() {
	return envVariableInfoSeq;
}

