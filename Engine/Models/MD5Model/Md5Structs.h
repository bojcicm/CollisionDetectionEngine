#pragma once

using namespace DirectX;
using namespace std;

namespace vxe 
{
	struct Point {
		XMFLOAT3 position;
		XMFLOAT4 orientation;
	};

	struct Joint : Point {
		Joint() : parentId(-1)
		{}

		int parentId;
		wstring name;
	};

	struct Weight {
		int jointId;
		float bias;
		XMFLOAT3 position;
		XMFLOAT3 normal;
	};

	struct VertexWeightInfo
	{
		int startingWeightId;
		int numberOfWeights;
	};

	typedef vector<Point> PointList;
	typedef vector<Joint> JointList;
	typedef vector<Weight> WeightList;
	typedef vector<VertexWeightInfo> VertexWeightInfoList;
}