set current_dir=%~dp0
mklink /D %current_dir%\build\test\resources\ %current_dir%\resources\
mklink /D %current_dir%\build\test\sdf\resources\Release %current_dir%\resources\
pause