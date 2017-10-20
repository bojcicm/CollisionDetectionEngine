#pragma once

#include "pch.h"
#include "ModelSubset.h"

namespace vxe {
	class Model3D {

	public:
		int numberOfSubsets;
		int numberOfJoints;

		DirectX::XMMATRIX modelWorld;
		std::vector<Joint> joints;
		std::vector<std::shared_ptr<ModelSubset>> subsets;

		void Reset()
		{
			DebugPrint(std::string("\t Model::Reset() ...\n"));
			for (auto& subset : subsets)
				subset->Reset();
		}

		concurrency::task<void> CreateAsync(_In_ ID3D11Device2* device, std::wstring filename);
	};
}