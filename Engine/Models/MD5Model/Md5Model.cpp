#pragma once

#include "pch.h"
#include "Md5Model.h"
#include "../../Helpers.h"

namespace vxe {

	void MD5Model::Update(DX::StepTimer const& timer)
	{
		auto deltaTime = timer.GetElapsedSeconds();

		//_localWorld->RotateY(timer.GetTotalSeconds());

		//if(false)
		if (m_hasAnimation)
		{
			_animation->Update(deltaTime);
			InitBoundingBox();
			for (auto i = 0; i < _meshes.size(); i++)
			{
				PrepareMesh(_meshes[i], _animation);
			}
		}
	}

	void MD5Model::UpdateBuffers(_In_ ID3D11DeviceContext2* context)
	{
		for (auto& m : _meshes)
		{
			m->UpdateVertexBuffer(context);
		}
		if(m_hasAnimation)
			_animation->UpdateBuffers(context);
	}

	void MD5Model::Render(_In_ ID3D11DeviceContext2* context, bool shouldRenderBoundingBox)
	{
		_localWorld->Update(context);
		_localWorld->GetConstantBuffer()->Bind(context);
		for (auto& m : _meshes)
		{
			RenderMesh(context, m);
		}

		if (m_hasAnimation)
		{
			_animation->RenderSkeleton(context);
			if (shouldRenderBoundingBox)
				_animation->RenderBoundingBox(context);
		}
	}

	void MD5Model::RenderMesh(_In_ ID3D11DeviceContext2* context, const std::shared_ptr<Md5Mesh>& mesh)
	{
		mesh->BindVertexBuffer(context);
		mesh->BindIndexBuffer(context);
		mesh->DrawIndexed(context);
	}

	vector<concurrency::task<void>> MD5Model::CreateAsync(ID3D11Device2 * device, std::wstring filename, std::wstring animationFileName)
	{
		DebugPrint(std::string("Model : Loading... \n"));
		
		_localWorld = std::make_shared<WorldTransforms>(device);
		_worldPosition = std::make_shared<Position>(device);
		_scale = std::make_shared<Position>(device);
		PositionDiff(15.0f, 0.0f, 0.0f);
		ScaleDiff(0.5f, 0.5f, 0.5f);
		UpdateLocalWorld();
		InitBoundingBox();

		auto tasks = vector<task<void>>();
		auto modelTask = concurrency::create_task(LoadMd5Model(device, filename));
		tasks.push_back(modelTask);

		if (!animationFileName.empty())
		{
			auto animationTask = concurrency::create_task(LoadMd5Animation(animationFileName)).then([this, device]()
			{ 
				_animation->InitAnimation(device).then([this]() {
					m_hasAnimation = true;
				});
			});
			tasks.push_back(animationTask);
		}

		return tasks;
	}

	concurrency::task<void> MD5Model::LoadMd5Model(ID3D11Device2 * device, std::wstring filename)
	{
		return concurrency::create_task([this, device, filename]()
		{
			DX::ReadDataAsync(filename).then([this, device](std::vector<byte> data)
			{
				std::vector<concurrency::task<void>> tasks;
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

				while (fileIn)
				{
					readString();

					if (checkString == L"MD5Version")
					{
						readString();
						assert(checkString == L"10");
						DebugPrint(std::wstring(L"\t MD5Version: ") + checkString.c_str() + std::wstring(L" \n"));
					}

					else if (checkString == L"commandline")
					{
						ignoreRestOfLine();
					}

					else if (checkString == L"numJoints")
					{
						fileIn >> m_numberOfJoints;
						DebugPrint(std::string("\t Number of joints: ") + std::to_string(m_numberOfJoints) + std::string(" \n"));
					}

					else if (checkString == L"numMeshes")
					{
						fileIn >> m_numberOfMeshes;
						DebugPrint(std::string("\t Number of subsets: ") + std::to_string(m_numberOfMeshes) + std::string(" \n"));
					}

					else if (checkString == L"joints")
					{
						Joint tempJoint;
						readString();				// Skip the '{'

						for (int i = 0; i < m_numberOfJoints; ++i)
						{
							tempJoint.name = getStringNameWithPossibleSpacesBetweenQuotes();		// Store joints name

							fileIn >> tempJoint.parentId;   // Store Parent joint's ID
							readString();					// Skip the "("
															// Store position of this joint (swap y and z axis if model was made in RH Coord Sys)
							fileIn >> tempJoint.position.x >> tempJoint.position.z >> tempJoint.position.y;
							readString(); readString();    // Skip the ")" and "("
														   // Store orientation of this joint
							fileIn >> tempJoint.orientation.x >> tempJoint.orientation.z >> tempJoint.orientation.y;

							//Remove the quotation marks from joints name
							RemoveQuotes(tempJoint.name);
							ComputeQuaternionW(tempJoint.orientation);

							_joints.push_back(tempJoint);

							// Skip rest of this line
							ignoreRestOfLine();
						}
						// Skip the '}'
						readString();
					}

					else if (checkString == L"mesh")
					{
						int numVerts, numTris, numWeights;

						std::shared_ptr<Md5Mesh> modelMesh;
						modelMesh = std::make_shared<Md5Mesh>();

						readString();                    // Skip the "{"
						readString();
						while (checkString != L"}") // Until we get to "}"
						{
							if (checkString == L"shader")
							{
								std::wstring fileNamePath = getStringNameWithPossibleSpacesBetweenQuotes();
								// Remove the quotation marks from texture path
								RemoveQuotes(fileNamePath);

								// TODO: Load texture

								ignoreRestOfLine();
							}
							else if (checkString == L"numverts")
							{
								fileIn >> numVerts;
								ignoreRestOfLine();

								for (int i = 0; i < numVerts; ++i)
								{
									DirectX::VertexPositionNormalColorTexture tempVert;

									readString();                      // Skip "vert"
									readString();                      // Skip "#"
									readString();                      // Skip "("

																	   // Store tex coords
									fileIn >> tempVert.textureCoordinate.x >> tempVert.textureCoordinate.y;
									readString();                      // Skip ")"

									VertexWeightInfo vertexToWeightInfo;

									fileIn >> vertexToWeightInfo.startingWeightId // Index of first weight this vert will be weighted to
										>> vertexToWeightInfo.numberOfWeights; // Number of weights for this vertex

									ignoreRestOfLine();

									modelMesh->PushBackVertex(tempVert);
									modelMesh->vertexWeightsInfo.push_back(vertexToWeightInfo);
								}
							}
							else if (checkString == L"numtris")
							{
								fileIn >> numTris;
								ignoreRestOfLine();
								modelMesh->numberOfTriangles = numTris;
								for (int i = 0; i < numTris; i++)               // Loop through each triangle
								{
									unsigned short tempIndex;
									readString();                      // Skip "tri"
									readString();                      // Skip tri counter

									for (int k = 0; k < 3; k++)                 // Store the 3 indices
									{
										fileIn >> tempIndex;
										modelMesh->PushBackIndices(tempIndex);
									}

									ignoreRestOfLine();
								}
							}
							else if (checkString == L"numweights")
							{
								fileIn >> numWeights;

								ignoreRestOfLine();

								for (int i = 0; i < numWeights; i++)
								{
									Weight tempWeight;
									readString(); readString();     // Skip "weight #"

									fileIn >> tempWeight.jointId               // Store weight's joint ID
										>> tempWeight.bias;					// Store weight's influence over a vertex

									readString();                   // Skip "("

																	// Store weight's position in joint's local space
									fileIn >> tempWeight.position.x >> tempWeight.position.z >> tempWeight.position.y;

									ignoreRestOfLine();

									// Push back tempWeight into subsets Weight array
									modelMesh->weights.push_back(tempWeight);
								}
							}
							else
							{
								ignoreRestOfLine();                // Skip anything else
							}
							readString();                                // Skip "}"
						}

						PrepareMesh(modelMesh);
						PrepareNormals(modelMesh);

						tasks.push_back(modelMesh->CreateAsync(device));
						_meshes.push_back(modelMesh);
					}

				}

				when_all(tasks.begin(), tasks.end()).then([this]()
				{
					DebugPrint(std::string("\t Model : Loading MD5 Vertex Data is complete! \n"));
				});
			});
		});
	}

	concurrency::task<void> MD5Model::LoadMd5Animation(std::wstring filename)
	{
		DebugPrint(std::wstring(L"Model : Loading animation - " + filename + L" \n"));
		_animation = std::make_shared<Md5Animation>();
		return concurrency::create_task(DX::ReadDataAsync(filename).then([this](std::vector<byte> data)
		{
			_animation->LoadAnimation(data);
			DebugPrint(std::string("\t Model : Loading Md5 Animation is complete! \n"));
		}));
	}

	void MD5Model::PrepareMesh(shared_ptr<Md5Mesh> mesh)
	{
		auto vertices = mesh->GetVertices();
		for (auto i = 0; i < mesh->GetVertexCount(); i++)
		{
			auto& vertex = vertices[i];
			auto tempPosition = DirectX::XMFLOAT3(0, 0, 0);
			vertex.position = tempPosition;
			auto vertexWeightInfo = mesh->vertexWeightsInfo[i];

			for (auto j = 0; j < vertexWeightInfo.numberOfWeights; j++)
			{
				auto& weight = mesh->weights[vertexWeightInfo.startingWeightId + j];
				auto& joint = _joints[weight.jointId];

				auto weightPosition = DirectX::XMLoadFloat3(&weight.position);
				auto jointOrientation = DirectX::XMLoadFloat4(&joint.orientation);
				auto tempJointOrientationConjugate = XMVectorSet(-joint.orientation.x, -joint.orientation.y, -joint.orientation.z, joint.orientation.w);

				XMFLOAT3 rotatedPoint;
				XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightPosition), tempJointOrientationConjugate));

				vertex.position.x += (joint.position.x + rotatedPoint.x) * weight.bias;
				vertex.position.y += (joint.position.y + rotatedPoint.y) * weight.bias;
				vertex.position.z += (joint.position.z + rotatedPoint.z) * weight.bias;
				UpdateBoundingBox(vertex.position);

				// Basically what has happened above, is 
				//		we have taken the weights position relative to the joints position
				//		we then rotate the weights position
				//				(so that the weight is actually being rotated around (0, 0, 0) in world space) 
				//				using the quaternion describing the joints rotation. 
				//		We have stored this rotated point in rotatedPoint, 
				//		which we then add to the joints position 
				//				(because we rotated the weight's position around (0,0,0) in world space, 
				//				and now need to translate it so that it appears to have been rotated around the joints position). 
				//		Finally we multiply the answer with the weights bias, or how much control the weight has over the final vertices position. 
				//		All weight's bias effecting a single vertex's position must add up to 1.
			}
		}
	}

	void MD5Model::PrepareMesh(shared_ptr<Md5Mesh> mesh, shared_ptr<Md5Animation> animation)
	{
		auto vertices = mesh->GetVertices();
		auto skeleton = animation->GetSkeleton();
		for (auto i = 0; i < mesh->GetVertexCount(); i++)
		{
			auto& vertex = vertices[i];
			vertex.position = DirectX::XMFLOAT3(0, 0, 0);

			auto vertexWeightInfo = mesh->vertexWeightsInfo[i];

			for (auto j = 0; j < vertexWeightInfo.numberOfWeights; j++)
			{
				auto& weight = mesh->weights[vertexWeightInfo.startingWeightId + j];
				auto& joint = skeleton->joints[weight.jointId];

				//update vertex position 

				auto weightPosition = DirectX::XMLoadFloat3(&weight.position);
				auto jointOrientation = DirectX::XMLoadFloat4(&joint.orientation);

				auto tempJointOrientationConjugate = DirectX::XMQuaternionInverse(jointOrientation);

				XMFLOAT3 rotatedPoint;
				XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightPosition), tempJointOrientationConjugate));

				vertex.position.x += (joint.position.x + rotatedPoint.x) * weight.bias;
				vertex.position.y += (joint.position.y + rotatedPoint.y) * weight.bias;
				vertex.position.z += (joint.position.z + rotatedPoint.z) * weight.bias;

				UpdateBoundingBox(vertex.position);
				//update normals

				//auto weightNormal = DirectX::XMLoadFloat3(&weight.normal);
				//// Rotate the normal
				//XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightNormal), tempJointOrientationConjugate));

				//vertex.normal.x -= rotatedPoint.x * weight.bias;
				//vertex.normal.y -= rotatedPoint.y * weight.bias;
				//vertex.normal.z -= rotatedPoint.z * weight.bias;
				//XMStoreFloat3(&vertex.normal, XMVector3Normalize(XMLoadFloat3(&vertex.normal)));

			}
		}
	}

	void MD5Model::PrepareNormals(shared_ptr<Md5Mesh> mesh)
	{
		auto vertices = mesh->GetVertices();
		auto indices = mesh->GetIndices();

		std::vector<DirectX::XMFLOAT3> tempNormal;
		DirectX::XMFLOAT3 unnormalized = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		float vecX, vecY, vecZ;

		DirectX::XMVECTOR edge1 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		DirectX::XMVECTOR edge2 = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		for (auto i = 0; i < mesh->numberOfTriangles; ++i)
		{
			//Get the vector describing one edge of our triangle (edge 0,2)
			vecX = vertices[indices[i * 3]].position.x - vertices[indices[i * 3 + 2]].position.x;
			vecY = vertices[indices[i * 3]].position.y - vertices[indices[i * 3 + 2]].position.y;
			vecZ = vertices[indices[i * 3]].position.z - vertices[indices[i * 3 + 2]].position.z;
			edge1 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

			vecX = vertices[indices[i * 3 + 2]].position.x - vertices[indices[i * 3 + 1]].position.x;
			vecY = vertices[indices[i * 3 + 2]].position.y - vertices[indices[i * 3 + 1]].position.y;
			vecZ = vertices[indices[i * 3 + 2]].position.z - vertices[indices[i * 3 + 1]].position.z;
			edge2 = DirectX::XMVectorSet(vecX, vecY, vecZ, 0.0f);

			//Cross multiply the two edge vectors to get the un-normalized face normal
			DirectX::XMStoreFloat3(&unnormalized, DirectX::XMVector3Cross(edge1, edge2));
			tempNormal.push_back(unnormalized);
		}

		DirectX::XMVECTOR normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		int facesUsing = 0;
		float tX, tY, tZ;

		for (int i = 0; i < mesh->GetVertexCount(); ++i)
		{
			//Check which triangles use this vertex
			for (int j = 0; j < mesh->numberOfTriangles; ++j)
			{
				if (indices[j * 3] == i ||
					indices[(j * 3) + 1] == i ||
					indices[(j * 3) + 2] == i)
				{
					tX = DirectX::XMVectorGetX(normalSum) + tempNormal[j].x;
					tY = DirectX::XMVectorGetY(normalSum) + tempNormal[j].y;
					tZ = DirectX::XMVectorGetZ(normalSum) + tempNormal[j].z;

					//If a face is using the vertex, add the unormalized face normal to the normalSum
					normalSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f);

					facesUsing++;
				}
			}

			//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
			//normalSum = normalSum / facesUsing;
			tX = DirectX::XMVectorGetX(normalSum) / facesUsing;
			tY = DirectX::XMVectorGetY(normalSum) / facesUsing;
			tZ = DirectX::XMVectorGetZ(normalSum) / facesUsing;
			normalSum = DirectX::XMVectorSet(tX, tY, tZ, 0.0f);

			//Normalize the normalSum vector
			normalSum = DirectX::XMVector3Normalize(normalSum);
			auto& vertex = vertices[i];
			//Store the normal and tangent in our current vertex
			vertex.normal.x = -DirectX::XMVectorGetX(normalSum);
			vertex.normal.y = -DirectX::XMVectorGetY(normalSum);
			vertex.normal.z = -DirectX::XMVectorGetZ(normalSum);

			//Clear normalSum, facesUsing for next vertex
			normalSum = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
			facesUsing = 0;
		}
	}
}