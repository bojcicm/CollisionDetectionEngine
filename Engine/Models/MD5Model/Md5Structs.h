#pragma once

using namespace DirectX;
using namespace std;

namespace vxe 
{
	struct Joint {
		wstring name;
		int parentId;
		XMFLOAT3 position;
		XMFLOAT4 orientation;
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

	typedef vector<Joint> JointList;
	typedef vector<Weight> WeightList;
	typedef vector<VertexWeightInfo> VertexWeightInfoList;
}