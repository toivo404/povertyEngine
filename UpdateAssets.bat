@echo off
REM Path to the folder to delete
set TARGET_FOLDER=bin\Debug-windows-x86_64\Game\assets
REM Path to the folder to copy from
set SOURCE_FOLDER=assets

REM Display a message to the user
echo ========================================
echo Starting folder replacement operation...
echo ========================================
echo Target folder: %TARGET_FOLDER%
echo Source folder: %SOURCE_FOLDER%

REM Delete the target folder and its contents
if exist "%TARGET_FOLDER%" (
    echo Deleting target folder: %TARGET_FOLDER%
    rd /s /q "%TARGET_FOLDER%"
) else (
    echo Target folder does not exist: %TARGET_FOLDER%
)

REM Replace the folder with the source folder
echo Copying contents from source folder...
xcopy "%SOURCE_FOLDER%" "%TARGET_FOLDER%" /e /i /q

REM Inform the user of completion
echo ========================================
echo Folder replacement complete!
echo ========================================

REM Pause for 1 second to keep the window visible
PAUSE