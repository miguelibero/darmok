#include <catch2/catch_test_macros.hpp>
#include <darmok/shape.hpp>
#include <darmok/math.hpp>
#include <string>
#include <glm/glm.hpp>

using namespace darmok;

TEST_CASE("Frustum planes are correct", "[shape]")
{
	auto proj = Math::ortho(glm::vec2(-1), glm::vec2(1), 0, 1);
	Frustum frust(proj);
	auto& near = frust.getPlane(Frustum::PlaneType::Near);
	auto& far = frust.getPlane(Frustum::PlaneType::Far);
	auto& bottom = frust.getPlane(Frustum::PlaneType::Bottom);
	auto& top = frust.getPlane(Frustum::PlaneType::Top);
	auto& left = frust.getPlane(Frustum::PlaneType::Left);
	auto& right = frust.getPlane(Frustum::PlaneType::Right);
	REQUIRE(near.constant == 1);
	REQUIRE(far.constant == 1);
	REQUIRE(bottom.constant == 1);
	REQUIRE(top.constant == 1);
	REQUIRE(left.constant == 1);
	REQUIRE(right.constant == 1);
}