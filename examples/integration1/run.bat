@echo off

rem RUN THE INTEGRATION 1 EXAMPLE
echo "Running integration 1 XATMI"
cd %BLACKTIE_HOME%\examples\integration1\xatmi_service\
call generate_server -Dservice.names=CREDIT,DEBIT -Dserver.includes="CreditService.c,DebitService.c"
IF %ERRORLEVEL% NEQ 0 exit -1
call btadmin startup
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
call generate_client -Dclient.includes=client.c 
.\client 
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_service\
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\integration1\ejb
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn install
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
call generate_client -Dclient.includes=client.c 
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn jboss:deploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn jboss:deploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\client\
@ping 127.0.0.1 -n 10 -w 1000 > nul
.\client
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\xatmi_adapter\ear\
call mvn jboss:undeploy
IF %ERRORLEVEL% NEQ 0 exit -1
cd %BLACKTIE_HOME%\examples\integration1\ejb\ear
call mvn jboss:undeploy
IF %ERRORLEVEL% NEQ 0 exit -1