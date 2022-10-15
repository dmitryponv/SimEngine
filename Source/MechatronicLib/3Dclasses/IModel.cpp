#define TINYOBJLOADER_IMPLEMENTATION
#include "IModel.h"

bool IModel::LoadOBJ(std::string object_file)
{
	std::string err;
	bool ret = tinyobj::LoadObj(&mAttrib, &mShapes, &mMaterials, &err, object_file.c_str());

	if (!err.empty()) { // `err` may contain warning message.
						//std::cerr << err << std::endl;
						//DebugOut(("Error: " + err + "\n").c_str());
	}

	if (!ret) {
		//DebugOut("Bounding Region Load Error\n");
		return false;
	}

	// Loop over shapes
	for (size_t s = 0; s < mShapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < mShapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = mShapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = mShapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = mAttrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = mAttrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = mAttrib.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t nx = mAttrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = mAttrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = mAttrib.normals[3 * idx.normal_index + 2];
				//tinyobj::real_t tx = mAttrib.texcoords[2 * idx.texcoord_index + 0];
				//tinyobj::real_t ty = mAttrib.texcoords[2 * idx.texcoord_index + 1];
				// Optional: vertex colors
				// tinyobj::real_t red = mAttrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = mAttrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = mAttrib.colors[3*idx.vertex_index+2];
			}
			index_offset += fv;

			// per-face material
			mShapes[s].mesh.material_ids[f];
		}
	}
	return true;
}