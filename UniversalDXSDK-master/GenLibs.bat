@ECHO OFF

REM ~ Copyright 2015 Minmin Gong.
REM ~ This file is part of Universal DX SDK (https://github.com/gongminmin/UniversalDXSDK)
REM ~ Distributed under the GNU General Public License, version 3.0.
REM ~ (See accompanying file LICENSE)

setlocal
goto Start

:Set_Error
color 00
goto :eof

:Clear_Error
ver >nul
goto :eof

:Test_Empty
REM Tests whether the given string is not empty
call :Clear_Error
setlocal
set test=%1
if not defined test (
    call :Clear_Error
    goto Test_Empty_End
)
set test=###%test%###
set test=%test:"###=%
set test=%test:###"=%
set test=%test:###=%
if not "" == "%test%" call :Set_Error
:Test_Empty_End
endlocal
goto :eof

:DoExportDotLib
if exist %SystemRoot%\%3\%2.dll (
	copy /Y %SystemRoot%\%3\%2.dll .
	lib.exe /DEF:%1.def /MACHINE:%4
	del %2.dll
	del %1.exp
	move /Y %1.lib ..\Lib\%4
)
goto :eof

:DoExportDotA
if exist %SystemRoot%\%3\%2.dll (
	copy /Y %SystemRoot%\%3\%2.dll .
	dlltool.exe -d %1.def -l lib%1.a -m %5
	del %2.dll
	move /Y lib%1.a ..\Lib\%4
)
goto :eof

:ExportDotLib
call :DoExportDotLib %1 %2 "SysWOW64" "x86"
call :DoExportDotLib %1 %2 "System32" "x64"
goto :eof

:ExportDotA
call :DoExportDotA %1 %2 "SysWOW64" "x86" "i386"
call :DoExportDotA %1 %2 "System32" "x64" "i386:x86-64"
goto :eof

:Guess_Toolset
REM Try and guess the toolset to bootstrap the build with...
REM Sets TOOLSET to the first found toolset.
REM May also set TOOLSET_ROOT to the
REM location of the found toolset.

call :Clear_Error
call :Test_Empty %ProgramFiles%
if not errorlevel 1 set ProgramFiles=C:\Program Files

call :Clear_Error
if not "_%VS140COMNTOOLS%_" == "__" (
	set "TOOLSET=vc140"
	set "TOOLSET_ROOT=%VS140COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 14.0\VC\VCVARSALL.BAT" (
	set "TOOLSET=vc140"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 14.0\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS120COMNTOOLS%_" == "__" (
    set "TOOLSET=vc120"
	set "TOOLSET_ROOT=%VS120COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 12.0\VC\VCVARSALL.BAT" (
    set "TOOLSET=vc120"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 12.0\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS110COMNTOOLS%_" == "__" (
    set "TOOLSET=vc110"
	set "TOOLSET_ROOT=%VS110COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 11.0\VC\VCVARSALL.BAT" (
	set "TOOLSET=vc110"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 11.0\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS100COMNTOOLS%_" == "__" (
    set "TOOLSET=vc100"
	set "TOOLSET_ROOT=%VS100COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 10.0\VC\VCVARSALL.BAT" (
	set "TOOLSET=vc100"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 10.0\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS90COMNTOOLS%_" == "__" (
    set "TOOLSET=vc90"
	set "TOOLSET_ROOT=%VS90COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 9.0\VC\VCVARSALL.BAT" (
	set "TOOLSET=vc90"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 9.0\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS80COMNTOOLS%_" == "__" (
    set "TOOLSET=vc80"
	set "TOOLSET_ROOT=%VS80COMNTOOLS%..\..\VC\"
    goto :eof)
call :Clear_Error
if exist "%ProgramFiles%\Microsoft Visual Studio 8\VC\VCVARSALL.BAT" (
	set "TOOLSET=vc80"
	set "TOOLSET_ROOT=%ProgramFiles%\Microsoft Visual Studio 8\VC\"
    goto :eof)
call :Clear_Error
if not "_%VS71COMNTOOLS%_" == "__" (
    set "TOOLSET=vc71"
	set "TOOLSET_ROOT=%VS71COMNTOOLS%\..\..\VC\"
    goto :eof)
call :Clear_Error
if not "_%VCINSTALLDIR%_" == "__" (
    REM %VCINSTALLDIR% is also set for VC9 (and probably VC8)
    set "TOOLSET=vc71"
	set "TOOLSET_ROOT=%VCINSTALLDIR%\VC7\"
    goto :eof)
call :Clear_Error
if exist "gcc.exe" (
    set "TOOLSET=mingw"
	set "TOOLSET_ROOT=%FOUND_PATH%\"
    goto :eof)
call :Clear_Error
call :Error_Print "Could not find a suitable toolset."
goto :eof

:Start
set TOOLSET=

REM If no arguments guess the toolset;
REM or if first argument is an option guess the toolset;
REM otherwise the argument is the toolset to use.
call :Clear_Error
call :Test_Empty %1
if not errorlevel 1 (
    call :Guess_Toolset
    if not errorlevel 1 ( goto Setup_Toolset ) else ( goto Finish )
)

call :Clear_Error
set TOOLSET=%1
shift
goto Setup_Toolset

:Setup_Toolset
REM Setup the toolset command and options. This bit of code
REM needs to be flexible enough to handle both when
REM the toolset was guessed at and found, or when the toolset
REM was indicated in the command arguments.
REM NOTE: The strange multiple "if ?? == _toolset_" tests are that way
REM because in BAT variables are subsituted only once during a single
REM command. A complete "if ... else ..."
REM is a single command, even though it's in multiple lines here.
:Config_Toolset
if not "_%TOOLSET%_" == "_vc71_" goto Skip_VC71
if not "_%VS71COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS71COMNTOOLS%..\..\VC7\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC71
if not "_%TOOLSET%_" == "_vc80_" goto Skip_VC80
if not "_%VS80COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS80COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC80
if not "_%TOOLSET%_" == "_vc90_" goto Skip_VC90
if not "_%VS90COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS90COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC90
if not "_%TOOLSET%_" == "_vc100_" goto Skip_VC100
if not "_%VS100COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS100COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC100
if not "_%TOOLSET%_" == "_vc110_" goto Skip_VC110
if not "_%VS110COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS110COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC110
if not "_%TOOLSET%_" == "_vc120_" goto Skip_VC120
if not "_%VS120COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS120COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC120
if not "_%TOOLSET%_" == "_vc140_" goto Skip_VC140
if not "_%VS140COMNTOOLS%_" == "__" (
    set "TOOLSET_ROOT=%VS140COMNTOOLS%..\..\VC\"
    )
if not "_%TOOLSET_ROOT%_" == "__" (
    if "_%VCINSTALLDIR%_" == "__" (
        set "PATH=%TOOLSET_ROOT%bin;%PATH%"
        ) )
set "LIB_TYPE=lib"
set "_known_=1"
:Skip_VC140
if not "_%TOOLSET%_" == "_mingw_" goto Skip_MINGW
if not "_%TOOLSET_ROOT%_" == "__" (
	set "PATH=%TOOLSET_ROOT%bin;%PATH%"
)
set "LIB_TYPE=a"
set "_known_=1"
:Skip_MINGW
call :Clear_Error
if "_%_known_%_" == "__" (
	call :Error_Print "Unknown toolset: %TOOLSET%"
)
if errorlevel 1 goto Finish

echo ###
echo ### Using '%TOOLSET%' toolset.
echo ###

if not exist Lib mkdir Lib
if not exist Lib\x86 mkdir Lib\x86
if not exist Lib\x64 mkdir Lib\x64

if "_%LIB_TYPE%_" == "_lib_" (
	cd Def
	call :ExportDotLib d2d1 d2d1
	call :ExportDotLib d3d9 d3d9
	call :ExportDotLib d3d10 d3d10
	call :ExportDotLib d3d10_1 d3d10_1
	call :ExportDotLib d3d11 d3d11
	call :ExportDotLib d3d12 d3d12
	call :ExportDotLib d3dcompiler d3dcompiler_47
	call :ExportDotLib d3dcsx d3dcsx_43
	call :ExportDotLib d3dcsxd d3dcsxd_43
	call :ExportDotLib d3dx9 d3dx9_43
	call :ExportDotLib d3dx9d d3dx9d_43
	call :ExportDotLib d3dx10 d3dx10_43
	call :ExportDotLib d3dx10d d3dx10d_43
	call :ExportDotLib d3dx11 d3dx11_43
	call :ExportDotLib d3dx11d d3dx11d_43
	call :ExportDotLib d3dxof d3dxof
	call :ExportDotLib dsound dsound
	call :ExportDotLib dinput8 dinput8
	call :ExportDotLib dwrite dwrite
	call :ExportDotLib dxgi dxgi
	cd ..
	goto Finish
)
if errorlevel 1 goto Finish

if "_%LIB_TYPE%_" == "_a_" (
	cd Def
	call :ExportDotA d2d1 d2d1
	call :ExportDotA d3d9 d3d9
	call :ExportDotA d3d10 d3d10
	call :ExportDotA d3d10_1 d3d10_1
	call :ExportDotA d3d11 d3d11
	call :ExportDotA d3d12 d3d12
	call :ExportDotA d3dcompiler d3dcompiler_47
	call :ExportDotA d3dcsx d3dcsx_43
	call :ExportDotA d3dcsxd d3dcsxd_43
	call :ExportDotA d3dx9 d3dx9_43
	call :ExportDotA d3dx9d d3dx9d_43
	call :ExportDotA d3dx10 d3dx10_43
	call :ExportDotA d3dx10d d3dx10d_43
	call :ExportDotA d3dx11 d3dx11_43
	call :ExportDotA d3dx11d d3dx11d_43
	call :ExportDotA d3dxof d3dxof
	call :ExportDotA dsound dsound
	call :ExportDotA dinput8 dinput8
	call :ExportDotA dwrite dwrite
	call :ExportDotA dxgi dxgi
	cd ..
	goto Finish
)
if errorlevel 1 goto Finish

:Finish
endlocal
exit /b %ERRORLEVEL%
