#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <string>
#include <array>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "print.h"
#include "image.h"

namespace world {

	// ============================================================================================================================
	// DATA STRUCTS ===============================================================================================================

	enum vertex_attrib_size : int {
		one = 1, two, three, four
	};

	struct vertex_info_entry {
		vertex_attrib_size size;
		std::size_t offset;

		constexpr vertex_info_entry(const vertex_attrib_size size, const std::size_t offset)
			: size(size), offset(offset)
		{}
	};

	template<typename T, typename Value = std::size_t>
	constexpr Value size_of = static_cast<Value>(sizeof(T));

	struct vertex {
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec3 color{ 1.f, 1.f, 1.f };
		glm::vec3 tangent{};
		glm::vec3 bittangent{};
		glm::vec2 texture_coordinates{};

		vertex() = default;
		vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texture_coordinates)
			: position(position), normal(normal), texture_coordinates(texture_coordinates) { }
	};

	constexpr std::array<vertex_info_entry, 6> vertex_info{
		vertex_info_entry(three, 0 * size_of<glm::vec3>),
		vertex_info_entry(three, 1 * size_of<glm::vec3>),
		vertex_info_entry(three, 2 * size_of<glm::vec3>),
		vertex_info_entry(three, 3 * size_of<glm::vec3>),
		vertex_info_entry(three, 4 * size_of<glm::vec3>),
		vertex_info_entry(two, 5 * size_of<glm::vec3>)
	};

	enum texture_type {
		diffuse_texture = 0,
	};

	struct texture {
		unsigned int id;
		texture_type type;

		constexpr texture(const unsigned int id, const texture_type type)
			: id(id), type(type)
		{ }

		template<typename T>
		std::string name(const T& index) const {

			std::string temp = "texture_";
			
			switch (type)
			{
			case world::diffuse_texture:
				temp.append("diffuse_");
				break;
			default:
				break;
			}

			temp.append(std::to_string(index));

			return temp;
		}
	};

	struct model;
	struct mesh {

		mesh() = default;
		mesh(
			const std::string& name,
			std::initializer_list<vertex> verticies,
			std::initializer_list<unsigned int> indices,
			std::initializer_list<texture> textures
		) : name_(name), verticies_(verticies), indices_(indices), textures_(textures) { }

		// return the id of this mesh's VAO: Vertex Array Objects
		const unsigned int& vao() const {
			return vao_;
		}

		// return the id of this mesh's VBO: Vertex Buffer Objects
		const unsigned int& vbo() const {
			return vbo_;
		}

		// return the id of this mesh's EBO: Element Buffer Objects
		const unsigned int& ebo() const {
			return ebo_;
		}

		// returns the name of this mesh
		const std::string& name() const {
			return name_;
		}

		// returns the amount of verticies in this mesh
		std::size_t vertex_size() const {
			return verticies_.size();
		}

		// returns the amount of indicies in this mesh
		std::size_t index_size() const {
			return indices_.size();
		}

		// returns a const pointer to the first vertex of this mesh
		const vertex* first_vertex() const {
			return &verticies_[0];
		}

		// returns a const pointer to the first index of this mesh
		const unsigned int* first_index() const {
			return &indices_[0];
		}

		bool has_textures() const {
			return !textures_.empty();
		}

		template<typename Callable, std::enable_if_t<std::is_invocable<Callable, const std::size_t&, const texture&>::value, int> = 0>
		void for_each_texture(Callable fn) const {

			for (std::size_t i = 0; i < textures_.size(); i++) {
				const auto& ref = textures_.at(i);

				fn(i, ref);
			}
		}

	private:
		std::string name_;
		std::vector<vertex> verticies_{};
		std::vector<unsigned int> indices_{};
		std::vector<texture> textures_{};

		unsigned int vao_, vbo_, ebo_;

		friend void load_textures(const aiMesh* mesh_ptr, const aiScene* scene_ptr, mesh& into_mesh, const model& into_model);
		friend void load_mesh(const aiMesh* mesh_ptr, const aiScene* scene_ptr, mesh& into_mesh, const model& into_model);
		friend void load_node(const aiNode* node_ptr, const aiScene* scene_ptr, model& into_model);
		friend void setup_mesh(world::mesh& mesh);
	};

	struct rotation_values {
		float angle_x = 0.f;
		float angle_y = 0.f;
		float angle_z = 0.f;

		bool operator==(const rotation_values& rhs) const {
			return angle_x == rhs.angle_x
				&& angle_y == rhs.angle_y
				&& angle_z == rhs.angle_z;
		}

		bool operator!=(const rotation_values& rhs) const {
			return angle_x != rhs.angle_x
				|| angle_y != rhs.angle_y
				|| angle_z != rhs.angle_z;
		}
	};

	struct model {

		glm::vec3 position{ 0.f, 0.f,0.f };
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		rotation_values rotation{};

		model() = default;
		model(std::initializer_list<mesh> meshes) : meshes(meshes) {}

		// the id of the shader this model uses;
		unsigned int shader_id = 0;
		
		// return the id of this model
		const unsigned int& id() const {
			return id_;
		}

		// returns the path where this model was loaded from
		const std::string& path_to_file() const {
			return path_to_file_;
		}

		// updates the 'forward', 'right' and 'up' vectors.
		// the update will only occus if the value of 'rotation' has changed since last time;
		void update_orientation() {

			if (rotation != last_rotation) {

				const glm::quat quat(
					glm::vec3(
						glm::radians(rotation.angle_x),
						glm::radians(rotation.angle_y),
						glm::radians(rotation.angle_z)
					)
				);

				forward_ = quat * glm::vec3(0.f, 0.f, 1.f);
				up_ = quat * glm::vec3(0.f, 1.f, 0.f);
				right_ = quat * glm::vec3(1.f, 0.f, 0.f);

				last_rotation = rotation;
			}
		}

		// Returns the current forward facing vector.
		// Be sure to call 'update_orientation' to update this vector according to the current rotation.
		const glm::vec3& forward() const {
			return forward_;
		}

		// Returns the current right facing vector.
		// Be sure to call 'update_orientation' to update this vector according to the current rotation.
		const glm::vec3& right() const {
			return right_;
		}

		// Returns the current up facing vector.
		// Be sure to call 'update_orientation' to update this vector according to the current rotation.
		const glm::vec3& up() const {
			return up_;
		}

		template<typename Callable>
		void for_each_mesh(Callable func) const {
			for (std::size_t i = 0; i < meshes.size(); i++) {
				const auto& ref = meshes.at(i);
				func(i, ref);
			}
		}

		std::vector<mesh>::const_iterator begin() const {
			return meshes.begin();
		}

		std::vector<mesh>::const_iterator end() const {
			return meshes.end();
		}

		std::vector<mesh>::iterator begin() {
			return meshes.begin();
		}

		std::vector<mesh>::iterator end() {
			return meshes.end();
		}

		std::size_t meshes_size() const {
			return meshes.size();
		}

	private:
		unsigned int id_ = 0;
		std::string path_to_file_ = "";

		glm::vec3 up_{ 0.f, 1.f, 0.f };
		glm::vec3 forward_{ 0.f, 0.f, 1.f };
		glm::vec3 right_{ 1.f, 0.f, 0.f };

		rotation_values last_rotation{};

		std::vector<mesh> meshes;

		friend bool load_model(std::size_t& model_index, const char * path, unsigned int load_flags);
		friend void load_node(const aiNode* node_ptr, const aiScene* scene_ptr, model& into_model);

		template<typename ... Args> friend void create_model(std::size_t& model_index, Args&& ... args);
	};

	// ============================================================================================================================
	namespace data {
		// stores all models that have been loaded
		std::vector<model> loaded_models;
	}
	// ============================================================================================================================

	constexpr unsigned int default_load_flags = aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes;

	// ============================================================================================================================

	void load_textures(const aiMesh* mesh_ptr, const aiScene* scene_ptr, mesh& into_mesh, const model& into_model) {
		aiMaterial* material = scene_ptr->mMaterials[mesh_ptr->mMaterialIndex];

		auto tex_num = scene_ptr->mNumTextures;
		auto diffuse_amount = material->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE);

		for (unsigned int i = 0; i < diffuse_amount; i++)
		{
			auto text_path = std::filesystem::path(into_model.path_to_file());
			text_path.remove_filename();

			aiString path;
			material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, i, &path);

			text_path.append(path.C_Str());

			std::string name = mesh_ptr->mName.C_Str();
			name.append("_diffuse");

			unsigned int texture_id;
			bool was_loaded = opengl::image::load(texture_id, text_path.generic_string().c_str(), name.c_str());

			if (was_loaded) {
				into_mesh.textures_.emplace_back(texture_id, texture_type::diffuse_texture);
			}
		}
	}

	void load_mesh(const aiMesh* mesh_ptr, const aiScene* scene_ptr, mesh& into_mesh, const model& into_model) {
		if (mesh_ptr == nullptr) {
			return;
		}

		load_textures(mesh_ptr, scene_ptr, into_mesh, into_model);

		for (unsigned int i = 0; i < mesh_ptr->mNumVertices; i++)
		{
			into_mesh.verticies_.emplace_back(vertex());

			vertex& vert = into_mesh.verticies_[into_mesh.verticies_.size() - 1llu];

			if (mesh_ptr->HasPositions()) {
				vert.position.x = mesh_ptr->mVertices[i].x;
				vert.position.y = mesh_ptr->mVertices[i].y;
				vert.position.z = mesh_ptr->mVertices[i].z;
			}

			if (mesh_ptr->HasNormals()) {
				vert.normal.x = mesh_ptr->mNormals[i].x;
				vert.normal.y = mesh_ptr->mNormals[i].y;
				vert.normal.z = mesh_ptr->mNormals[i].z;
			}

			if (mesh_ptr->HasTangentsAndBitangents()) {
				vert.tangent.x = mesh_ptr->mTangents[i].x;
				vert.tangent.y = mesh_ptr->mTangents[i].y;
				vert.tangent.z = mesh_ptr->mTangents[i].z;

				vert.bittangent.x = mesh_ptr->mBitangents[i].x;
				vert.bittangent.y = mesh_ptr->mBitangents[i].y;
				vert.bittangent.z = mesh_ptr->mBitangents[i].z;
			}

			if (mesh_ptr->HasVertexColors(0u)) {
				vert.color.x = mesh_ptr->mColors[0][i].r;
				vert.color.y = mesh_ptr->mColors[0][i].g;
				vert.color.z = mesh_ptr->mColors[0][i].b;
			}

			if (mesh_ptr->HasTextureCoords(0u)) {
				vert.texture_coordinates.x = mesh_ptr->mTextureCoords[0u][i].x;
				vert.texture_coordinates.y = mesh_ptr->mTextureCoords[0u][i].y;
			}
		}

		for (unsigned int i = 0; i < mesh_ptr->mNumFaces; i++)
		{
			aiFace& face = mesh_ptr->mFaces[i];

			for (unsigned int y = 0; y < face.mNumIndices; y++) {
				into_mesh.indices_.push_back(face.mIndices[y]);
			}
		}
	}

	void load_node(const aiNode* node_ptr, const aiScene* scene_ptr, model& into_model) {
		if (node_ptr == nullptr) {
			return;
		}

		for (unsigned int i = 0; i < node_ptr->mNumMeshes; i++)
		{
			into_model.meshes.emplace_back(mesh());
			mesh& mesh = into_model.meshes[into_model.meshes.size() - 1llu];
			mesh.name_ = node_ptr->mName.C_Str();

			unsigned int mesh_index = node_ptr->mMeshes[i];
			aiMesh* mesh_ptr = scene_ptr->mMeshes[mesh_index];

			load_mesh(mesh_ptr, scene_ptr, mesh, into_model);
		}

		for (unsigned int i = 0; i < node_ptr->mNumChildren; i++)
		{
			load_node(node_ptr->mChildren[i], scene_ptr, into_model);
		}
	}

	// loads a Model using ASSIMP, returns the Index of the loaded model
	bool load_model(std::size_t& model_index, const char * path, unsigned int load_flags = default_load_flags) {
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path, load_flags);

		if (scene == nullptr || scene->mRootNode == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
			print_error("loading model failed: ", importer.GetErrorString());

			importer.FreeScene();
			return false;
		}

		data::loaded_models.push_back({});

		auto index = data::loaded_models.size() - 1llu;

		model& model_ref = data::loaded_models[index];
		model_ref.id_ = static_cast<unsigned int>(index);
		model_ref.path_to_file_ = path;

		load_node(scene->mRootNode, scene, model_ref);

		model_index = index;

		importer.FreeScene();
		return true;
	}

	template<typename ... Args>
	void create_model(std::size_t& model_index, Args&& ... args) {

		data::loaded_models.emplace_back<model>(std::forward<Args>(args)...);
		auto index = data::loaded_models.size() - 1llu;

		model& model_ref = data::loaded_models[index];
		model_ref.id_ = static_cast<unsigned int>(index);

		model_index = index;
	}

	void setup_mesh(world::mesh& mesh) {

		// create all the buffers needed to store our mesh data
		// VAO: Vertex Array Objects
		// VBO: Vertex Buffer Objects
		// EBO: Element Buffer Objects
		glGenVertexArrays(1, &mesh.vao_);
		glGenBuffers(1, &mesh.vbo_);
		glGenBuffers(1, &mesh.ebo_);

		glBindVertexArray(mesh.vao_);

		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_);
		glNamedBufferData(mesh.vbo_, mesh.vertex_size() * sizeof(world::vertex), mesh.first_vertex(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo_);
		glNamedBufferData(mesh.ebo_, mesh.index_size() * sizeof(unsigned int), mesh.first_index(), GL_STATIC_DRAW);

		constexpr auto vertex_info = world::vertex_info;
		constexpr auto size = world::size_of<world::vertex, GLsizei>;

		for (unsigned int i = 0; i < vertex_info.size(); i++)
		{
			auto& info = vertex_info.at(i);

			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, info.size, GL_FLOAT, GL_FALSE, size, (void*)info.offset);
		}

		glBindVertexArray(0);
	}

	model& model_get(std::size_t model_index) {
		return data::loaded_models.at(model_index);
	}

	void setup_model(std::size_t model_index) {
		world::model& model = world::model_get(model_index);

		for (auto & mesh : model) {
			setup_mesh(mesh);
		}
	}

	void model_set_shader(std::size_t model_index, unsigned int shader_id) {

		model& model_ref = data::loaded_models.at(model_index);
		model_ref.shader_id = shader_id;
	}

	
}