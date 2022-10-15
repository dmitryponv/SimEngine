#pragma once
#include <string>
#include <cmath>

#include <vector>
#include "3Dclasses/IModel.h"
#include "../Containers.h"

class BoundingRegion
{
public:
	BoundingRegion(Returns& e_returns, std::string chain_name, std::shared_ptr<IModel> p_model, IModel::Vec3 end_effector_location);

	~BoundingRegion();


	std::shared_ptr<IModel> GetModel()
	{
		if (boundingModel != nullptr)
			return boundingModel;
		else
			return nullptr;
	}

	bool CheckBoundingVolume(IModel::Vec3 end_effector_location, bool& inside_volume, IModel::Vec3& first_intersection_point);

	IModel::Vec3 endEffectorDefault;

private:
	Returns& eReturns;
	std::string chainName;

	std::shared_ptr<IModel> boundingModel = nullptr;

	bool RayIntersectsTriangle(IModel::Vec3 ray_origin, IModel::Vec3 ray_vector, IModel::Face3 in_triangle, IModel::Vec3& out_intersection_point);
};

