#pragma once

namespace imgui {
	void bindEngine();
	void unbindEngine();

	void cancelSetCursor();
	void updateEngine(bool allow_set_cursor);
	void drawEngine();

	void showTestInputWindow(bool* p_open = nullptr);
};
