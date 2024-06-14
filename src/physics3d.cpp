#include <darmok/physics3d.hpp>

namespace darmok
{
	Physics3dCollision Physics3dCollision::swap() const noexcept
	{
		return {
			rigidBody2,
			rigidBody1,
			normal,
			contacts
		};
	}
}