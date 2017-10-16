#pragma once

#include "pch.h"
#include "Md5Model.h"

concurrency::task<void> vxe::Model3D::CreateAsync(ID3D11Device2 * device, std::wstring filename)
{
	return concurrency::create_task([this, device, filename]() 
	{
		std::vector<concurrency::task<void>> tasks;
		std::wifstream fileIn(filename.c_str());	// Open file
		std::wstring checkString;					// Stores the next string from our file
		
		auto readString = [&fileIn, &checkString]{ fileIn >> checkString; };

		std::vector<VertexPositionNormalTangentTextureWeight> verticesStorage;
		std::vector<unsigned short> indicesStorage;

		if (fileIn) 
		{
			while (fileIn)
			{
				readString();

				if (checkString == L"MD5Version")
				{
					readString();
					DebugPrint(std::wstring(L"\t MD5Version: ") + checkString.c_str() + std::wstring(L" \n"));
				}

				else if (checkString == L"commandline") 
				{
					std::getline(fileIn, checkString);	// Ignore the rest of this line
				}

				else if (checkString == L"numJoints")
				{
					fileIn >> numberOfJoints;			// Store number of joints
					DebugPrint(std::string("\t Number of joints: ") + std::to_string(numberOfJoints) + std::string(" \n"));
				}

				else if (checkString == L"numMeshes")
				{
					fileIn >> numberOfSubsets;			// Store number of joints
					DebugPrint(std::string("\t Number of subsets: ") + std::to_string(numberOfSubsets) + std::string(" \n"));
				}

				else if (checkString == L"joints")
				{
					DebugPrint(std::string("\t Model CreateAsync : Parsing joints data\n"));
					Joint tempJoint;
					readString();				// Skip the '{'

					for (int i = 0; i < numberOfJoints; i++)
					{
						fileIn >> tempJoint.name;		// Store joints name

														// Sometimes the names might contain spaces. If that is the case, we need to continue
														// to read the name until we get to the closing " (quotation marks)
						if (tempJoint.name[tempJoint.name.size() - 1] != '"')
						{
							wchar_t checkChar;
							bool jointNameFound = false;
							while (!jointNameFound)
							{
								checkChar = fileIn.get();

								if (checkChar == '"')
									jointNameFound = true;

								tempJoint.name += checkChar;
							}
						}
						// TODO: Remove the quotation marks from joints name
						//tempJoint.name.erase(0, 1);
						//tempJoint.name.erase(tempJoint.name.size()-1, 1);
						DebugPrint(std::wstring(L"\t Name of joint: ") + tempJoint.name + std::wstring(L" \n"));

						fileIn >> tempJoint.parentId;   // Store Parent joint's ID
						readString();					// Skip the "("

						// Store position of this joint (swap y and z axis if model was made in RH Coord Sys)
						fileIn >> tempJoint.position.x >> tempJoint.position.z >> tempJoint.position.y;

						fileIn >> checkString >> checkString;    // Skip the ")" and "("

						// Store orientation of this joint
						fileIn >> tempJoint.orientation.x >> tempJoint.orientation.z >> tempJoint.orientation.y;

						// Compute the w axis of the quaternion (The MD5 model uses a 3D vector to describe the
						// direction the bone is facing. However, we need to turn this into a quaternion, and the way
						// quaternions work, is the xyz values describe the axis of rotation, while the w is a value
						// between 0 and 1 which describes the angle of rotation)
						float t = 1.0f - (tempJoint.orientation.x * tempJoint.orientation.x)
							- (tempJoint.orientation.y * tempJoint.orientation.y)
							- (tempJoint.orientation.z * tempJoint.orientation.z);
						if (t < 0.0f)
						{
							tempJoint.orientation.w = 0.0f;
						}
						else
						{
							tempJoint.orientation.w = -sqrtf(t);
						}

						joints.push_back(tempJoint);

						std::getline(fileIn, checkString);        // Skip rest of this line
					}

					readString();
				}

				else if (checkString == L"mesh")
				{
					DebugPrint(std::string("\t Model CreateAsync : Parsing mesh data\n"));

					int numVerts, numTris, numWeights;
					auto modelSubset = std::make_shared<ModelSubset>();

					readString();                    // Skip the "{"

					readString();
					while (checkString != L"}")
					{
						if (checkString == L"shader")
						{
							std::wstring fileNamePath;
							fileIn >> fileNamePath;            // Get texture's filename

							// Take spaces into account if filename or material name has a space in it
							if (fileNamePath[fileNamePath.size() - 1] != '"')
							{
								wchar_t checkChar;
								bool fileNameFound = false;
								while (!fileNameFound)
								{
									checkChar = fileIn.get();

									if (checkChar == '"')
										fileNameFound = true;

									fileNamePath += checkChar;
								}
							}

							// Remove the quotation marks from texture path
							fileNamePath.erase(0, 1);
							fileNamePath.erase(fileNamePath.size() - 1, 1);

							// TODO: Load texture

							std::getline(fileIn, checkString);                // Skip rest of this line
						}
						else if (checkString == L"numverts")
						{
							fileIn >> numVerts;
							std::getline(fileIn, checkString);                // Skip rest of this line

							for (int i = 0; i < numVerts; i++)
							{
								VertexPositionNormalTangentTextureWeight tempVert;

								readString();                      // Skip "vert"
								readString();                      // Skip "#"
								readString();                      // Skip "("
								
								// Store tex coords
								fileIn >> tempVert.textureCoordinate.x >> tempVert.textureCoordinate.y;

								readString();                      // Skip ")"
								
								// Index of first weight this vert will be weighted to
								fileIn >> tempVert.StartWeight;
								// Number of weights for this vertex
								fileIn >> tempVert.WeightCount;

								std::getline(fileIn, checkString);	// Skip rest of this line

								verticesStorage.push_back(tempVert);
							}
						}
						else if (checkString == L"numtris")
						{
							fileIn >> numTris;
							std::getline(fileIn, checkString);				// Skip rest of this line

							for (int i = 0; i < numTris; i++)               // Loop through each triangle
							{
								unsigned short tempIndex;
								readString();                      // Skip "tri"
								readString();                      // Skip tri counter

								for (int k = 0; k < 3; k++)                 // Store the 3 indices
								{
									fileIn >> tempIndex;
									indicesStorage.push_back(tempIndex);
								}

								std::getline(fileIn, checkString);          // Skip rest of this line
							}
						}
						else if (checkString == L"numweights")
						{
							fileIn >> numWeights;

							std::getline(fileIn, checkString);              // Skip rest of this line

							for (int i = 0; i < numWeights; i++)
							{
								Weight tempWeight;
								fileIn >> checkString >> checkString;        // Skip "weight #"

								fileIn >> tempWeight.jointId;                // Store weight's joint ID

								fileIn >> tempWeight.bias;                    // Store weight's influence over a vertex

								readString();                        // Skip "("

								// Store weight's pos in joint's local space
								fileIn >> tempWeight.position.x >> tempWeight.position.z >> tempWeight.position.y;

								std::getline(fileIn, checkString);            // Skip rest of this line

								// Push back tempWeight into subsets Weight array
								modelSubset->weights.push_back(tempWeight);        
							}
						}
						else
						{
							std::getline(fileIn, checkString);                // Skip anything else
						}
						readString();                                // Skip "}"
					}

					DebugPrint(std::string("\t Model CreateAsync : finding each vertex's position using the joints and weights \n"));

					//*** find each vertex's position using the joints and weights ***//
					for (auto i = 0; i < verticesStorage.size(); ++i)
					{
						auto tempVert = verticesStorage[i];

						// Make sure the vertex's pos is cleared first
						tempVert.position = XMFLOAT3(0, 0, 0);

						// Sum up the joints and weights information to get vertex's position
						for (auto j = 0; j < tempVert.WeightCount; ++j)
						{
							auto tempWeight = modelSubset->weights[tempVert.StartWeight + j];
							auto tempJoint = joints[tempWeight.jointId];

							// Convert joint orientation and weight pos to vectors for easier computation
							// When converting a 3d vector to a quaternion, you should put 0 for "w", and
							// When converting a quaternion to a 3d vector, you can just ignore the "w"
							XMVECTOR tempJointOrientation = XMVectorSet(tempJoint.orientation.x, tempJoint.orientation.y, tempJoint.orientation.z, tempJoint.orientation.w);
							XMVECTOR tempWeightPos = XMVectorSet(tempWeight.position.x, tempWeight.position.y, tempWeight.position.z, 0.0f);

							// We will need to use the conjugate of the joint orientation quaternion
							// To get the conjugate of a quaternion, all you have to do is inverse the x, y, and z
							XMVECTOR tempJointOrientationConjugate = XMVectorSet(-tempJoint.orientation.x, -tempJoint.orientation.y, -tempJoint.orientation.z, tempJoint.orientation.w);

							// Calculate vertex position (in joint space, eg. rotate the point around (0,0,0)) for this weight using the joint orientation quaternion and its conjugate
							// We can rotate a point using a quaternion with the equation "rotatedPoint = quaternion * point * quaternionConjugate"
							XMFLOAT3 rotatedPoint;
							DirectX::XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tempJointOrientation, tempWeightPos), tempJointOrientationConjugate));

							// Now move the verices position from joint space (0,0,0) to the joints position in world space, taking the weights bias into account
							// The weight bias is used because multiple weights might have an effect on the vertices final position. Each weight is attached to one joint.
							tempVert.position.x += (tempJoint.position.x + rotatedPoint.x) * tempWeight.bias;
							tempVert.position.y += (tempJoint.position.y + rotatedPoint.y) * tempWeight.bias;
							tempVert.position.z += (tempJoint.position.z + rotatedPoint.z) * tempWeight.bias;

							// Basically what has happened above, is we have taken the weights position relative to the joints position
							// we then rotate the weights position (so that the weight is actually being rotated around (0, 0, 0) in world space) using
							// the quaternion describing the joints rotation. We have stored this rotated point in rotatedPoint, which we then add to
							// the joints position (because we rotated the weight's position around (0,0,0) in world space, and now need to translate it
							// so that it appears to have been rotated around the joints position). Finally we multiply the answer with the weights bias,
							// or how much control the weight has over the final vertices position. All weight's bias effecting a single vertex's position
							// must add up to 1.
						}

						// Store the vertices position in the position vector instead of straight into the vertex vector
						// since we can use the positions vector for certain things like collision detection or picking
						// without having to work with the entire vertex structure.
						modelSubset->positions.push_back(tempVert.position);
					}

					// Put the positions into the vertices for this subset
					for (int i = 0; i < verticesStorage.size(); i++)
					{
						verticesStorage[i].position = modelSubset->positions[i];
					}

					//*** Calculate vertex normals using normal averaging ***///
					std::vector<XMFLOAT3> tempNormal;

					//normalized and unnormalized normals
					XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

					//Used to get vectors (sides) from the position of the verts
					float vecX, vecY, vecZ;

					//Two edges of our triangle
					XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
					XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

					//Compute face normals
					for (int i = 0; i < numTris; ++i)
					{
						//Get the vector describing one edge of our triangle (edge 0,2)
						vecX = verticesStorage[indicesStorage[(i * 3)]].position.x - verticesStorage[indicesStorage[(i * 3) + 2]].position.x;
						vecY = verticesStorage[indicesStorage[(i * 3)]].position.y - verticesStorage[indicesStorage[(i * 3) + 2]].position.y;
						vecZ = verticesStorage[indicesStorage[(i * 3)]].position.z - verticesStorage[indicesStorage[(i * 3) + 2]].position.z;
						edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge

																		//Get the vector describing another edge of our triangle (edge 2,1)
						vecX = verticesStorage[indicesStorage[(i * 3) + 2]].position.x - verticesStorage[indicesStorage[(i * 3) + 1]].position.x;
						vecY = verticesStorage[indicesStorage[(i * 3) + 2]].position.y - verticesStorage[indicesStorage[(i * 3) + 1]].position.y;
						vecZ = verticesStorage[indicesStorage[(i * 3) + 2]].position.z - verticesStorage[indicesStorage[(i * 3) + 1]].position.z;
						edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge

																		//Cross multiply the two edge vectors to get the un-normalized face normal
						DirectX::XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));

						tempNormal.push_back(unnormalized);
					}

					//Compute vertex normals (normal Averaging)
					XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
					int facesUsing = 0;
					float tX, tY, tZ;    //temp axis variables

										 //Go through each vertex
					for (int i = 0; i < verticesStorage.size(); ++i)
					{
						//Check which triangles use this vertex
						for (int j = 0; j < numTris; ++j)
						{
							if (indicesStorage[j * 3] == i ||
								indicesStorage[(j * 3) + 1] == i ||
								indicesStorage[(j * 3) + 2] == i)
							{
								tX = XMVectorGetX(normalSum) + tempNormal[j].x;
								tY = XMVectorGetY(normalSum) + tempNormal[j].y;
								tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

								normalSum = XMVectorSet(tX, tY, tZ, 0.0f);    //If a face is using the vertex, add the unormalized face normal to the normalSum

								facesUsing++;
							}
						}

						//Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
						normalSum = normalSum / facesUsing;

						//Normalize the normalSum vector
						normalSum = XMVector3Normalize(normalSum);

						//Store the normal and tangent in our current vertex
						verticesStorage[i].normal.x = -XMVectorGetX(normalSum);
						verticesStorage[i].normal.y = -XMVectorGetY(normalSum);
						verticesStorage[i].normal.z = -XMVectorGetZ(normalSum);

						//Clear normalSum, facesUsing for next vertex
						normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
						facesUsing = 0;
					}
					subsets.push_back(modelSubset);
					tasks.push_back(modelSubset->CreateAsync(device, verticesStorage, indicesStorage));
				}

				else
				{
					DebugPrint(std::wstring(L"\t Model : Failed to load data!" + filename + L"\n"));
				}
			}
		}

		when_all(tasks.begin(), tasks.end()).then([this]()
		{
			DebugPrint(std::string("\t Model : Loading is complete! \n"));
		});
	});
}