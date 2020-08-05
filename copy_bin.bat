@setlocal

@set DIR=%~dp0
@mkdir %DIR%\bin

@set BUILDTYPE=%1
::@set BUILDTYPE=Debug
::@set BUILDTYPE=Release

@del  %DIR%\bin\zlib.dll
@copy %DIR%\prebuild\zlib\lib\%BUILDTYPE%\zlib.dll            %DIR%\bin\zlib.dll

@del  %DIR%\bin\zip.dll
@copy %DIR%\prebuild\libzip\lib\%BUILDTYPE%\zip.dll           %DIR%\bin\zip.dll

@del  %DIR%\bin\ogg.dll
@copy %DIR%\prebuild\libogg\lib\%BUILDTYPE%\ogg.dll           %DIR%\bin\ogg.dll

@del  %DIR%\bin\vorbis.dll
@del  %DIR%\bin\vorbisfile.dll
@copy %DIR%\prebuild\libvorbis\lib\%BUILDTYPE%\vorbis.dll     %DIR%\bin\vorbis.dll
@copy %DIR%\prebuild\libvorbis\lib\%BUILDTYPE%\vorbisfile.dll %DIR%\bin\vorbisfile.dll

@del  %DIR%\bin\freetype.dll
@copy %DIR%\prebuild\freetype\lib\%BUILDTYPE%\freetype.dll    %DIR%\bin\freetype.dll

@del  %DIR%\bin\fancy2d.dll
@copy %DIR%\build\fancy2d\%BUILDTYPE%\fancy2d.dll             %DIR%\bin\fancy2d.dll

@del  %DIR%\bin\eyes2d.dll
@copy %DIR%\build\eyes2d\%BUILDTYPE%\eyes2d.dll               %DIR%\bin\eyes2d.dll

@del  %DIR%\bin\lua51.dll
@copy %DIR%\prebuild\luajit\lib\%BUILDTYPE%\lua51.dll         %DIR%\bin\lua51.dll

@del  %DIR%\bin\LuaSTG.exe
@copy %DIR%\build\LuaSTG\%BUILDTYPE%\LuaSTG.exe               %DIR%\bin\LuaSTG.exe

@del  %DIR%\bin\cjson.dll
@del  %DIR%\bin\lfs.dll
@copy %DIR%\prebuild\luaext\%BUILDTYPE%\cjson.dll             %DIR%\bin\cjson.dll
@copy %DIR%\prebuild\luaext\%BUILDTYPE%\lfs.dll               %DIR%\bin\lfs.dll

@endlocal
