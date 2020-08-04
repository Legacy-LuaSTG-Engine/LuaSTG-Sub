#pragma once

#include <string>

namespace Eyes2D {
	struct E2DException {
		long errCode;
		long errResult;//用于COM
		std::wstring errSrc;
		std::wstring errDesc;
		E2DException(long code, long result, const std::wstring& src, const std::wstring& desc) {
			errCode = code;
			errResult = result;
			errSrc = std::move(src);
			errDesc = std::move(desc);
		}
	};
}
