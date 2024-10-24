#include <catch2/catch_test_macros.hpp>
#include <darmok/scene_filter.hpp>
#include <string>
#include <glm/glm.hpp>
#include <bgfx/bgfx.h>
#include <fstream>

using namespace darmok;

TEST_CASE("EntityFilter stores operators", "[scene-filter]")
{
	auto filter = EntityFilter(1) & 2;
	REQUIRE(filter.toString() == "1 && 2");
	filter |= EntityFilter::negate(3);
	REQUIRE(filter.toString() == "( 1 && 2 ) || !3");
	filter = EntityFilter::negate(EntityFilter(4) & 5);
	REQUIRE(filter.toString() == "!( 4 && 5 )");
	filter = EntityFilter(1) & 2 | 3;
	REQUIRE(filter.toString() == "( 1 && 2 ) || 3");
	filter &= EntityFilter::negate(EntityFilter(4) & 5);
	REQUIRE(filter.toString() == "( ( 1 && 2 ) || 3 ) && !( 4 && 5 )");

	filter = EntityFilter({ 1, 2, 3 }, EntityFilterOperation::Not);
	REQUIRE(filter.toString() == "!1 || !2 || !3");
}

struct Comp1 {};
struct Comp2 {};
struct Comp3 {};

TEST_CASE("EntityView simple", "[scene-filter]")
{
	EntityRegistry registry;
	auto entity1 = registry.create();
	auto entity2 = registry.create();
	registry.emplace<Comp1>(entity1);
	registry.emplace<Comp2>(entity2);

	auto filter = EntityFilter(entt::type_hash<Comp2>::value());
	EntityView view(registry, filter);

	REQUIRE(!view.contains(entity1));
	REQUIRE(view.contains(entity2));

	auto i = 0;
	for (auto entity : view)
	{
		REQUIRE(entity == entity2);
		++i;
	}
	REQUIRE(i == 1);
}

TEST_CASE("EntityView or", "[scene-filter]")
{
	EntityRegistry registry;
	auto entity1 = registry.create();
	auto entity2 = registry.create();
	registry.emplace<Comp1>(entity1);
	registry.emplace<Comp2>(entity2);

	auto filter = EntityFilter::create<Comp1>() | EntityFilter::create<Comp2>();
	EntityView view(registry, filter);

	REQUIRE(view.contains(entity1));
	REQUIRE(view.contains(entity2));

	std::vector<Entity> v(view.begin(), view.end());
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == entity2);
	REQUIRE(v[1] == entity1);

	v.clear();
	v.insert(v.end(), view.rbegin(), view.rend());
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == entity1);
	REQUIRE(v[1] == entity2);
}

TEST_CASE("EntityView and", "[scene-filter]")
{
	EntityRegistry registry;
	auto entity1 = registry.create();
	auto entity2 = registry.create();
	registry.emplace<Comp1>(entity1);
	registry.emplace<Comp1>(entity2);
	registry.emplace<Comp2>(entity2);

	auto filter = EntityFilter::create<Comp1>() & EntityFilter::create<Comp2>();
	EntityView view(registry, filter);

	std::vector<Entity> v(view.begin(), view.end());
	REQUIRE(v.size() == 1);
	REQUIRE(v[0] == entity2);
}

TEST_CASE("EntityView not", "[scene-filter]")
{
	EntityRegistry registry;
	auto entity1 = registry.create();
	auto entity2 = registry.create();
	registry.emplace<Comp1>(entity1);
	registry.emplace<Comp1>(entity2);
	registry.emplace<Comp2>(entity2);

	auto filter = EntityFilter::create<Comp2>().negate();
	EntityView view(registry, filter);

	std::vector<Entity> v(view.begin(), view.end());
	REQUIRE(v.size() == 1);
	REQUIRE(v[0] == entity1);
}

TEST_CASE("EntityView complex", "[scene-filter]")
{

	EntityRegistry registry;
	auto entity1 = registry.create();
	auto entity2 = registry.create();
	auto entity3 = registry.create();
	auto entity4 = registry.create();
	registry.emplace<Comp1>(entity1);
	registry.emplace<Comp3>(entity1);
	registry.emplace<Comp1>(entity2);
	registry.emplace<Comp2>(entity2);
	registry.emplace<Comp3>(entity2);
	registry.emplace<Comp2>(entity3);
	registry.emplace<Comp3>(entity3);
	registry.emplace<Comp1>(entity4);

	auto filter = (EntityFilter::create<Comp1>() & (EntityFilter::create<Comp2>() | EntityFilter::create<Comp3>().negate()));
	EntityView view(registry, filter);

	std::vector<Entity> v(view.begin(), view.end());
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == entity4);
	REQUIRE(v[1] == entity2);
}