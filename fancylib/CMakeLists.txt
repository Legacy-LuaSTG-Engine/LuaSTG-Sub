
set(fancylib_src
    fcyType.h
    fcyResult.h
    fcyRefObj.h
    fcyRefObj.cpp

    fcyException.h
    fcyException.cpp

    fcyMath.h

    fcyIO/fcyStream.h
    fcyIO/fcyStream.cpp
    fcyIO/fcyBinaryHelper.h
    fcyIO/fcyBinaryHelper.cpp

    fcyMisc/fcyRandom.h
    fcyMisc/fcyRandom.cpp
    fcyMisc/fcyStopWatch.h
    fcyMisc/fcyStopWatch.cpp

    fcyOS/fcyMemPool.h
)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} PREFIX fancylib FILES ${fancylib_src})

add_library(fancylib STATIC)
luastg_target_common_options(fancylib)
luastg_target_more_warning(fancylib)
target_include_directories(fancylib PUBLIC
    .
)
target_sources(fancylib PRIVATE ${fancylib_src})
