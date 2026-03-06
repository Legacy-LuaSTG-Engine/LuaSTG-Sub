#include <stdexcept>

#include "core/ReferenceCounted.hpp"
#include "core/WeakReference.hpp"
#include "core/WeakReferenceSource.hpp"
#include "core/SmartReference.hpp"

#include "core/implement/ReferenceCounted.hpp"
#include "core/implement/WeakReference.hpp"
#include "core/implement/WeakReferenceSource.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"

#include "gtest/gtest.h"

namespace {
	CORE_INTERFACE IAlpha : core::IReferenceCounted {
		virtual void print() = 0;
	};

	CORE_INTERFACE IResource : core::IWeakReferenceSource {
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

TEST(ReferenceCounted, BasicTests) {
	core::SmartReference<IAlpha> alpha;
	alpha.attach(new Alpha());

	EXPECT_EQ(alpha->retain(), 2);
	EXPECT_EQ(alpha->release(), 1);

	{
		core::SmartReference<IAlpha> v;
		EXPECT_TRUE(alpha->queryInterface(v.put()));
	}
	{
		core::SmartReference<core::IReferenceCounted> v;
		EXPECT_TRUE(alpha->queryInterface(v.put()));
	}

	alpha->print();
}

TEST(WeakReferenceSource, BasicTests) {
	core::SmartReference<IResource> resource;
	resource.attach(new Resource());

	EXPECT_EQ(resource->retain(), 2);
	EXPECT_EQ(resource->release(), 1);

	{
		core::SmartReference<IResource> v;
		EXPECT_TRUE(resource->queryInterface(v.put()));
	}
	{
		core::SmartReference<core::IWeakReferenceSource> v;
		EXPECT_TRUE(resource->queryInterface(v.put()));
	}
	{
		core::SmartReference<core::IReferenceCounted> v;
		EXPECT_TRUE(resource->queryInterface(v.put()));
	}

	resource->download();

	core::SmartReference<core::IWeakReference> weak;
	resource->getWeakReference(weak.put());
	{
		core::SmartReference<core::IReferenceCounted> c;
		EXPECT_TRUE(weak->queryInterface(c.put()));
	}
	{
		core::SmartReference<IResource> strong;
		EXPECT_TRUE(weak->resolve(strong.put()));
	}

	resource.reset();

	{
		core::SmartReference<IResource> strong;
		EXPECT_FALSE(weak->resolve(strong.put()));
	}
}

#ifndef NDEBUG
TEST(ReferenceCountedDebugger, LeakTests) {
	EXPECT_FALSE(core::implement::ReferenceCountedDebugger::hasLeak());
	core::implement::ReferenceCountedDebugger::reportLeak();
}
#endif
