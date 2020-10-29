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

//#define DMAP_IMG_FILE "DefaultDiffuseMap.png"
#define DMAP_IMG_FILE "CheckerDiffuseMap.png"
#define NMAP_IMG_FILE "DefaultNormalMap.png"

namespace {
	Meshgroup::Texture default_diffuse;
	Meshgroup::Texture default_normal;}


mat4 fromAssimpTransform(const aiMatrix4x4& aiTransform) {
	mat4 ret = transpose(mat4(
		aiTransform.a1, aiTransform.a2, aiTransform.a3, aiTransform.a4,
		aiTransform.b1, aiTransform.b2, aiTransform.b3, aiTransform.b4,
		aiTransform.c1, aiTransform.c2, aiTransform.c3, aiTransform.c4,
		aiTransform.d1, aiTransform.d2, aiTransform.d3, aiTransform.d4
	));
	return ret;
}

size_t getNodeHierarchySize( const aiNode* ainode )
{
	size_t ret = 1;
	//ret = ainode->mNumChildren;

	for (unsigned i = 0; i < ainode->mNumChildren; ++i) {
		aiNode* aiChild = ainode->mChildren[i];
		ret += getNodeHierarchySize(aiChild);
	}
	return ret;
}

void getNodeHierarchy(std::vector<Node>& nodes, size_t nodeIndex, size_t& currentSize,  const aiNode* ainode,  std::vector<Meshgroup::Mesh>& meshes, std::vector<std::string>& names) 
{
	// WARNING: this will not do, need decompose
	//aiMatrix4x4 local = ainode->mTransformation;
	//aiVector3D aiposition;
	//aiVector3D aiscale;
	//aiQuaternion airotation;

	//local.Decompose(aiscale, airotation, aiposition);

	mat4 localMatrix = fromAssimpTransform(ainode->mTransformation);

	vec3 position(0,0,0);
	vec3 scale(0,0,0);
	versor rotation(0,0,0,1);
	localMatrix.decompose(rotation, position, scale);
	
	//printf("ReadName:%s\n", ainode->mName.C_Str());
	//print(localMatrix);

	Node& node = nodes[nodeIndex];
	//node.position = vec3 (aiposition.x, aiposition.y, aiposition.z);
	//node.rotation = versor(airotation.x, airotation.y, airotation.z, airotation.w);
	//node.scale = vec3(aiscale.x,aiscale.y,aiscale.z);
	node.position = position;
	node.rotation = rotation;
	node.scale = scale;

	//print(node.position);
	//print(node.rotation);
	//print(node.scale);

	mat4 localMatrix2 = quat_to_mat4(node.rotation);
	localMatrix2.setColumn(3, vec4(node.position, 1));

	names[nodeIndex] = ainode->mName.C_Str();

	// update meshes node
	assert(ainode->mNumMeshes < 2 && "Do not support more than one mesh per node");
	if (ainode->mNumMeshes > 0) {
		for (unsigned i = 0; i < ainode->mNumMeshes; ++i) {
			meshes[ainode->mMeshes[i]].node = &node;
		}
	}

	// add children
	size_t childCount = ainode->mNumChildren;
	nodeIndex = currentSize;
	currentSize += childCount;

	for (unsigned i = 0; i < childCount; ++i) {
		Node& child = nodes[nodeIndex + i];
		node.addChild(child);
	}

	for (unsigned i = 0; i < childCount; ++i) {
		Node& child = nodes[nodeIndex + i];
		aiNode* aiChild = ainode->mChildren[i];
		getNodeHierarchy(nodes, nodeIndex + i, currentSize, aiChild, meshes, names);
	}
}


aiNode* getMeshNode(const aiScene* scene, aiNode* node, const aiMesh* mesh) {
	aiNode* ret = nullptr;
	if (node->mNumMeshes) {
		for (unsigned i = 0; i < node->mNumMeshes; ++i) {
			if (scene->mMeshes[node->mMeshes[i]] == mesh) {
				ret = node;
				return ret;
			}
		}
	}
	for (unsigned i = 0; i < node->mNumChildren; ++i) {
		aiNode* child = node->mChildren[i];
		ret = getMeshNode(scene, child, mesh);
		if (ret) {
			break;
		}
	}
	return ret;
}

bool getTransform(const aiScene* scene, const aiNode* node, const aiMesh* meshGroup, aiMatrix4x4& transform) {
	if (node->mNumMeshes) {
		for (unsigned i = 0; i < node->mNumMeshes; ++i) {
			if (scene->mMeshes[node->mMeshes[i]] == meshGroup) {
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
		if (getTransform(scene, child, meshGroup, transform)) {
			transform = node->mTransformation * transform;
			return true;
		}
	}
	return false;
}

bool Meshgroup::load_from_file(const char* file_name, int index ) {
	const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_CalcTangentSpace| aiProcess_GenSmoothNormals /*| aiProcess_FlipUVs*/);
	aiNode* aiRootNode = scene->mRootNode;

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
		getTransform(scene, aiRootNode, aimesh, aiTransform);
		aiNode* node = getMeshNode(scene, aiRootNode, aimesh);
		printf("mesh %d:%s\n", m, node->mName.C_Str());

		mat4 transform = transpose(mat4( 
			aiTransform.a1, aiTransform.a2, aiTransform.a3, aiTransform.a4,
			aiTransform.b1, aiTransform.b2, aiTransform.b3, aiTransform.b4,
			aiTransform.c1, aiTransform.c2, aiTransform.c3, aiTransform.c4,
			aiTransform.d1, aiTransform.d2, aiTransform.d3, aiTransform.d4
		));

		print(transform);
		printf("    %i vertices in mesh[%d]\n", aimesh->mNumVertices, m);
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

				Texture& tex= mesh.diffuse;
				tex.image_data = default_diffuse.image_data;
				tex.x = default_diffuse.x;
				tex.y = default_diffuse.y;
				tex.n = default_diffuse.n;

				if (path.length) {
					load_image_data(path.C_Str(), &tex.image_data, tex.x, tex.y, tex.n);
					//load_texture_to_gpu(mesh.diffuse_image_data, &mesh.dmap_tex, x, y, n);
					//unload_image_data(mesh.diffuse_image_data);
				}
				//else {
					//mesh.diffuse_image_data = default_diffuse_data;
					//load_texture_to_gpu(default_diffuse_data, &mesh.dmap_tex, default_diffuse_x, default_diffuse_y, default_diffuse_n);
				//}
			}
			{
				aiString path;
				unsigned count = (*material).GetTextureCount(aiTextureType_NORMALS);
				if (count) {
					(*material).GetTexture(aiTextureType_NORMALS, 0, &path);
				}
				Texture& tex= mesh.normal;
				tex.image_data = default_normal.image_data;
				tex.x = default_normal.x;
				tex.y = default_normal.y;
				tex.n = default_normal.n;
				
				if (path.length) {
					load_image_data(path.C_Str(), &tex.image_data, tex.x, tex.y, tex.n);
					//load_texture_to_gpu(mesh.normal_image_data, &mesh.nmap_tex, x, y, n);
					//unload_image_data(mesh.normal_image_data);
				}
				//else {
				//	load_texture_to_gpu(default_normal_data, &mesh.dmap_tex, default_normal_x, default_normal_y, default_normal_n);
				//}
			}
			{
				aiColor3D color(0.f, 0.f, 0.f);
				(*material).Get(AI_MATKEY_COLOR_DIFFUSE, color);
				//printf("read color:%f,%f,%f", color.r, color.g, color.b);
				mesh.diffuse_base_color = vec3(color.r, color.g, color.b);
			}
		}
	}

	size_t nodeSize = getNodeHierarchySize(aiRootNode);
	nodes.resize(nodeSize);
	names.resize(nodeSize);
	for (size_t i = 0; i < nodeSize; ++i) {
		nodes[i].init();
	}
	// TODO: evaluate if this is working (it is not)
	size_t currentSize = 1;
	getNodeHierarchy(nodes, 0, currentSize, aiRootNode, meshes, names);
	
	aiReleaseImport(scene);

	printf("mesh loaded\n");

	return true;
}

void Meshgroup::load_default_textures() {
	load_image_data(DMAP_IMG_FILE, &default_diffuse.image_data, default_diffuse.x, default_diffuse.y, default_diffuse.n);
	load_image_data(NMAP_IMG_FILE, &default_normal.image_data, default_normal.x, default_normal.y, default_normal.n);
}

void Meshgroup::Mesh::load_geometry_to_gpu() {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	size_t attribIx = 0;

	if (NULL != vp) {
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribIx);
		glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	++attribIx;

	if (NULL != vn) {
		glGenBuffers(1, &normals_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * vertex_count * sizeof(GLfloat), vn, GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribIx);
		glVertexAttribPointer(attribIx, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	++attribIx;

	uvs_vbos.resize(uvs.size());
	for (unsigned j = 0; j < 2; ++j) {
		bool hasUVs = uvs.size() > j ;
		if (hasUVs) {
			glGenBuffers(1, &uvs_vbos[j]);
			glBindBuffer(GL_ARRAY_BUFFER, uvs_vbos[j]);
			glBufferData(GL_ARRAY_BUFFER, 2 * vertex_count * sizeof(GLfloat), uvs[j], GL_STATIC_DRAW);
			glEnableVertexAttribArray(attribIx);
			glVertexAttribPointer(attribIx, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		}
		++attribIx;
	}

	if (NULL != vtans) {
		glGenBuffers(1, &tangents_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, tangents_vbo);
		glBufferData(GL_ARRAY_BUFFER, 4 * vertex_count * sizeof(GLfloat), vtans, GL_STATIC_DRAW);
		glEnableVertexAttribArray(attribIx);
		glVertexAttribPointer(attribIx, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	}
	++attribIx;

	if (faces_indices != nullptr) {
		glGenBuffers(1, &faces_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*index_count, &faces_indices[0], GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}

void Meshgroup::Mesh::load_textures_to_gpu() 
{
	load_texture_to_gpu(diffuse.image_data, &dmap_tex, diffuse.x, diffuse.y, diffuse.n);
	load_texture_to_gpu(normal.image_data, &nmap_tex, normal.x, normal.y, normal.n);
}

void Meshgroup::load_to_gpu() {

	for (unsigned m = 0; m < meshes.size(); ++m) {
		Mesh& mesh= meshes[m];
		mesh.load_geometry_to_gpu();
	}

	for (unsigned m = 0; m < meshes.size(); ++m) {
		Mesh& mesh= meshes[m];
		mesh.load_textures_to_gpu();
	}
}

void Meshgroup::get_shader_uniforms(GLuint shader_programme) 
{
	for (size_t i = 0; i < meshes.size(); ++i) {
		Mesh& mesh= meshes[i];
		mesh.get_shader_uniforms(shader_programme);
	}
}

void Meshgroup::set_shader_uniforms(GLuint shader_programme /*, const mat4& modelMatrix*/, const vec3& ambient_color) 
{
	for (size_t i = 0; i < meshes.size(); ++i) {
		Mesh& mesh = meshes[i];
		mesh.set_shader_uniforms(shader_programme, ambient_color);
	}
	//transform = modelMatrix;
}

void Meshgroup::Mesh::get_shader_uniforms(GLuint shader_programme) {

	normal_map_location = glGetUniformLocation( shader_programme, "normal_map" );
	diffuse_map_location = glGetUniformLocation( shader_programme, "diffuse_map" );
	model_matrix_location = glGetUniformLocation( shader_programme, "model" );
	diffuse_base_color_location = glGetUniformLocation( shader_programme, "diffuse_base_color" );
	ambient_color_location = glGetUniformLocation( shader_programme, "ambient_color" );
}

void Meshgroup::Mesh::set_shader_uniforms(GLuint shader_programme, const vec3& ambient_color) {
	glUniform3fv( ambient_color_location, 1, &ambient_color.v[0] );
}

void Meshgroup::Mesh::render(GLuint shader_programme) 
{
	assert(node != nullptr);

	mat4& modelMat = (*node).worldMatrix;
	glUniformMatrix4fv(model_matrix_location, 1, GL_FALSE, modelMat.m);

	glUniform3fv(diffuse_base_color_location, 1, &diffuse_base_color.v[0]);

	glUniform1i( normal_map_location, 0 );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, nmap_tex);
	
	glUniform1i( diffuse_map_location, 1 );
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, dmap_tex);

	glBindVertexArray(vao);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.faces_vbo);

	glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Meshgroup::render(GLuint shader_programme)
{
	for (size_t i = 0; i < meshes.size(); ++i) {

		Mesh& mesh= meshes[i];
		mesh.render(shader_programme);
	}
}



