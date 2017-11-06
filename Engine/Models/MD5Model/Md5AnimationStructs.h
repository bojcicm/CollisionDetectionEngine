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
	
	struct FrameData
	{
		int frameID;
		std::vector<float> frameData;
	};

	struct SkeletonJoint : Point
	{
		SkeletonJoint() : Point(), parentId(-1)
		{}

		int parentId;
	};
	typedef std::vector<SkeletonJoint> SkeletonJointList;

	struct FrameSkeleton
	{
		SkeletonJointList joints;
	};

	typedef std::vector<BoundingBox> BoundingBoxList;
	typedef std::vector<AnimationJointInfo> AnimationJointInfoList;
	typedef std::vector<FrameData> FrameDataList;
	typedef std::vector<FrameSkeleton> FrameSkeletonList;
}