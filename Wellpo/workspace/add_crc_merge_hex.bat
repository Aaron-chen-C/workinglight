
@echo off

::change to the current dir.
cd /d %cd%

::get the file name of self.
set file_name=%~nx0 
::echo %file_name%
::pause

::change file name.
set file_name=%file_name:.bat=.py%
::echo %file_name%
::pause

:::action
cls
python %file_name%
::pause

::goto action

