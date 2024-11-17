@echo off

set "repo_path=C:\Users\Maxim\OS\shared-repo"

:loop
git --git-dir=%repo_path%\.git fetch origin main
for /f "delims=" %%i in ('git --git-dir=%repo_path%\.git rev-list --count HEAD...origin/main') do set status=%%i
echo %status%

if "%status%" NEQ "0" (
    git --git-dir=%repo_path%\.git --work-tree=%repo_path% pull origin main
) else (
    echo 
)

timeout /t 10 >nul /nobreak

goto :loop