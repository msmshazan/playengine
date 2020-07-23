@echo off
set PATH=%PATH%;%cd%\tools\;
set CompilerFlags= /FC /nologo /Z7 /EHsc /wd4003 /wd4551 /MT
set LinkerFlags=-subsystem:windows
set bits=x64
set LibraryLocation=..\deps\libs\%bits%\
set LinkLibraries= advapi32.lib gdi32.lib winmm.lib user32.lib Shcore.lib kernel32.lib bgfx-shared-libRelease.lib
mkdir build > NUL 2> NUL

IF NOT DEFINED vcvars_called (
      pushd %cd%
	  set vcvars_called=1
	  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" %bits% > NUL 2>NUL 
	  popd )

cd build
del *.pdb > NUL 2> NUL
shaderc -f ..\code\vs_texquad.sc -o vs_texquad.bin -i ..\deps\include\bgfx --type v --platform windows --debug --disasm --profile vs_5_0 -O 3  --varyingdef ..\code\varying.def.sc
shaderc -f ..\code\fs_texquad.sc -o fs_texquad.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\varying.def.sc
shaderc -f ..\code\vs_imgui.sc -o vs_imgui.bin -i ..\deps\include\bgfx --type v --platform windows --debug --disasm --profile vs_5_0 -O 3  --varyingdef ..\code\imgui_varying.def.sc
shaderc -f ..\code\fs_imgui.sc -o fs_imgui.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\imgui_varying.def.sc
shaderc -f ..\code\fs_textilemap.sc -o fs_textilemap.bin -i ..\deps\include\bgfx --type f --platform windows --debug --disasm --profile ps_5_0 -O 3 --varyingdef ..\code\varying.def.sc
ctime -begin playground.ctm
cl %CompilerFlags% ..\code\main.cpp  /I..\deps\include /link -incremental:no /LIBPATH:%LibraryLocation%  %LinkLibraries% %LinkerFlags% -out:playengine.exe
REM "C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\gflags.exe" /p /enable playengine.exe /full
xcopy %LibraryLocation%*.dll . /Q /Y
..\tools\TexturePacker.exe ATLAS ..\assets\images -j -u -v -x -b -p 
..\tools\TexturePacker.exe Test ..\assets\test -j -u -v -x -b -p -r
ctime -end playground.ctm
cd ..
