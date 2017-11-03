#pragma once

#include "pch.h"
#include "Md5Structs.h"

namespace vxe {
	
	struct BoundingBox
	{
		DirectX::XMFLOAT3 min;
		DirectX::XMFLOAT3 max;
	};

	struct AnimationJointInfo
	{
		std::wstring name;
		int parentID;

		int flags;
		int startIndex;
	};

	struct BaseFrame 
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 orientation;
	};
	
	struct FrameData
	{
		int frameID;
		std::vector<float> frameData;
	};

	struct SkeletonJoint
	{
		SkeletonJoint()
			: parentId(-1)
			, position(0)
		{}

		int parentId;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 orientation;
	};
	typedef std::vector<SkeletonJoint> SkeletonJointList;

	struct FrameSkeleton
	{
		SkeletonJointList joints;
	};

	typedef std::vector<BoundingBox> BoundingBoxList;
	typedef std::vector<AnimationJointInfo> AnimationJointInfoList;
	typedef std::vector<BaseFrame> BaseFrameList;
	typedef std::vector<FrameData> FrameDataList;
	typedef std::vector<FrameSkeleton> FrameSkeletonList;
}