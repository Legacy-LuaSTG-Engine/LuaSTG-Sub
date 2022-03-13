
set(fancylib_src
    fcyType.h
    fcyResult.h
    fcyRefObj.h
    fcyRefObj.cpp

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
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX fancylib FILES ${fancylib_src})

add_library(fancylib STATIC)
luastg_target_common_options(fancylib)
target_include_directories(fancylib PUBLIC
    .
)
target_sources(fancylib PRIVATE ${fancylib_src})
