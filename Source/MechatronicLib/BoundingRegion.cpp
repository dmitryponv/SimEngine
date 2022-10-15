#include "BoundingRegion.h"

BoundingRegion::BoundingRegion(Returns& e_returns, std::string chain_name, std::shared_ptr<IModel> p_model, IModel::Vec3 end_effector_default) :
	eReturns(e_returns), 
	chainName(chain_name), 
	boundingModel(p_model),
	endEffectorDefault(end_effector_default)
{
}


BoundingRegion::~BoundingRegion()
{
}

bool BoundingRegion::CheckBoundingVolume(IModel::Vec3 end_effector_location, bool& is_inside_volume, IModel::Vec3& first_intersection_point)
{
	if (boundingModel == nullptr)
	{
		eReturns.Throw(chainName, "Bounding Region does not have a model");
		return false;
	}

	//Get the faces
	std::vector<IModel::Face3> bounding_faces;
	boundingModel->GetFaces(bounding_faces);
	if (bounding_faces.size() == 0)
	{
		eReturns.Throw(chainName, "Bounding Region does not have any faces");
		return false;
	}


	//Project a ray in any direction from end effector's location (+x works) and check how many of the faces it intersects
	IModel::Vec3 ray_vector = endEffectorDefault - end_effector_location;//
	if(ray_vector.Length() < 1.0F) //In case of returning close to zero position
		ray_vector = IModel::Vec3(1.0, 0.0, 0.0);
	int intersection_count = 0;
	float distance_to_intersection = 1000000.0F;
	IModel::Vec3 intersection_point;
	for (auto& v_face : bounding_faces)
	{
		//IModel::Vec3 intersection_point;
		if (RayIntersectsTriangle(end_effector_location, ray_vector, v_face, intersection_point))
		{
			intersection_count++;

			//Check which intersection point is closest
			IModel::Vec3 distance_vector = end_effector_location - intersection_point;
			float distance_vector_length = distance_vector.Length();
			if (distance_vector_length < distance_to_intersection)
			{
				distance_to_intersection = distance_vector_length;
				first_intersection_point = intersection_point;
			}
			
		}
	}

	//If intersection count is an odd number, the end effector is inside boundign region, otherwise it is outside.
	is_inside_volume = (intersection_count % 2 == 1);

	return true;
}

bool BoundingRegion::RayIntersectsTriangle(IModel::Vec3 ray_origin, IModel::Vec3 ray_vector, IModel::Face3 in_triangle, IModel::Vec3& out_intersection_point)
{
	const float EPSILON = 0.0000001;
	IModel::Vec3 vertex_0 = in_triangle.vertex_0;
	IModel::Vec3 vertex_1 = in_triangle.vertex_1;
	IModel::Vec3 vertex_2 = in_triangle.vertex_2;
	IModel::Vec3 edge_1, edge_2, _h, _s, _q;
	float _a, _f, _u, _v;
	edge_1 = vertex_1 - vertex_0;
	edge_2 = vertex_2 - vertex_0;
	_h = ray_vector.CrossProduct(edge_2);
	_a = edge_1.DotProduct(_h);
	if (_a > -EPSILON && _a < EPSILON)
		return false;
	_f = 1 / _a;
	_s = ray_origin - vertex_0;
	_u = _f * (_s.DotProduct(_h));
	if (_u < 0.0 || _u > 1.0)
		return false;
	_q = _s.CrossProduct(edge_1);
	_v = _f * ray_vector.DotProduct(_q);
	if (_v < 0.0 || _u + _v > 1.0)
		return false;
	// At this stage we can compute t to find out where the intersection point is on the line.
	float _t = _f * edge_2.DotProduct(_q);
	if (_t > EPSILON) // ray intersection
	{
		out_intersection_point = ray_origin + ray_vector * _t;
		return true;
	}
	else // This means that there is a line intersection but not a ray intersection.
		return false;
}
