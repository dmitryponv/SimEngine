#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../Containers.h"

#ifdef __linux__ // maybe check the specific version, too...
#define SSCANF sscanf
#else
#define SSCANF sscanf_s
#endif

//enum MechanismType
//{
//	Chain,
//	Hexapod
//};

#define SOLIDWORKS


struct LdrVector
{
	float x = 0.0F;
	float y = 0.0F;
	float z = 0.0F;

	Matrix4x4 GetTranslation()
	{
#ifdef SOLIDWORKS
		Vector4 translation = Vector4(x, -y, -z); 
#else
		Vector4 translation = Matrix4x4::CreateRotX(-90)*Vector4(x, y, z); //old method to rotate by -90 deg around X axis to convert from solidworks to DirectX coordinates
#endif
		return Matrix4x4::CreateTranslation(translation);
	}

	Matrix4x4 GetRotation()
	{
#ifdef SOLIDWORKS
		return Matrix4x4::RotateZaxisToVector(Vector4(x, y, z)); 
#else
		return Matrix4x4::CreateRotX(-90)*Matrix4x4::RotateZaxisToVector(Vector4(x, y, z)); //old method to rotate by -90 deg around X axis to convert from solidworks to DirectX coordinates
#endif
	}
};

struct LdrJoint
{
	std::string jointName = "";
	std::string modelFile = "";
	std::string servoType = "";
	float stepsPerUnit = 0.0F;
	float minRange = 0.0F;
	float maxRange = 0.0F;
	bool prismaticJoint = false;
	bool staticJoint = false;
	bool clockWise = false;
	float unitsLimitToHome = 0.0F;
	LdrVector jointLocation;
	LdrVector jointDirection;
	uint8_t servoId = 0;
	uint16_t currLimit = 100;
	uint16_t maxVelocity = 350;
};

struct LdrChain
{
	std::string chainName = "";
	std::vector<LdrJoint> Joints;
	bool useIK = false;
	std::string modelFile = "";
	LdrVector chainLocation;
	LdrVector chainDirection;
	bool collisionDetection = false;
	std::string boundingModel = "";
};

struct LdrMechanism
{
	std::string mechanismName = "";
	std::string mechanismType = "";
	std::string parentMechanism = "";
	std::string parentChain = "";
	std::vector<LdrChain> Chains;
	std::vector<LdrJoint> independentJoints;
};

struct LdrMechanisms 
{
	float cameraDistance = 100;
	std::vector<LdrMechanism> ldrMechanisms;
};

class Loader
{
public:

	Loader(std::string servo_name)
	{
		fileName = "Inputs\\" + servo_name;
		//Create file if it doesnt exist
		//setupFile.open(fileName, std::fstream::in | std::fstream::out | std::fstream::app);
		//setupFile.close();
	}


	~Loader(){	}

	LdrMechanisms LoadMechanisms()
	{
		std::string v_line;
		LdrMechanisms v_mechanisms;

		setupFile.open(fileName, std::fstream::in);
		if (!setupFile)
		{
			setupFile.close();
			return v_mechanisms;
		}

		int mechanism_index = -1;
		int chain_index = -1;
		int joint_index = -1;
		while (std::getline(setupFile, v_line))
		{
			//Remove everything after >
			std::string line_ss = v_line.substr(0, v_line.find(">", 0));

			//Split up a line by spaces
			std::string v_buf; // Have a buffer string
			std::stringstream v_ss(line_ss); // Insert the string into a stream
			std::vector<std::string> v_tokens; // Create vector to hold our words
			while (v_ss >> v_buf)
				v_tokens.push_back(v_buf);

			//Add Mechanisms
			if (v_tokens[0] == "<mechanism")
			{
				v_mechanisms.ldrMechanisms.push_back(LdrMechanism());
				mechanism_index = static_cast<int>(v_mechanisms.ldrMechanisms.size()) - 1;
				chain_index = 0;

				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get Name
					if (v_tokens[i].substr(0, nameToken.size()) == nameToken)
						v_mechanisms.ldrMechanisms[mechanism_index].mechanismName = v_tokens[i].substr(nameToken.length() + 1, v_tokens[i].length() - nameToken.length() - 2);

					//Get Type
					else if (v_tokens[i].substr(0, typeToken.size()) == typeToken)
						v_mechanisms.ldrMechanisms[mechanism_index].mechanismType = v_tokens[i].substr(typeToken.length() + 1, v_tokens[i].length() - typeToken.length() - 2);

					//Get Parent Mechanism
					if (v_tokens[i].substr(0, parentMechanismToken.size()) == parentMechanismToken)
						v_mechanisms.ldrMechanisms[mechanism_index].parentMechanism = v_tokens[i].substr(parentMechanismToken.length() + 1, v_tokens[i].length() - parentMechanismToken.length() - 2);

					//Get Parent Chain
					if (v_tokens[i].substr(0, parentChainToken.size()) == parentChainToken)
						v_mechanisms.ldrMechanisms[mechanism_index].parentChain = v_tokens[i].substr(parentChainToken.length() + 1, v_tokens[i].length() - parentChainToken.length() - 2);

					//Get Camera Distance
					if (v_tokens[i].substr(0, distanceToken.size()) == distanceToken)
						SSCANF((v_tokens[i].substr(distanceToken.length(), v_tokens[i].length() - distanceToken.length())).c_str(), "%f", &v_mechanisms.cameraDistance);

				}
			}

			//Add Chains
			if (v_tokens[0] == "<chain")
			{
				if (v_mechanisms.ldrMechanisms.size() <= mechanism_index)
					return LdrMechanisms();

				v_mechanisms.ldrMechanisms[mechanism_index].Chains.push_back(LdrChain());
				chain_index = static_cast<int>(v_mechanisms.ldrMechanisms[mechanism_index].Chains.size()) - 1;
				joint_index = 0;
				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get Name
					if (v_tokens[i].substr(0, nameToken.size()) == nameToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainName = v_tokens[i].substr(nameToken.length() + 1, v_tokens[i].length() - nameToken.length() - 2);

					//Get Model
					else if (v_tokens[i].substr(0, modelToken.size()) == modelToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].modelFile = v_tokens[i].substr(modelToken.length() + 1, v_tokens[i].length() - modelToken.length() - 2);

					//Get useIK
					else if (v_tokens[i].substr(0, useIKToken.size()) == useIKToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].useIK = v_tokens[i].substr(useIKToken.length(), v_tokens[i].length() - useIKToken.length()) == "true";

					//Get collision
					else if (v_tokens[i].substr(0, collisionToken.size()) == collisionToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].collisionDetection = v_tokens[i].substr(collisionToken.length(), v_tokens[i].length() - collisionToken.length()) == "true";

					//Get Boundign Region
					else if (v_tokens[i].substr(0, boundingRegionToken.size()) == boundingRegionToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].boundingModel = v_tokens[i].substr(boundingRegionToken.length() + 1, v_tokens[i].length() - boundingRegionToken.length() - 2);
				}
			}

			else if (v_tokens[0] == "<chain_location")
			{
				if (v_mechanisms.ldrMechanisms.size() <= mechanism_index)
					return LdrMechanisms();

				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get x
					if (v_tokens[i].substr(0, xToken.size()) == xToken)
						SSCANF((v_tokens[i].substr(xToken.length(), v_tokens[i].length() - xToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainLocation.x);
					//Get y
					else if (v_tokens[i].substr(0, yToken.size()) == yToken)
						SSCANF((v_tokens[i].substr(yToken.length(), v_tokens[i].length() - yToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainLocation.y);
					//Get z
					else if (v_tokens[i].substr(0, zToken.size()) == zToken)
						SSCANF((v_tokens[i].substr(zToken.length(), v_tokens[i].length() - zToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainLocation.z);
				}
			}

			else if (v_tokens[0] == "<chain_direction")
			{
				if (v_mechanisms.ldrMechanisms.size() <= mechanism_index)
					return LdrMechanisms();

				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get x
					if (v_tokens[i].substr(0, xToken.size()) == xToken)
						SSCANF((v_tokens[i].substr(xToken.length(), v_tokens[i].length() - xToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainDirection.x);
					//Get y
					else if (v_tokens[i].substr(0, yToken.size()) == yToken)
						SSCANF((v_tokens[i].substr(yToken.length(), v_tokens[i].length() - yToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainDirection.y);
					//Get z
					else if (v_tokens[i].substr(0, zToken.size()) == zToken)
						SSCANF((v_tokens[i].substr(zToken.length(), v_tokens[i].length() - zToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].chainDirection.z);
				}
			}

			//Add Joints
			else if (v_tokens[0] == "<joint")
			{
				if (v_mechanisms.ldrMechanisms[mechanism_index].Chains.size() <= chain_index)
					return LdrMechanisms();

				v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints.push_back(LdrJoint());
				joint_index = static_cast<int>(v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints.size()) - 1;
				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get Name
					if (v_tokens[i].substr(0, nameToken.size()) == nameToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointName = v_tokens[i].substr(nameToken.length() + 1, v_tokens[i].length() - nameToken.length() - 2);
					//Get Model
					else if (v_tokens[i].substr(0, modelToken.size()) == modelToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].modelFile = v_tokens[i].substr(modelToken.length() + 1, v_tokens[i].length() - modelToken.length() - 2);
					//Get servo type
					else if (v_tokens[i].substr(0, servoTypeToken.size()) == servoTypeToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].servoType = v_tokens[i].substr(servoTypeToken.length() + 1, v_tokens[i].length() - servoTypeToken.length() - 2);

					//Get steps per unit
					else if (v_tokens[i].substr(0, stepsPerUnitToken.size()) == stepsPerUnitToken)
						SSCANF((v_tokens[i].substr(stepsPerUnitToken.length(), v_tokens[i].length() - stepsPerUnitToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].stepsPerUnit);
					//Get min
					else if (v_tokens[i].substr(0, minToken.size()) == minToken)
						SSCANF((v_tokens[i].substr(minToken.length(), v_tokens[i].length() - minToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].minRange);
					//Get max
					else if (v_tokens[i].substr(0, maxToken.size()) == maxToken)
						SSCANF((v_tokens[i].substr(maxToken.length(), v_tokens[i].length() - maxToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].maxRange);
					//Get steps per unit
					else if (v_tokens[i].substr(0, stepsPerUnitToken.size()) == stepsPerUnitToken)
						SSCANF((v_tokens[i].substr(stepsPerUnitToken.length(), v_tokens[i].length() - stepsPerUnitToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].stepsPerUnit);
					//Get limit to home
					else if (v_tokens[i].substr(0, unitLimitToken.size()) == unitLimitToken)
						SSCANF((v_tokens[i].substr(unitLimitToken.length(), v_tokens[i].length() - unitLimitToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].unitsLimitToHome);
					//Get servo Id
					else if (v_tokens[i].substr(0, servoIdToken.size()) == servoIdToken)
						SSCANF((v_tokens[i].substr(servoIdToken.length(), v_tokens[i].length() - servoIdToken.length())).c_str(), "%d", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].servoId);
					//Get current Limit
					else if (v_tokens[i].substr(0, currLimitToken.size()) == currLimitToken)
						SSCANF((v_tokens[i].substr(currLimitToken.length(), v_tokens[i].length() - currLimitToken.length())).c_str(), "%d", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].currLimit);
					//Get Max Velocity
					else if (v_tokens[i].substr(0, maxVelocityToken.size()) == maxVelocityToken)
						SSCANF((v_tokens[i].substr(maxVelocityToken.length(), v_tokens[i].length() - maxVelocityToken.length())).c_str(), "%d", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].maxVelocity);

					//Get prismatic
					else if (v_tokens[i].substr(0, prismaticToken.size()) == prismaticToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].prismaticJoint = v_tokens[i].substr(prismaticToken.length(), v_tokens[i].length() - prismaticToken.length()) == "true";
					//Get static
					else if (v_tokens[i].substr(0, staticToken.size()) == staticToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].staticJoint = v_tokens[i].substr(staticToken.length(), v_tokens[i].length() - staticToken.length()) == "true";
					//Get clock wise
					else if (v_tokens[i].substr(0, clockWiseToken.size()) == clockWiseToken)
						v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].clockWise = v_tokens[i].substr(clockWiseToken.length(), v_tokens[i].length() - clockWiseToken.length()) == "true";

				}
			}

			else if (v_tokens[0] == "<joint_location")
			{
				if (v_mechanisms.ldrMechanisms[mechanism_index].Chains.size() <= chain_index)
					return LdrMechanisms();

				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get x
					if (v_tokens[i].substr(0, xToken.size()) == xToken)
						SSCANF((v_tokens[i].substr(xToken.length(), v_tokens[i].length() - xToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointLocation.x);
					//Get y
					else if (v_tokens[i].substr(0, yToken.size()) == yToken)
						SSCANF((v_tokens[i].substr(yToken.length(), v_tokens[i].length() - yToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointLocation.y);
					//Get z
					else if (v_tokens[i].substr(0, zToken.size()) == zToken)
						SSCANF((v_tokens[i].substr(zToken.length(), v_tokens[i].length() - zToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointLocation.z);
				}
			}

			else if (v_tokens[0] == "<joint_direction")
			{
				if (v_mechanisms.ldrMechanisms[mechanism_index].Chains.size() <= chain_index)
					return LdrMechanisms();

				for (int i = 1; i < v_tokens.size(); i++)
				{
					//Get x
					if (v_tokens[i].substr(0, xToken.size()) == xToken)
						SSCANF((v_tokens[i].substr(xToken.length(), v_tokens[i].length() - xToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointDirection.x);
					//Get y
					else if (v_tokens[i].substr(0, yToken.size()) == yToken)
						SSCANF((v_tokens[i].substr(yToken.length(), v_tokens[i].length() - yToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointDirection.y);
					//Get z
					else if (v_tokens[i].substr(0, zToken.size()) == zToken)
						SSCANF((v_tokens[i].substr(zToken.length(), v_tokens[i].length() - zToken.length())).c_str(), "%f", &v_mechanisms.ldrMechanisms[mechanism_index].Chains[chain_index].Joints[joint_index].jointDirection.z);
				}
			}
		}
		
		setupFile.close();
		
		return v_mechanisms;
	}

private:
	std::string fileName;
	std::fstream setupFile;

	std::string nameToken = "name=";
	std::string typeToken = "type=";
	std::string parentMechanismToken = "parent_mechanism=";
	std::string parentChainToken = "parent_chain=";
	std::string distanceToken = "camera_distance=";
	std::string modelToken = "model=";
	std::string useIKToken = "useIK=";
	std::string boundingRegionToken = "bounding_region=";
	std::string collisionToken = "collision=";
	std::string aboutXToken = "about_x=";
	std::string aboutYToken = "about_y=";
	std::string aboutZToken = "about_z=";
	std::string xToken = "x=";
	std::string yToken = "y=";
	std::string zToken = "z=";
	std::string stepsPerUnitToken = "steps_per_unit=";
	std::string minToken = "min=";
	std::string maxToken = "max=";
	std::string prismaticToken = "prismatic=";
	std::string staticToken = "static=";
	std::string clockWiseToken = "clock_wise=";
	std::string unitLimitToken = "unit_limit_to_home=";
	std::string servoTypeToken = "servo_type=";
	std::string servoIdToken = "servo_id=";
	std::string currLimitToken = "current_limit=";
	std::string maxVelocityToken = "max_velocity=";
};