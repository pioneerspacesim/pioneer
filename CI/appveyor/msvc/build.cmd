@echo on
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

mkdir c:\projects\pioneer\build || goto error
cd c:\projects\pioneer\build || goto error

cmake -G Ninja^
 -DCMAKE_INSTALL_PREFIX="C:/Program Files/Pioneer"^
 -DPIONEER_DATA_DIR="C:/Program Files/Pioneer/data"^
 -DCMAKE_BUILD_TYPE:STRING=Release^
 -DCMAKE_INSTALL_MESSAGE=NEVER^
 -DGIT_EXECUTABLE="c:/Program Files/Git/cmd/git.exe"^
 c:\projects\pioneer || goto error

cmake --build . || goto error
cd ..
modelcompiler.exe -b inplace || goto error
cd build
cmake --build . --target install || goto error
cmake --build . --target win-installer || goto error

if %APPVEYOR_REPO_TAG%==true call publish.cmd

goto success

:error
exit 1

:success
