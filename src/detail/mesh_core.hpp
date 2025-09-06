#pragma once

#include <mikktspace.h>

namespace darmok
{
    struct MeshData;

    struct MeshDataCalcTangentsOperation final
	{
	public:
		MeshDataCalcTangentsOperation() noexcept;
        void operator()(MeshData& mesh) noexcept;

	private:
		SMikkTSpaceInterface _iface;
		SMikkTSpaceContext _context;

		static MeshData& getMeshDataFromContext(const SMikkTSpaceContext* context) noexcept;
		static int getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept;
		static int getNumFaces(const SMikkTSpaceContext* context) noexcept;
		static int getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept;
		static void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept;
		static void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept;
		static void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept;
		static void setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept;
	};
}