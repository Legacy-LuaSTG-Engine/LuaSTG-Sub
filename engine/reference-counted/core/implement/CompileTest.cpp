#include "core/ReferenceCounted.hpp"
#include "core/WeakReference.hpp"
#include "core/WeakReferenceSource.hpp"
#include "core/SmartReference.hpp"

#include "core/implement/ReferenceCounted.hpp"
#include "core/implement/WeakReference.hpp"
#include "core/implement/WeakReferenceSource.hpp"

namespace {
	struct CORE_NO_VIRTUAL_TABLE IAlpha : core::IReferenceCounted {
		virtual void print() = 0;
	};

	struct CORE_NO_VIRTUAL_TABLE IResource : core::IWeakReferenceSource {
		virtual void download() = 0;
	};
}

namespace core {
	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/unused.IAlpha
	template<> constexpr InterfaceId getInterfaceId<IAlpha>() { return UUID::parse("b009fba2-0cc7-5a14-b703-d908bfd9547b"); }

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/unused.IResource
	template<> constexpr InterfaceId getInterfaceId<IResource>() { return UUID::parse("b7c0c21c-46fd-5a06-b8d2-637bb0281325"); }
}

namespace {
	class Alpha final : public core::implement::ReferenceCounted<IAlpha> {
	public:
		Alpha() = default;
		Alpha(Alpha const&) = delete;
		Alpha(Alpha&&) = delete;
		~Alpha() override = default;

		Alpha& operator=(Alpha const&) = delete;
		Alpha& operator=(Alpha&&) = delete;

		void print() override {}
	};

	class Resource final : public core::implement::WeakReferenceSource<IResource> {
	public:
		Resource() = default;
		Resource(Resource const&) = delete;
		Resource(Resource&&) = delete;
		~Resource() override = default;

		Resource& operator=(Resource const&) = delete;
		Resource& operator=(Resource&&) = delete;

		void download() override {}
	};
}

namespace {
	[[maybe_unused]] void testReferenceCounted() {
		core::SmartReference<IAlpha> alpha;
		alpha.attach(new Alpha());

		core::SmartReference<core::IReferenceCounted> a;
		alpha->queryInterface(a.put());
		alpha->retain();
		alpha->release();
		alpha->print();
	}

	[[maybe_unused]] void testWeakReferenceSource() {
		core::SmartReference<IResource> resource;
		resource.attach(new Resource());

		core::SmartReference<core::IWeakReferenceSource> a;
		resource->queryInterface(a.put());
		core::SmartReference<core::IReferenceCounted> b;
		resource->queryInterface(b.put());
		resource->retain();
		resource->release();
		resource->download();

		core::SmartReference<core::IWeakReference> weak;
		resource->getWeakReference(weak.put());

		core::SmartReference<core::IReferenceCounted> c;
		weak->queryInterface(c.put());

		core::SmartReference<IResource> strong;
		weak->resolve(strong.put());
	}
}
