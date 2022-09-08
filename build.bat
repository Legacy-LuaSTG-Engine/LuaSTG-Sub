@cd %~dp0
@echo %cd%
@setlocal
    :: values
    @set SOURCE=%cd%
    @set BUILD=%cd%\build
    @set BUILD32=%BUILD%\x86
    @set BUILD64=%BUILD%\amd64
    
    :: build and install directory
    mkdir %BUILD%
    mkdir %BUILD32%
    mkdir %BUILD64%
    
    :: x86
    @echo ============================ build Win32 ===========================
    cmake -S %SOURCE% -B %BUILD32% -G "Visual Studio 17 2022" -T v143 -A Win32 -DCMAKE_SYSTEM_VERSION="10.0.22000.0"
    cmake --build %BUILD32% --target LuaSTG --config Release --clean-first
    ::cmake --build %BUILD32% --target ALL_BUILD --config Release        --clean-first
    ::cmake --build %BUILD32% --target ALL_BUILD --config Debug          --clean-first
    ::cmake --build %BUILD32% --target ALL_BUILD --config RelWithDebInfo --clean-first
    ::cmake --build %BUILD32% --target ALL_BUILD --config MinSizeRel     --clean-first
    
    :: amd64
    @echo ============================= build x64 ============================
    cmake -S %SOURCE% -B %BUILD64% -G "Visual Studio 17 2022" -T v143 -A x64 -DCMAKE_SYSTEM_VERSION="10.0.22000.0"
    cmake --build %BUILD64% --target LuaSTG --config Release --clean-first
    ::cmake --build %BUILD64% --target ALL_BUILD --config Release        --clean-first
    ::cmake --build %BUILD64% --target ALL_BUILD --config Debug          --clean-first
    ::cmake --build %BUILD64% --target ALL_BUILD --config RelWithDebInfo --clean-first
    ::cmake --build %BUILD64% --target ALL_BUILD --config MinSizeRel     --clean-first
    
    @echo ============================= build finish ============================
@endlocal
