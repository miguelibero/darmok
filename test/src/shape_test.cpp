#include <catch2/catch_test_macros.hpp>
#include <darmok/shape.hpp>
#include <darmok/math.hpp>
#include <string>
#include <glm/glm.hpp>

using namespace darmok;

TEST_CASE("Plane transform", "[shape]")
{
	Plane plane(glm::vec3(0, 0, 1), 1);
	plane *= Math::transform(glm::vec3(1, 0, 0));

	REQUIRE(plane.normal == glm::vec3(0, 0, 1));
	REQUIRE(plane.distance == 1);

	plane *= Math::transform(glm::vec3(0, 0, 3));

	REQUIRE(plane.normal == glm::vec3(0, 0, 1));
	REQUIRE(plane.distance == 4);

	auto scale = glm::scale(glm::mat4(1), glm::vec3(2, 3, 4));
	plane *= scale;

	REQUIRE(plane.normal == glm::vec3(0, 0, 4));
	REQUIRE(plane.distance == 64);

	auto roty = glm::mat4_cast(glm::quat(glm::radians(glm::vec3(0, -90, 0))));

	plane = Plane(glm::vec3(0, 0, 1), 1);
	plane *= roty;

	REQUIRE(Math::almostEqual(plane.normal, glm::vec3(-1, 0, 0)));
	REQUIRE(Math::almostEqual(plane.distance, 1));

	plane *= scale;

	REQUIRE(Math::almostEqual(plane.normal, glm::vec3(-2, 0, 0)));
	REQUIRE(Math::almostEqual(plane.distance, 4));

	plane = Plane(glm::vec3(0, 0, 1), 0) * roty;

	REQUIRE(Math::almostEqual(plane.normal, glm::vec3(-1, 0, 0)));
	REQUIRE(Math::almostEqual(plane.distance, 0));

	plane = Plane(glm::vec3(0, 0, -1), 1) * roty;

	REQUIRE(Math::almostEqual(plane.normal, glm::vec3(1, 0, 0)));
	REQUIRE(Math::almostEqual(plane.distance, 1));
}

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
	auto nn = std::abs(Math::getNormalizedNearDepth());
	REQUIRE(near.distance == nn);
	REQUIRE(near.normal == glm::vec3(0, 0, -1));
	REQUIRE(far.distance == 1);
	REQUIRE(far.normal == glm::vec3(0, 0, 1));
	REQUIRE(bottom.distance == 1);
	REQUIRE(bottom.normal == glm::vec3(0, -1, 0));
	REQUIRE(top.distance == 1);
	REQUIRE(top.normal == glm::vec3(0, 1, 0));
	REQUIRE(left.distance == 1);
	REQUIRE(left.normal == glm::vec3(-1, 0, 0));
	REQUIRE(right.distance == 1);
	REQUIRE(right.normal == glm::vec3(1, 0, 0));
}