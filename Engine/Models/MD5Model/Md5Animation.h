#pragma once

#include "pch.h"
#include "Md5AnimationStructs.h"
#include "Md5AnimationSkeletonMesh.h"
#include "Md5BoundingBoxMesh.h"

namespace vxe {
	class Md5Animation {
	public:
		Md5Animation() {};

		// Load an animation from the animation file
		void LoadAnimation(const std::vector<byte>& data);
		concurrency::task<void> InitAnimation(ID3D11Device2 * device);
		// Update this animation's joint set.
		void Update(float deltaTime);
		void UpdateBuffers(_In_ ID3D11DeviceContext2* context);
		// Draw the animated skeleton
		void RenderBoundingBox(_In_ ID3D11DeviceContext2* context);
		void RenderSkeleton(_In_ ID3D11DeviceContext2* context);
		
		void Reset()
		{
			_animationMesh->Reset();
			_animationBoundingBox->Reset();
		}

		const FrameSkeleton* GetSkeleton() const
		{
			return &frameSkeleton;
		}

		int GetNumJoints() const
		{
			return numJoints;
		}

	protected:
		void BuildFrameSkeleton(const FrameData& frameData);
		void InterpolateSkeletons(FrameSkeleton& finalSkeleton, const FrameSkeleton& skeleton0, const FrameSkeleton& skeleton1, float fInterpolate);
		void InterpolateBoundingBox(const BoundingBoxBorders& box0, const BoundingBoxBorders& box1, float fInterpolate);

		AnimationJointInfoList jointInfoList;
		BoundingBoxList frameBounds;
		PointList    baseFrameJoints;
		FrameDataList    frameData;
		FrameSkeletonList skeletons; //all the skeletons for all frames

		FrameSkeleton frameSkeleton;
		std::shared_ptr<Md5AnimationSkeletonMesh> _animationMesh;

		BoundingBoxBorders frameBoundingBox;
		std::shared_ptr<Md5BoundingBoxMesh> _animationBoundingBox;

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