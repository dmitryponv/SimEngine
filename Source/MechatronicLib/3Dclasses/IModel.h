#pragma once
#include <stdexcept>
#include "../../Containers.h"
#include "../../Includes/tiny_obj_loader.h"

struct EmptyContainer
{

};

class IModel
{
public:

	struct Vec3
	{
		Vec3() { X = Y = Z = 0; }

		Vec3(float _x, float _y, float _z)
		{
			X = _x;
			Y = _y;
			Z = _z;
		}

		float X, Y, Z;

		friend Vec3 operator- (const Vec3 v_a, const Vec3 v_b)
		{
			return{ Vec3(v_a.X - v_b.X, v_a.Y - v_b.Y, v_a.Z - v_b.Z) };
		}
		friend Vec3 operator+ (const Vec3 v_a, const Vec3 v_b)
		{
			return{ Vec3(v_a.X + v_b.X, v_a.Y + v_b.Y, v_a.Z + v_b.Z) };
		}
		friend Vec3 operator* (const Vec3 v_a, const float v_scalar)
		{
			return{ Vec3(v_a.X * v_scalar, v_a.Y * v_scalar, v_a.Z * v_scalar) };
		}
		Vec3 CrossProduct(const Vec3 vec_b) const
		{
			return Vec3(Y * vec_b.Z - Z * vec_b.Y, Z * vec_b.X - X * vec_b.Z, X * vec_b.Y - Y * vec_b.X);
		}
		float DotProduct(const Vec3 vec_b) const
		{
			return (X * vec_b.X + Y * vec_b.Y + Z * vec_b.Z);
		}
		float Length() const
		{
			return sqrtf(X * X + Y * Y + Z * Z);
		}
	};

	struct Face3
	{
		Vec3 vertex_0, vertex_1, vertex_2;
	};

	IModel() {}
	IModel(const IModel&) {}
	virtual IModel* Clone() { return new IModel(*this); }
	virtual ~IModel() {}

	virtual bool SetWorldMatrix(Matrix4x4& world_matrix)
	{
		//TODO: add world_matrix to non-3d app
		//throw std::runtime_error("Tried to Set World Matrix in a non Direct3D app");
		return true;
	}

	virtual bool LoadOBJ(std::string object_file) {
		std::string err;
		bool ret = tinyobj::LoadObj(&mAttrib, &mShapes, &mMaterials, &err, object_file.c_str());
		if (!err.empty()) { 
			DebugOut(("Error: " + err + "\n").c_str());
		}
		return ret;
	};

	bool GetFaces(std::vector<Face3>& bounding_faces)
	{
		bounding_faces.clear();

		for (size_t s = 0; s < mShapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			int num_faces = mShapes[s].mesh.num_face_vertices.size();
			for (size_t f = 0; f < num_faces; f++) {
				int fv = mShapes[s].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				//Only supporting 3-vertex faces
				if (fv != 3)
				{
					index_offset += fv;
					continue;
				}

				Face3 current_face;

				tinyobj::index_t idx_0 = mShapes[s].mesh.indices[index_offset];
				current_face.vertex_0.X = mAttrib.vertices[3 * idx_0.vertex_index + 0];
				current_face.vertex_0.Y = mAttrib.vertices[3 * idx_0.vertex_index + 1];
				current_face.vertex_0.Z = mAttrib.vertices[3 * idx_0.vertex_index + 2];

				tinyobj::index_t idx_1 = mShapes[s].mesh.indices[index_offset + 1];
				current_face.vertex_1.X = mAttrib.vertices[3 * idx_1.vertex_index + 0];
				current_face.vertex_1.Y = mAttrib.vertices[3 * idx_1.vertex_index + 1];
				current_face.vertex_1.Z = mAttrib.vertices[3 * idx_1.vertex_index + 2];

				tinyobj::index_t idx_2 = mShapes[s].mesh.indices[index_offset + 2];
				current_face.vertex_2.X = mAttrib.vertices[3 * idx_2.vertex_index + 0];
				current_face.vertex_2.Y = mAttrib.vertices[3 * idx_2.vertex_index + 1];
				current_face.vertex_2.Z = mAttrib.vertices[3 * idx_2.vertex_index + 2];

				index_offset += fv;

				bounding_faces.push_back(current_face);
			}
		}

		return true;
	}

	//Load function
	virtual bool Load(Matrix4x4 matrix_transformation, std::string file_mesh)
	{
		return true;
	}

	//Render functions
	virtual void Draw()
	{

	}

	//Render functions
	virtual void UpdateEffects(float fog_distance)
	{

	}


	std::string modelName;
	std::string rendererName;

protected:
	tinyobj::attrib_t mAttrib;
	std::vector<tinyobj::shape_t> mShapes;
	std::vector<tinyobj::material_t> mMaterials;
};