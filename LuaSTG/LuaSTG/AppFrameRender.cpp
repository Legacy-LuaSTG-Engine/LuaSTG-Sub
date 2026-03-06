#include "AppFrame.h"
#include "GameResource/LegacyBlendStateHelper.hpp"

namespace luastg {
	void AppFrame::updateGraph2DBlendMode(BlendMode const blend) {
		auto const [v, b] = translateLegacyBlendState(blend);
		m_renderer->setVertexColorBlendState(v);
		m_renderer->setBlendState(b);
	}

	bool AppFrame::Render(IParticlePool* p, float hscale, float vscale) noexcept {
		assert(p);

		// 设置混合
		updateGraph2DBlendMode(p->GetBlendMode());

		// 渲染
		p->Render(hscale, vscale);
		return true;
	}

	void AppFrame::SnapShot(const char* path) noexcept {
		if (!m_swap_chain->saveSnapshotToFile(path)) {
			spdlog::error("[luastg] SnapShot: 保存截图到文件'{}'失败", path);
			return;
		}
	}
	void AppFrame::SaveTexture(const char* tex_name, const char* path) noexcept {
		core::SmartReference<IResourceTexture> resTex = LRES.FindTexture(tex_name);
		if (!resTex) {
			spdlog::error("[luastg] SaveTexture: 找不到纹理资源'{}'", tex_name);
			return;
		}
		if (!resTex->GetTexture()->saveToFile(path)) {
			spdlog::error("[luastg] SaveTexture: 保存纹理'{}'到文件'{}'失败", tex_name, path);
			return;
		}
	}
};
