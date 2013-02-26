@echo off
echo Compilazione di EWX (utilit� ExitWindowsEx) per Windows NT/XP...
cl -nologo -FeEWX_NT.exe -O1g -DMADE_FOR_NT EWX32.c
echo Compilazione di EWX (utilit� ExitWindowsEx) per Windows 9x...
cl -nologo -FeEWX_9x.exe -O1g EWX32.c EWX16.cpp
echo Compilazione di EWX (utilit� ExitWindowsEx) per Windows Vista+...
cl -nologo -FeEWX_NEW.exe -O1g EWX32_NEW.c
del *.obj
echo.
echo Uso: EWX -logoff/-reboot/-poweroff/-shutdown/-force
echo          [-restart/-suspend/-exit (Win9x)]
echo Senza parametri, non esegue alcuna azione.
echo.
echo Su NT, il riavvio e l'arresto necessitano di apposito privilegio; lo
echo spegnimento dell'alimentazione richiede un driver APM OEM.
