@echo off

rem GENERATE AN EMPTY SERVER
cd %BLACKTIE_HOME%\examples\xatmi\library
call generate_server
IF %ERRORLEVEL% NEQ 0 exit -1

rem GENERATE A LIBRARY WITH THE BAR SERVICE IN IT
cd %BLACKTIE_HOME%\examples\xatmi\library
call generate_library -Dlibrary.includes=BarService.c
IF %ERRORLEVEL% NEQ 0 exit -1

rem START THE SERVER
set BLACKTIE_CONFIGURATION=win32
call btadmin startup
set BLACKTIE_CONFIGURATION=
IF %ERRORLEVEL% NEQ 0 exit -1

rem RUN THE C CLIENT
call generate_client -Dclient.includes=client.c
client
IF %ERRORLEVEL% NEQ 0 exit -1

cd %BLACKTIE_HOME%\examples\xatmi\fooapp
call btadmin shutdown
IF %ERRORLEVEL% NEQ 0 exit -1