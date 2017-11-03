#pragma once

#include "pch.h"
#include "Md5Animation.h"
#include "Md5AnimationStructs.h"
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
				ignoreRestOfLine();

				for (auto i = 0; i < numJoints; i++)
				{
					BaseFrame baseFrame;
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
				tempFrame.frameData.reserve(numAnimatedComponents);
				for (auto i = 0; i < numAnimatedComponents; i++)
				{
					float tempData;
					fileIn >> tempData;
					tempFrame.frameData.push_back(tempData);
				}

				frameData.push_back(tempFrame);

				// Build a skeleton for this frame
				BuildFrameSkeleton(skeletons, jointInfoList, baseFrameJoints, tempFrame);

				ignoreRestOfLine();
			}
		}
	}

	void Md5Animation::BuildFrameSkeleton(FrameSkeletonList& skeletons, const AnimationJointInfoList& jointInfos, const BaseFrameList& baseFrames, const FrameData& frame)
	{
		FrameSkeleton skeleton;

		for (auto i = 0; i < jointInfoList.size(); i++)
		{
			unsigned int j = 0;
			const auto& jointInfo = jointInfoList[i];
			
			SkeletonJoint animatedJoint;
			animatedJoint.orientation = baseFrames[i].orientation;
			animatedJoint.position = baseFrames[i].position;
			animatedJoint.parentId = jointInfo.parentID;

			if (jointInfo.flags & 1) // Pos.x
			{
				animatedJoint.position.x = frame.frameData[jointInfo.startIndex + j++];
			}
			if (jointInfo.flags & 2) // Pos.y
			{
				animatedJoint.position.y = frame.frameData[jointInfo.startIndex + j++];
			}
			if (jointInfo.flags & 4) // Pos.x
			{
				animatedJoint.position.z = frame.frameData[jointInfo.startIndex + j++];
			}
			if (jointInfo.flags & 8) // Orient.x
			{
				animatedJoint.orientation.x = frame.frameData[jointInfo.startIndex + j++];
			}
			if (jointInfo.flags & 16) // Orient.y
			{
				animatedJoint.orientation.y = frame.frameData[jointInfo.startIndex + j++];
			}
			if (jointInfo.flags & 32) // Orient.z
			{
				animatedJoint.orientation.z = frame.frameData[jointInfo.startIndex + j++];
			}

			ComputeQuaternionW(animatedJoint.orientation);

			if (animatedJoint.parentId >= 0) // Has a parent joint
			{
				auto& parentJoint = skeleton.joints[animatedJoint.parentId];
				auto parentJointOrientation = DirectX::XMLoadFloat4(&parentJoint.orientation);
				auto animatedJointPosition = DirectX::XMLoadFloat3(&animatedJoint.position);
				
				//joint.orientation * weight.position
				auto rotatedPositionOfWeight = DirectX::XMQuaternionMultiply(parentJointOrientation, animatedJointPosition);

				DirectX::XMStoreFloat3(&animatedJoint.position, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&parentJoint.position), rotatedPositionOfWeight));

				auto parentOrientMulWithJointOrientNormalized = DirectX::XMVector4Normalize(
					DirectX::XMVectorMultiply(
						parentJointOrientation, 
						DirectX::XMLoadFloat4(&animatedJoint.orientation)
				));
				DirectX::XMStoreFloat4(&animatedJoint.orientation, parentOrientMulWithJointOrientNormalized);
			}

			skeleton.joints.push_back(animatedJoint);
		}
		
		skeletons.push_back(skeleton);
	}

}
