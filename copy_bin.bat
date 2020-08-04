@cd /d %~dp0
@setlocal

@mkdir bin

@set BUILDTYPE=Debug
::@set BUILDTYPE=Release

@del bin\zlib.dll
@copy prebuild\zlib\lib\%BUILDTYPE%\zlib.dll bin\zlib.dll

@del bin\zip.dll
@copy prebuild\libzip\lib\%BUILDTYPE%\zip.dll bin\zip.dll

@del bin\ogg.dll
@copy prebuild\libogg\lib\%BUILDTYPE%\ogg.dll bin\ogg.dll

@del bin\vorbis.dll
@del bin\vorbisfile.dll
@copy prebuild\libvorbis\lib\%BUILDTYPE%\vorbis.dll     bin\vorbis.dll
@copy prebuild\libvorbis\lib\%BUILDTYPE%\vorbisfile.dll bin\vorbisfile.dll

@del bin\freetype.dll
@copy prebuild\freetype\lib\%BUILDTYPE%\freetype.dll bin\freetype.dll

@del bin\fancy2d.dll
@copy build\fancy2d\%BUILDTYPE%\fancy2d.dll bin\fancy2d.dll

@del bin\eyes2d.dll
@copy build\eyes2d\%BUILDTYPE%\eyes2d.dll bin\eyes2d.dll

@del bin\lua51.dll
@copy prebuild\luajit\lib\%BUILDTYPE%\lua51.dll bin\lua51.dll

@del bin\LuaSTG.exe
@copy build\LuaSTG\%BUILDTYPE%\LuaSTG.exe bin\LuaSTG.exe

@endlocal
