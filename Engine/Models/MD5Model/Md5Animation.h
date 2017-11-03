#pragma once

#include "pch.h"
#include "Md5AnimationStructs.h"

namespace vxe {
	class Md5Animation {
	public:
		Md5Animation();
		virtual ~Md5Animation();

		// Load an animation from the animation file
		void LoadAnimation(const std::vector<byte>& data);
		// Update this animation's joint set.
		void Update(float fDeltaTime);
		// Draw the animated skeleton
		void Render();

		const FrameSkeleton& GetSkeleton() const
		{
			return frameSkeleton;
		}

		int GetNumJoints() const
		{
			return numJoints;
		}

		const AnimationJointInfo& GetJointInfo(unsigned int index) const
		{
			assert(index < jointInfoList.size());
			return jointInfoList[index];
		}

	protected:
		void BuildFrameSkeleton(FrameSkeletonList& skeletons, const AnimationJointInfoList& jointInfos, const BaseFrameList& baseFrames, const FrameData& frameData);

		AnimationJointInfoList jointInfoList;
		BoundingBoxList frameBounds;
		BaseFrameList    baseFrameJoints;
		FrameDataList    frameData;
		FrameSkeletonList skeletons; //all the skeletons for all frames

		FrameSkeleton frameSkeleton;

	private:
			int numFrames;
			int numJoints;
			int frameRate;
			int numAnimatedComponents;

			float animationDuration;
			float frameDuration;
			float animationTime;
	};
}