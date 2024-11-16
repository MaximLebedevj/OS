@echo off

set "repo_path=C:\Users\Maxim\OS\shared-repo"

setlocal enabledelayedexpansion

:loop
set "command=git --git-dir=%repo_path%\.git remote show origin"

set "state="

for /f "tokens=* delims=" %%a in ('%command%') do (
    set "line=%%a"
    echo !line! | find "main pushes to main" >nul
    if not errorlevel 1 (
        for /f "tokens=2 delims=()" %%b in ("!line!") do (
            set "state=%%b"
            goto :check_state
        )
    )
)

:check_state
if "%state%"=="local out of date" (
    git --git-dir=%repo_path%\.git --work-tree=%repo_path% pull origin main
) else (
    echo 
)

timeout /t 10 >nul

goto :loop

