cmake_minimum_required(VERSION 3.15)

add_library(fancylib)

target_compile_options(fancylib PUBLIC
    "/utf-8"
    "/MP4"
    "/std:c++17"
)

target_compile_definitions(fancylib PRIVATE
    UNICODE
    _UNICODE
    ZLIB_DLL
)

target_include_directories(fancylib PRIVATE
    .
    ../prebuild/zlib/include
)

target_sources(fancylib PRIVATE
    fcyType.h
    fcyRefObj.h
    
    fcyException.h
    fcyException.cpp
    
    fcyMath.h
    
    fcyFile/fcyResPackage.h
    fcyFile/fcyResPackage.cpp
    
    fcyIO/fcyStream.h
    fcyIO/fcyStream.cpp
    fcyIO/fcyDeflate.h
    fcyIO/fcyDeflate.cpp
    fcyIO/fcyBinaryHelper.h
    fcyIO/fcyBinaryHelper.cpp
    
    fcyMisc/fcyFunctor.h
    fcyMisc/fcyHash.h
    fcyMisc/fcyHash.cpp
    fcyMisc/fcyRandom.h
    fcyMisc/fcyRandom.cpp
    fcyMisc/fcyStopWatch.h
    fcyMisc/fcyStopWatch.cpp
    fcyMisc/fcyStringHelper.h
    fcyMisc/fcyStringHelper.cpp
    
    fcyOS/fcyMemPool.h
    fcyOS/fcyCPUID.h
    fcyOS/fcyCPUID.cpp
    fcyOS/fcyDebug.h
    fcyOS/fcyDebug.cpp
    fcyOS/fcyMultiThread.h
    fcyOS/fcyMultiThread.cpp
    fcyOS/fcySocket.h
    fcyOS/fcySocket.cpp
    
    fcyParser/fcyLexicalParser.h
    fcyParser/fcyLexicalParser.cpp
    fcyParser/fcyIni.h
    fcyParser/fcyIni.cpp
    fcyParser/fcyJson.h
    fcyParser/fcyJson.cpp
    fcyParser/fcyPathParser.h
    fcyParser/fcyPathParser.cpp
    fcyParser/fcyXml.h
    fcyParser/fcyXml.cpp
)
