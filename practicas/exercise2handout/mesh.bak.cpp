#include "mesh.h"

#include <assert.h>
#include <assimp/cimport.h>			// C importer
#include <assimp/postprocess.h> // various extra operations
#include <assimp/scene.h>				// collects data

//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

#include "maths_funcs.h"
#include "gl_utils.h"

#define DMAP_IMG_FILE "DefaultDiffuseMap.png"
#define NMAP_IMG_FILE "DefaultNormalMap.png"

bool getTransform(const aiScene* scene, const aiNode* node, const aiMesh* mesh, aiMatrix4x4& transform) {
	if (node->mNumMeshes) {
		for (unsigned i = 0; i < node->mNumMeshes; ++i) {
			if (scene->mMeshes[node->mMeshes[i]] == mesh) {
				transform = node->mTransformation * transform;
				return true;
			}
		}
	}
	//else {
	//	transform = node->mTransformation * transform;
	//}
	for (unsigned i = 0; i < node->mNumChildren; ++i) {
		aiNode* child = node->mChildren[i];
		if (getTransform(scene, child, mesh, transform)) {
			transform = node->mTransformation * transform;
			return true;
		}
	}
	return false;
}

bool Meshgroup::load_from_file(const char* file_name, int index ) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_CalcTangentSpace| aiProcess_GenSmoothNormals /*| aiProcess_FlipUVs*/);
	aiNode* root = scene->mRootNode;

	if (!scene) {
		const char* error = aiGetErrorString();
		fprintf(stderr, "ERROR: reading mesh %s:\n%s\n", file_name, error);
		return false;
	}
	printf("  %i animations\n", scene->mNumAnimations);
	printf("  %i cameras\n", scene->mNumCameras);
	printf("  %i lights\n", scene->mNumLights);
	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	// get first mesh only
	meshes.resize(scene->mNumMeshes);
	for (unsigned m = 0; m < meshes.size(); ++m) {
		Mesh& mesh = meshes[m]; 

		const aiMesh* aimesh = scene->mMeshes[m];
		aiMatrix4x4 aiTransform;
		getTransform(scene, root, aimesh, aiTransform);

		mesh.transform = transpose(mat4( 
			aiTransform.a1, aiTransform.a2, aiTransform.a3, aiTransform.a4,
			aiTransform.b1, aiTransform.b2, aiTransform.b3, aiTransform.b4,
			aiTransform.c1, aiTransform.c2, aiTransform.c3, aiTransform.c4,
			aiTransform.d1, aiTransform.d2, aiTransform.d3, aiTransform.d4
		));

		printf("    %i vertices in mesh[0]\n", aimesh->mNumVertices);
		mesh.vertex_count = aimesh->mNumVertices;
		mesh.face_count = aimesh->mNumFaces;

		// allocate memory for vertex points
		if (aimesh->HasPositions()) {
			printf("mesh has positions\n");
			mesh.vp = (GLfloat*)malloc(mesh.vertex_count * 3 * sizeof(GLfloat));
		}
		if (aimesh->HasNormals()) {
			printf("mesh has normals\n");
			mesh.vn = (GLfloat*)malloc(mesh.vertex_count * 3 * sizeof(GLfloat));
		}

		//unsigned colors = aimesh->GetNumColorChannels();
		//printf("mesh has %d color channels\n", colors);
		//if (colors > 0) {
		//	if (aimesh->HasVertexColors(0)) {
		//		printf("mesh has colors in channe 0\n");
		//		mesh.vc = (GLfloat*)malloc(mesh.vertex_count * 4 * sizeof(GLfloat));
		//	}
		//}
		int uvs = aimesh->GetNumUVChannels();
		if (uvs) {
			printf("mesh has texture coords\n");
			mesh.uvs.resize(uvs);
			for (int j = 0; j < uvs; j++) {
				mesh.uvs[j] = (GLfloat*)malloc(mesh.vertex_count * 2 * sizeof(GLfloat));
			}
		}
		if (aimesh->HasTangentsAndBitangents()) {
			printf("mesh has tangents\n");
			mesh.vtans = (GLfloat*)malloc(mesh.vertex_count * 4 * sizeof(GLfloat));
		}
		if (aimesh->HasFaces()) {
			printf("mesh has tangents\n");
			mesh.faces_indices = (GLuint*)malloc(mesh.face_count * 3 * sizeof(GLuint));
		}

		for (unsigned int i = 0; i < aimesh->mNumVertices; i++) {
			if (aimesh->HasPositions()) {
				const aiVector3D* aivp = &(aimesh->mVertices[i]);
				mesh.vp[i * 3] = (GLfloat)aivp->x;
				mesh.vp[i * 3 + 1] = (GLfloat)aivp->y;
				mesh.vp[i * 3 + 2] = (GLfloat)aivp->z;
			}
			if (aimesh->HasNormals()) {
				const aiVector3D* aivn = &(aimesh->mNormals[i]);
				mesh.vn[i * 3] = (GLfloat)aivn->x;
				mesh.vn[i * 3 + 1] = (GLfloat)aivn->y;
				mesh.vn[i * 3 + 2] = (GLfloat)aivn->z;
			}
			//if (aimesh->GetNumColorChannels() > 0 && aimesh->HasVertexColors(0)) {
			//	const aiColor4D* aivc = &(aimesh->mColors[0][i]);
			//	mesh.vc[i * 4] = (GLfloat)aivc->r;
			//	mesh.vc[i * 4 + 1] = (GLfloat)aivc->g;
			//	mesh.vc[i * 4 + 2] = (GLfloat)aivc->b;
			//	mesh.vc[i * 4 + 3] = (GLfloat)aivc->a;
			//}
			int uvs = aimesh->GetNumUVChannels();
			for (int j = 0; j< uvs; j++) {
				if (aimesh->HasTextureCoords(j)) {
					const aiVector3D* aivt = &(aimesh->mTextureCoords[j][i]);
					mesh.uvs[j][i * 2] = (GLfloat)aivt->x;
					mesh.uvs[j][i * 2 + 1] = (GLfloat)aivt->y;

					//printf("%0.2f %0.2f\n", mesh.uvs[j][i * 2], mesh.uvs[j][i * 2 + 1]);
				}
			}
			if (aimesh->HasTangentsAndBitangents()) {
				const aiVector3D* aitangent = &(aimesh->mTangents[i]);
				const aiVector3D* aibitangent = &(aimesh->mBitangents[i]);
				const aiVector3D* ainormal = &(aimesh->mNormals[i]);

				// put the three vectors into my vec3 struct format for doing maths
				vec3 t(aitangent->x, aitangent->y, aitangent->z);
				vec3 n(ainormal->x, ainormal->y, ainormal->z);
				vec3 b(aibitangent->x, aibitangent->y, aibitangent->z);
				// orthogonalise and normalise the aitangent so we can use it in something
				// approximating a T,N,B inverse matrix
				vec3 t_i = normalise(t - n * dot(n, t));

				// get determinant of T,B,N 3x3 matrix by dot*cross method
				float det = (dot(cross(n, t), b));
				if (det < 0.0f) {
					det = -1.0f;
				}
				else {
					det = 1.0f;
				}

				// push back 4d vector for inverse aitangent with determinant
				mesh.vtans[i * 4] = t_i.v[0];
				mesh.vtans[i * 4 + 1] = t_i.v[1];
				mesh.vtans[i * 4 + 2] = t_i.v[2];
				mesh.vtans[i * 4 + 3] = det;
			}
		}
		for (unsigned i = 0; i < aimesh->mNumFaces; ++i) {
			if (aimesh->HasFaces()) {
				const aiFace& aiface = aimesh->mFaces[i];
				assert(aiface.mNumIndices == 3);
				mesh.faces_indices[i * 3] = aiface.mIndices[0];
				mesh.faces_indices[i * 3 + 1] = aiface.mIndices[1];
				mesh.faces_indices[i * 3 + 2] = aiface.mIndices[2];
			}
		}
		mesh.index_count = mesh.face_count*3;
		
		mesh.MaterialIndex = aimesh->mMaterialIndex;
		unsigned materialsSize = scene->mNumMaterials;
		//for (int i = 0; i < materialsSize; ++i) {
		if (materialsSize > mesh.MaterialIndex) {
			const aiMaterial* material = scene->mMaterials[mesh.MaterialIndex];
			{
				aiString path;
				aiTextureMapping mapping;
				unsigned int uvindex;
				unsigned count = (*material).GetTextureCount(aiTextureType_DIFFUSE);
				if (count) {
					(*material).GetTexture(aiTextureType_DIFFUSE, 0, &path, &mapping, &uvindex);
				}
				path = path.length == 0 ? aiString(DMAP_IMG_FILE) : path;
				if (path.length) {
					unsigned char* image_data;
					int x, y, n;
					load_image_data(path.C_Str(), &image_data, x, y, n);
					load_texture_to_gpu(image_data, &mesh.dmap_tex, x, y, n);
					unload_image_data(image_data);
				}
			}
			{
				aiString path;
				unsigned count = (*material).GetTextureCount(aiTextureType_NORMALS);
				if (count) {
					(*material).GetTexture(aiTextureType_NORMALS, 0, &path);
				}
				path = path.length == 0 ? aiString(NMAP_IMG_FILE) : path;
				if (path.length) {
					unsigned char* image_data;
					int x, y, n;
					load_image_data(path.C_Str(), &image_data, x, y, n);
					load_texture_to_gpu(image_data, &mesh.nmap_tex, x, y, n);
					unload_image_data(image_data);
				}
			}
			{
				aiColor3D color(0.f, 0.f, 0.f);
				(*material).Get(AI_MATKEY_COLOR_DIFFUSE, color);
				//printf("read color:%f,%f,%f", color.r, color.g, color.b);
				mesh.diffuse_base_color = vec3(color.r, color.g, color.b);
			}
		}
	}
	
	aiReleaseImport(scene);

	printf("mesh loaded\n");

	return true;
}

void Meshgroup::load_to_gpu() {

	for (unsigned m = 0; m < meshes.size(); ++m) {
		Mesh& mesh = meshes[m];

		glGenVertexArrays(1, &mesh.vao);
		glBindVertexArray(mesh.vao);

		size_t attribIx = 0;

		if (NULL != mesh.vp) {
			glGenBuffers(1, &mesh.points_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.points_vbo);
			glBufferData(GL_ARRAY_BUFFER, 3 * mesh.vertex_count * sizeof(GLfloat), mesh.vp, GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribIx);
			glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		++attribIx;

		if (NULL != mesh.vn) {
			glGenBuffers(1, &mesh.normals_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.normals_vbo);
			glBufferData(GL_ARRAY_BUFFER, 3 * mesh.vertex_count * sizeof(GLfloat), mesh.vn, GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribIx);
			glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		}

		++attribIx;

		mesh.uvs_vbos.resize(mesh.uvs.size());
		for (unsigned j = 0; j < 2; ++j) {
			bool hasUVs = mesh.uvs.size() > j ;
			if (hasUVs) {
				glGenBuffers(1, &mesh.uvs_vbos[j]);
				glBindBuffer(GL_ARRAY_BUFFER, mesh.uvs_vbos[j]);
				glBufferData(GL_ARRAY_BUFFER, 2 * mesh.vertex_count * sizeof(GLfloat), mesh.uvs[j], GL_STATIC_DRAW);
				glEnableVertexAttribArray(attribIx);
				glVertexAttribPointer(attribIx, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			}
			++attribIx;
		}

		if (NULL != mesh.vtans) {
			glGenBuffers(1, &mesh.tangents_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh.tangents_vbo);
			glBufferData(GL_ARRAY_BUFFER, 4 * mesh.vertex_count * sizeof(GLfloat), mesh.vtans, GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribIx);
			glVertexAttribPointer(attribIx, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		}
		++attribIx;

		if (mesh.faces_indices != nullptr) {
			glGenBuffers(1, &mesh.faces_vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.faces_vbo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*mesh.index_count, &mesh.faces_indices[0], GL_STATIC_DRAW);
		}
		glBindVertexArray(0);

	}
}

void Meshgroup::load_textures_to_gpu() {


	//load_texture( DMAP_IMG_FILE, &dmap_tex );
	//load_texture( NMAP_IMG_FILE, &nmap_tex );
}

void Meshgroup::get_shader_locations(GLuint shader_programme) {
	normal_map_location = glGetUniformLocation( shader_programme, "normal_map" );
	diffuse_map_location = glGetUniformLocation( shader_programme, "diffuse_map" );
	model_matrix_location = glGetUniformLocation( shader_programme, "model" );
	diffuse_base_color_location = glGetUniformLocation( shader_programme, "diffuse_base_color" );
	ambient_color_location = glGetUniformLocation( shader_programme, "ambient_color" );
}

void Meshgroup::set_shader_data(GLuint shader_programme, const Node& node, const vec3& ambient_color) 
{
	transform = node.worldMatrix;
	glUniform3fv( ambient_color_location, 1, &ambient_color.v[0] );
}

void Meshgroup::render(GLuint shader_programme)
{
	for (size_t i = 0; i < meshes.size(); ++i) {
		Mesh& mesh = meshes[i];

		mat4 modelMat = transform * mesh.transform;
		glUniformMatrix4fv( model_matrix_location, 1, GL_FALSE, modelMat.m );
	
		glUniform3fv(diffuse_base_color_location, 1, &mesh.diffuse_base_color.v[0]);

		glUniform1i( normal_map_location, 0 );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, mesh.nmap_tex);
		
		glUniform1i( diffuse_map_location, 1 );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_2D, mesh.dmap_tex);

		glBindVertexArray(mesh.vao);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.faces_vbo);

		glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
}



