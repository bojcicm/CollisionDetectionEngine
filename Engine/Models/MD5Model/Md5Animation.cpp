#pragma once

#include "pch.h"
#include "Md5Animation.h"
#include "Md5AnimationStructs.h"
#include "../../Third Party/DirectX Tool Kit/VertexTypes.h"
#include "../../Helpers.h"
#include "../../Utilities.h"

namespace vxe {
	
	void Md5Animation::LoadAnimation(const std::vector<byte>& data)
	{
		std::wstringstream fileIn(std::wstring(data.begin(), data.end()));
		std::wstring checkString;					// Stores the next string from our file

		auto readString = [&fileIn, &checkString] { fileIn >> checkString; };
		auto ignoreRestOfLine = [&fileIn, &checkString] { std::getline(fileIn, checkString); };
		auto getStringNameWithPossibleSpacesBetweenQuotes = [&fileIn] {
			std::wstring jointName;
			fileIn >> jointName;

			// Sometimes the names might contain spaces. If that is the case, we need to continue
			// to read the name until we get to the closing " (quotation marks)
			if (jointName[jointName.size() - 1] != '"')
			{
				wchar_t checkChar;
				bool jointNameFound = false;
				while (!jointNameFound)
				{
					checkChar = fileIn.get();

					if (checkChar == '"')
						jointNameFound = true;

					jointName += checkChar;
				}
			}
			return jointName;
		};

		DebugPrint(std::string("Model : Parsing animation data.. \n"));

		while (fileIn)
		{
			readString();
			if (checkString == L"MD5Version")
			{
				int md5Version;
				fileIn >> md5Version;
				assert(md5Version == 10);
			}
			else if (checkString == L"commandline") 
			{
				ignoreRestOfLine();
			}
			else if (checkString == L"numFrames")
			{
				fileIn >> numFrames;
			}
			else if (checkString == L"numJoints")
			{
				fileIn >> numJoints;
			}
			else if (checkString == L"frameRate")
			{
				fileIn >> frameRate;
			}
			else if (checkString == L"numAnimatedComponents")
			{
				fileIn >> numAnimatedComponents;
			}
			else if (checkString == L"hierarchy")
			{
				readString(); //skip '{'

				for (auto i = 0; i < numJoints; i++)
				{
					AnimationJointInfo joint;
					joint.name = getStringNameWithPossibleSpacesBetweenQuotes();
					RemoveQuotes(joint.name);

					fileIn >> joint.parentID >> joint.flags >> joint.startIndex;
					
					jointInfoList.push_back(joint);
					ignoreRestOfLine();
				}
			}
			else if (checkString == L"bounds")
			{
				readString(); //skip '{'

				for (auto i = 0; i < numFrames; i++)
				{
					BoundingBox bound;
					readString(); // skip '('
					
					fileIn >> bound.min.x >> bound.min.z >> bound.min.y;
					
					readString(); //skip ') ('
					readString();

					fileIn >> bound.max.x >> bound.max.z >> bound.max.y;

					readString();

					frameBounds.push_back(bound);	
				}
				ignoreRestOfLine();
			}
			else if (checkString == L"baseframe")
			{
				readString();

				for (auto i = 0; i < numJoints; i++)
				{
					Point baseFrame;
					readString();

					fileIn >> baseFrame.position.x >> baseFrame.position.z >> baseFrame.position.y;

					readString(); readString();

					fileIn >> baseFrame.orientation.x >> baseFrame.orientation.z >> baseFrame.orientation.y;
					
					readString();

					baseFrameJoints.push_back(baseFrame);
				}
			}
			else if (checkString == L"frame")
			{
				FrameData tempFrame;
				fileIn >> tempFrame.frameID;

				readString(); //skip '{'

				for (auto i = 0; i < numAnimatedComponents; i++)
				{
					float tempData;
					fileIn >> tempData;
					tempFrame.frameData.push_back(tempData);
				}

				frameData.push_back(tempFrame);

				// Build a skeleton for this frame
				BuildFrameSkeleton(tempFrame);

				ignoreRestOfLine();
			}
		} // while fileIn.eof

		frameSkeleton = skeletons[0];
		frameDuration = 1.0f / (float)frameRate;
		animationDuration = frameDuration * (float)numFrames;
		animationTime = 0.0f;
	}

	concurrency::task<void> Md5Animation::InitAnimation(ID3D11Device2 * device)
	{
		_animationMesh = std::make_shared<Md5AnimationSkeletonMesh>();
		return _animationMesh->CreateAsync(device, &frameSkeleton.joints);
	}

	void Md5Animation::BuildFrameSkeleton(const FrameData& frame)
	{
		FrameSkeleton skeleton;

		for (auto i = 0; i < jointInfoList.size(); i++)
		{
			unsigned int j = 0;
			const auto& jointInfo = jointInfoList[i];
			
			SkeletonJoint animatedJoint;
			animatedJoint.orientation = baseFrameJoints[i].orientation;
			animatedJoint.position = baseFrameJoints[i].position;
			animatedJoint.parentId = jointInfo.parentID;

			if (jointInfo.flags & 1) // Pos.x
			{
				animatedJoint.position.x = frame.frameData[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flags & 2) // Pos.z RH vs LH here should be Pos.y
			{
				animatedJoint.position.z = frame.frameData[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flags & 4) // Pos.y RH vs LH here should be Pos.z
			{
				animatedJoint.position.y = frame.frameData[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flags & 8) // Orient.x
			{
				animatedJoint.orientation.x = frame.frameData[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flags & 16) // Orient.y
			{
				animatedJoint.orientation.z = frame.frameData[jointInfo.startIndex + j];
				j++;
			}
			if (jointInfo.flags & 32) // Orient.z
			{
				animatedJoint.orientation.y = frame.frameData[jointInfo.startIndex + j];
				j++;
			}

			ComputeQuaternionW(animatedJoint.orientation);

			if (animatedJoint.parentId >= 0) // Has a parent joint
			{
				auto& parentJoint = skeleton.joints[animatedJoint.parentId];

				auto parentJointPosition = DirectX::XMLoadFloat3(&parentJoint.position);
				auto parentJointOrientation = DirectX::XMLoadFloat4(&parentJoint.orientation);
				auto animatedJointPosition = DirectX::XMLoadFloat3(&animatedJoint.position);
				auto animatedJointOrientation = DirectX::XMLoadFloat4(&animatedJoint.orientation);

				auto parentJointOrientationConjugate = DirectX::XMQuaternionInverse(parentJointOrientation);
				auto rotPos = XMQuaternionMultiply(XMQuaternionMultiply(parentJointOrientation, animatedJointPosition), parentJointOrientationConjugate);
				
				DirectX::XMStoreFloat3(&animatedJoint.position, DirectX::XMVectorAdd(parentJointPosition, rotPos));

				auto newOrientation = DirectX::XMQuaternionMultiply(parentJointOrientation, animatedJointOrientation);
				DirectX::XMStoreFloat4(&animatedJoint.orientation, DirectX::XMQuaternionNormalize(newOrientation));
			}

			skeleton.joints.push_back(animatedJoint);
		}
		
		skeletons.push_back(skeleton);
	}

	void Md5Animation::Update(float deltaTime)
	{
		if (numFrames < 1) return;

		animationTime += (deltaTime * 0.5);

		if (animationTime > animationDuration) animationTime = fmod(animationTime,animationDuration);
		//while (animationTime < 0.0f) animationTime += animationDuration;

		auto frameNumber = animationTime * (float)frameRate;
		auto frame0 = (int)floorf(frameNumber) % numFrames;
		auto frame1 = frame0 + 1;
		if (frame0 == numFrames - 1)
			frame1 = 0;

		auto interpolate = frameNumber - frame0;

		auto& skeleton0 = skeletons[frame0];
		auto& skeleton1 = skeletons[frame1];

		frameSkeleton.joints.clear();
		for (auto i = 0; i < numJoints; i++)
		{
			auto finalJoint = SkeletonJoint();
			const auto& joint0 = skeleton0.joints[i];
			const auto& joint1 = skeleton1.joints[i];

			auto joint0Orientation = DirectX::XMLoadFloat4(&joint0.orientation);
			auto joint1Orientation = DirectX::XMLoadFloat4(&joint1.orientation);

			finalJoint.parentId = joint0.parentId;
			finalJoint.position.x = joint0.position.x + (interpolate * (joint1.position.x - joint0.position.x));
			finalJoint.position.y = joint0.position.y + (interpolate * (joint1.position.y - joint0.position.y));
			finalJoint.position.z = joint0.position.z + (interpolate * (joint1.position.z - joint0.position.z));

			XMStoreFloat4(&finalJoint.orientation, DirectX::XMQuaternionSlerp(joint0Orientation, joint1Orientation, interpolate));

			frameSkeleton.joints.push_back(finalJoint);
		}

		InterpolateSkeletons(frameSkeleton, skeletons[frame0], skeletons[frame1], interpolate);
	}

	void Md5Animation::InterpolateSkeletons(FrameSkeleton& finalSkeleton, const FrameSkeleton& frameSkeleton0, const FrameSkeleton& frameSkeleton1, float fInterpolate)
	{
		for (auto i = 0; i < numJoints; i++)
		{
			auto& finalJoint = finalSkeleton.joints[i];
			finalJoint = SkeletonJoint();

			const auto& joint0 = frameSkeleton0.joints[i];
			const auto& joint1 = frameSkeleton1.joints[i];

			auto joint0Orientation = DirectX::XMLoadFloat4(&joint0.orientation);
			auto joint1Orientation = DirectX::XMLoadFloat4(&joint1.orientation);

			finalJoint.parentId = joint0.parentId;
			finalJoint.position.x = joint0.position.x + (fInterpolate * (joint1.position.x - joint0.position.x));
			finalJoint.position.y = joint0.position.y + (fInterpolate * (joint1.position.y - joint0.position.y));
			finalJoint.position.z = joint0.position.z + (fInterpolate * (joint1.position.z - joint0.position.z));

			XMStoreFloat4(&finalJoint.orientation, DirectX::XMQuaternionSlerp(joint0Orientation, joint1Orientation, fInterpolate));
		}
	}

	void Md5Animation::Render(_In_ ID3D11DeviceContext2* context)
	{
		_animationMesh->BindVertexBuffer(context);
		_animationMesh->BindIndexBuffer(context);
		_animationMesh->DrawIndexed(context);
	}

	void Md5Animation::UpdateBuffers(_In_ ID3D11DeviceContext2* context)
	{
		auto joints = &(GetSkeleton()->joints);
		_animationMesh->UpdateSkeletonMesh(joints);

		_animationMesh->UpdateVertexBuffer(context);
	}
}
