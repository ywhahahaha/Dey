copy /Y .\Objects\I03M.hex .\Upgrade\I03M.hex
cd .\Upgrade

SET PROJECT_NAME=I03M

del %PROJECT_NAME%_*.bin /S /Q
del %PROJECT_NAME%_*.hex /S /Q

::SET TIME_NOW=%date:~0,4%%date:~5,2%%date:~8,2%%time:~0,2%%time:~3,2%%time:~6,2%
::SET TIME_NOW=%date:~0,4%%date:~5,2%%date:~8,2%
::set fileDate2=%date:~0,4%%date:~5,2%%date:~8,2%%t:~0,4%%time:~6,2%
for /f "tokens=1,2 delims=:" %%i in ('time/t') do set t=%%i%%j
SET TIME_NOW=%date:~0,4%%date:~5,2%%date:~8,2%%t:~0,4%%time:~6,2%
SET INIT_VERSION=############
SET VERSION=V1.3.0



firmware.exe %PROJECT_NAME%.bin %INIT_VERSION% %PROJECT_NAME%_%VERSION%_%TIME_NOW%.bin 1024 0
mergehex.exe -m boot.hex %PROJECT_NAME%.hex -o %PROJECT_NAME%_%VERSION%_%TIME_NOW%.hex

copy /Y %PROJECT_NAME%_%VERSION%_%TIME_NOW%.bin G:\\DEY\\UPGRADE\\%PROJECT_NAME%_%VERSION%_%TIME_NOW%.bin