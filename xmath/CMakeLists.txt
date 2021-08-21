add_library(xmath STATIC)
luastg_target_common_options(xmath)
target_include_directories(xmath PUBLIC
    .
    math
    xmath
)
target_sources(xmath PRIVATE
    math/Vec2.h
    math/Vec2.cpp
    xmath/meow_fft.c
    xmath/meow_fft.h
    xmath/XCollision.cpp
    xmath/XCollision.h
    xmath/XComplex.cpp
    xmath/XComplex.h
    xmath/XConstant.h
    xmath/XDistance.cpp
    xmath/XDistance.h
    xmath/XEquation.cpp
    xmath/XEquation.h
    xmath/XFFT.cpp
    xmath/XFFT.h
    xmath/XIntersect.cpp
    xmath/XIntersect.h
    xmath/XMath.h
    xmath/XRandom.cpp
    xmath/XRandom.h
    xmath/XSpline.cpp
    xmath/XSpline.h
    xmath/XTween.cpp
    xmath/XTween.h
)
