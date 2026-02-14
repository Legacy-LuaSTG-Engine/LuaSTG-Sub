#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOSERVICE
#define NOMCX
#define NOIME

#include <Windows.h>

// Now, let's FUCK MICROSOFT

#ifdef DrawText
#undef DrawText
#endif

#ifdef CreateWindow
#undef CreateWindow
#endif

#ifdef DefWindowProc
#undef DefWindowProc
#endif

#ifdef MessageBox
#undef MessageBox
#endif

#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX
#undef NOSERVICE
#undef NOMCX
#undef NOIME
