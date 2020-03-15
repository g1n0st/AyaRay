#include "ObjMesh.h"

namespace Aya {
	void ObjMesh::parserFramework(const char *filename, std::function<void(char*, char*)> callback) {
		int cmd_len, header_len;
		char command[AYA_MAX_PATH], cmd_header[AYA_MAX_PATH];

		FILE *fp;
		fopen_s(&fp, filename, "rt");
		assert(fp);

#define FREAD_BUF_SIZE 131072
		char buf[FREAD_BUF_SIZE], *p1 = buf, *p2 = buf;
		auto getChar = [&]() -> char {
			if (p1 == p2) {
				p2 = (p1 = buf) + fread(buf, 1, FREAD_BUF_SIZE, fp);
				if (p1 == p2)
					return EOF;
			}
			return *p1++;
		};
		auto fEOF = [&]() -> bool {
			return feof(fp) && (p1 == p2);
		};

		while (!fEOF()) {
			cmd_len = 0;

			char c;
			do {
				c = getChar();
			} while (c == '\t' || c == '\n' || c == '\r');
			do {
				command[cmd_len++] = c;
				c = getChar();
			} while (c != '\n' && c != '\r' && c != EOF);
			//while (command[cmd_len] = getChar(),
			//	command[cmd_len] != '\n' && command[cmd_len] != EOF)
			//	++cmd_len;
			command[cmd_len] = 0;

			// Header Part
			header_len = 0;
			while (command[header_len] != ' ' && header_len < cmd_len) {
				cmd_header[header_len] = command[header_len];
				header_len++;
			}
			cmd_header[header_len] = 0;

			// Parameter Part
			char *cmd_para = command + header_len + 1;
			
			callback(cmd_header, cmd_para);
		}

		fclose(fp);
	}
	bool ObjMesh::loadObj(const char *path, const bool force_compute_normal, const bool left_handed) {
		std::vector<Point3> position_buff;
		std::vector<Normal3> normal_buff;
		std::vector<Vector2f> uv_buff;
		int smoothing_group = force_compute_normal ? 1 : 0;
		bool has_smooth_group = false;
		int current_mtl = 0;
		char mtl_filename[AYA_MAX_PATH] = { 0 };

		parserFramework(path, [&] (char *cmd_header, char *cmd_para) {
			if (0 == std::strcmp(cmd_header, "#")) {
				// Comment
			}
			else if (0 == std::strcmp(cmd_header, "v")) {
				// Vertex Position
				float x, y, z;
				sscanf_s(cmd_para, "%f %f %f", &x, &y, &z);
				if (left_handed)
					position_buff.emplace_back(-x, y, z);
				else
					position_buff.emplace_back(x, y, z);
			}
			else if (0 == std::strcmp(cmd_header, "vt")) {
				// Vertex TexCoord
				float u, v;
				sscanf_s(cmd_para, "%f %f", &u, &v);
				uv_buff.emplace_back(u, 1.f - v);
				m_textured = true;
			}
			else if (0 == std::strcmp(cmd_header, "vn")) {
				// Vertex Normal
				float x, y, z;
				sscanf_s(cmd_para, "%f %f %f", &x, &y, &z);
				if (left_handed)
					normal_buff.emplace_back(-x, y, z);
				else
					normal_buff.emplace_back(x, y, z);
			}
			else if (0 == std::strcmp(cmd_header, "s")) {
				// smoothing group for normal computation
				if (cmd_para[0] >= '1' && cmd_para[0] <= '9') {
					has_smooth_group = true;
					sscanf_s(cmd_para, "%d", &smoothing_group);
				}
				else
					smoothing_group = 0;
			}
			else if (0 == std::strcmp(cmd_header, "mtllib")) {
				// Material library
				sscanf_s(cmd_para, "%s", mtl_filename, AYA_MAX_PATH);
			}
			else if (0 == std::strcmp(cmd_header, "usemtl")) {
				char name[AYA_MAX_PATH];
				sscanf_s(cmd_para, "%s", name, AYA_MAX_PATH);

				ObjMaterial mtl(name);
				auto idx_iter = std::find(m_materials.begin(), m_materials.end(), mtl);
				if (idx_iter == m_materials.end()) {
					current_mtl = int(m_materials.size());
					m_materials.push_back(mtl);
				}
				else {
					current_mtl = int(idx_iter - m_materials.begin());
				}

				m_subset_start_idx.push_back(int(m_indices.size()));
				m_subset_mtl_idx.push_back(current_mtl);
				m_subset_count++;
			}
			else if (0 == std::strcmp(cmd_header, "f")) {
				// Face
				int len = int(std::strlen(cmd_para));

				int pos_idx, uv_idx, normal_idx;
				MeshVertex vertex;
				MeshFace face, quad_face;

				uint32_t face_idx[4] = { 0, 0, 0, 0 };
				int vertex_count = 0;

				int slash_count = -1;
				bool double_slash = false;
				auto start_idx = 0;
				for (auto i = 0; i <= len; i++) {
					char c = cmd_para[i];
					if (c != ' ' && c != '\t' && c != '\n' && c != '\0')
						continue;
					// Move to data header
					if (start_idx == i) {
						start_idx++;
						continue;
					}
					if (slash_count == -1) {
						slash_count = 0;
						for (auto cur = start_idx; cur < i; cur++) {
							if (cmd_para[cur] == '/') {
								if (cur - 1 >= 0 && cmd_para[cur - 1] == '/')
									double_slash = true;
								slash_count++;
							}
						}
					}

					if (double_slash) {
						sscanf_s(cmd_para + start_idx, "%d//%d", &pos_idx, &normal_idx);
						if (pos_idx < 0) pos_idx = int(position_buff.size()) + pos_idx + 1;
						if (normal_idx < 0) normal_idx = int(normal_buff.size()) + normal_idx + 1;
						vertex.p = position_buff[pos_idx - 1];
						vertex.n = normal_buff[normal_idx - 1];
					}
					else {
						if (slash_count == 0) {
							sscanf_s(cmd_para + start_idx, "%d", &pos_idx);
							if (pos_idx < 0) pos_idx = int(position_buff.size()) + pos_idx + 1;
							vertex.p = position_buff[pos_idx - 1];
						}
						else if (slash_count == 1) {
							sscanf_s(cmd_para + start_idx, "%d/%d", &pos_idx, &uv_idx);
							if (pos_idx < 0) pos_idx = int(position_buff.size()) + pos_idx + 1;
							if (uv_idx < 0) uv_idx = int(uv_buff.size()) / 2 + uv_idx + 1;
							vertex.p = position_buff[pos_idx - 1];
							vertex.uv = uv_buff[uv_idx - 1];
						}
						else if (slash_count == 2) {
							sscanf_s(cmd_para + start_idx, "%d/%d/%d", &pos_idx, &uv_idx, &normal_idx);
							if (pos_idx < 0) pos_idx = int(position_buff.size()) + pos_idx + 1;
							if (uv_idx < 0) uv_idx = int(uv_buff.size()) / 2 + uv_idx + 1;
							if (normal_idx < 0) normal_idx = int(normal_buff.size()) + normal_idx + 1;
							vertex.p = position_buff[pos_idx - 1];
							vertex.uv = uv_buff[uv_idx - 1];
							vertex.n = normal_buff[normal_idx - 1];
						}
					}

					if (vertex_count >= 4)
						break;

					face_idx[vertex_count] = addVertex(pos_idx - 1, &vertex);
					++vertex_count;
					start_idx = i + 1;
				}

				if (left_handed) {
					face.idx[0] = face_idx[0];
					face.idx[1] = face_idx[2];
					face.idx[2] = face_idx[1];
				}
				else {
					face.idx[0] = face_idx[0];
					face.idx[1] = face_idx[1];
					face.idx[2] = face_idx[2];
				}

				m_indices.push_back(face.idx[0]);
				m_indices.push_back(face.idx[1]);
				m_indices.push_back(face.idx[2]);

				face.smoothing_group = smoothing_group;
				m_faces.push_back(face);
				m_material_idx.push_back(current_mtl);

				if (vertex_count == 4) {
					// Trianglarize quad
					if (left_handed) {
						quad_face.idx[0] = face_idx[3];
						quad_face.idx[1] = face.idx[1];
						quad_face.idx[2] = face.idx[0];
					}
					else {
						quad_face.idx[0] = face_idx[3];
						quad_face.idx[1] = face.idx[0];
						quad_face.idx[2] = face.idx[2];
					}

					m_indices.push_back(quad_face.idx[0]);
					m_indices.push_back(quad_face.idx[1]);
					m_indices.push_back(quad_face.idx[2]);

					quad_face.smoothing_group = smoothing_group;
					m_faces.push_back(quad_face);
					m_material_idx.push_back(current_mtl);
				}
			}
			else {
				// Ignore
			}
		});

		if (m_subset_count == 0) {
			m_subset_start_idx.push_back(0);
			m_subset_mtl_idx.push_back(0);
			m_subset_count = 1;
		}

		m_subset_start_idx.push_back(uint32_t(m_indices.size()));
		m_vertex_count = uint32_t(m_vertices.size());
		m_triangle_count = uint32_t(m_indices.size()) / 3;

		// Recomputed pre-vertex normals
		if (force_compute_normal || has_smooth_group || !m_normaled)
			computeVertexNormals();

		for (auto list : m_caches) {
			while (list != NULL) {
				auto next = list->next;
				SafeDelete(list);
				list = next;
			}
		}
		m_caches.clear();

		if (mtl_filename[0]) {
			const char *path1 = strrchr(path, '/');
			const char *path2 = strrchr(path, '\\');
			if (path1 || path2) {
				int idx = int((path1 ? path1 : path2) - path + 1);
				char mtl_path[AYA_MAX_PATH] = { 0 };
				strncpy_s(mtl_path, AYA_MAX_PATH, path, idx);
				strcat(mtl_path, mtl_filename);
				loadMtl(mtl_path);
			}
			else {
				loadMtl(mtl_filename);
			}
		}

		if (!m_materials.size())
			m_materials.push_back(ObjMaterial());

		return true;
	}
	void ObjMesh::loadMtl(const char *path) {
		int current_material = -1;
		parserFramework(path, [this, &current_material, path](char *cmd_header, char *cmd_para) {
			if (0 == std::strcmp(cmd_header, "#")) {
				// Comment
			}
			else if (0 == std::strcmp(cmd_header, "newmtl")) {
				// Switching active materials
				char name[AYA_MAX_PATH];
				sscanf_s(cmd_para, "%s", name, AYA_MAX_PATH);
				ObjMaterial mtl(name);
				current_material = int(std::find(m_materials.begin(), m_materials.end(), mtl) - m_materials.begin());
			}

			if (!~current_material)
				return;

			else if (0 == std::strcmp(cmd_header, "Ni")) {
				// Refractive Index
				float ni;
				sscanf_s(cmd_para, "%f", &ni);
				m_materials[current_material].refractive_index = ni;
			}
			else if (0 == std::strcmp(cmd_header, "Kd")) {
				// Diffuse color
				float r, g, b;
				sscanf_s(cmd_para, "%f %f %f", &r, &g, &b);
				m_materials[current_material].diffuse_color = Spectrum::fromRGB(r, g, b);
			}
			else if (0 == std::strcmp(cmd_header, "Ks")) {
				// Specular color
				float r, g, b;
				sscanf_s(cmd_para, "%f %f %f", &r, &g, &b);
				m_materials[current_material].specular_color = Spectrum::fromRGB(r, g, b);
			}
			else if (0 == std::strcmp(cmd_header, "Tf")) {
				// Transmission color
				float r, g, b;
				sscanf_s(cmd_para, "%f %f %f", &r, &g, &b);
				m_materials[current_material].trans_color = Spectrum::fromRGB(r, g, b);
			}
			else if (0 == std::strcmp(cmd_header, "d") ||
				0 == std::strcmp(cmd_header, "Tr")) {
				// Alpha
				sscanf_s(cmd_para, "%f", &m_materials[current_material].diffuse_color[3]);
			}
			else if (0 == std::strcmp(cmd_header, "map_Kd")) {
				// Texture Map
				
				const char *path1 = std::strrchr(path, '/');
				const char *path2 = std::strrchr(path,  '\\');

				if (path1 || path2) {
					int idx = int((path1 ? path1 : path2) - path + 1);
					strncpy_s(m_materials[current_material].texture_path, AYA_MAX_PATH, path, idx);
				}
				strcat_s(m_materials[current_material].texture_path, AYA_MAX_PATH, cmd_para);
			}
			else if (0 == std::strcmp(cmd_header, "bump")) {
				// Bump Map
				if (!m_materials[current_material].bump_path[0]) {
					const char *path1 = std::strrchr(path, '/');
					const char *path2 = std::strrchr(path, '\\');
					
					if (path1 || path2) {
						int idx = int((path1 ? path1 : path2) - path + 1);
						strncpy_s(m_materials[current_material].bump_path, AYA_MAX_PATH, path, idx);
					}
					strcat_s(m_materials[current_material].bump_path, AYA_MAX_PATH, cmd_para);
				}
			}
			else {
				// Ignore
			}
		});
	}
	uint32_t ObjMesh::addVertex(uint32_t hash, const MeshVertex *vertex) {
		bool is_found = false;
		uint32_t idx = 0;

		if (m_caches.size() > hash) {
			Cache *list = m_caches[hash];
			while (list != NULL) {
				MeshVertex *cache_vertex = m_vertices.data() + list->idx;
				if (*cache_vertex == *vertex) {
					is_found = true;
					idx = list->idx;
					break;
				}
				list = list->next;
			}
		}

		if (!is_found) {
			idx = uint32_t(m_vertices.size());
			m_vertices.push_back(*vertex);

			Cache *cache = new Cache();
			if (cache == NULL)
				return uint32_t(-1);

			cache->idx = idx;
			cache->next = NULL;

			while (m_caches.size() <= hash)
				m_caches.push_back(NULL);

			Cache *list = m_caches[hash];

			if (list == NULL)
				m_caches[hash] = cache;
			else {
				while (list->next != NULL)
					list = list->next;
				list->next = cache;
			}
		}

		return idx;
	}
	void ObjMesh::computeVertexNormals() {
		// Compute per face Normals
		const Normal3 ZERO_NORMAL(0.f, 0.f, 0.f);

		std::vector<Normal3> face_normal;
		face_normal.resize(m_faces.size());
		for (auto i = 0; i < m_faces.size(); ++i) {
			const Point3 &p1 = getVertexAt(m_faces[i].idx[0]).p;
			const Point3 &p2 = getVertexAt(m_faces[i].idx[1]).p;
			const Point3 &p3 = getVertexAt(m_faces[i].idx[2]).p;

			Vector3 v1 = p2 - p1;
			Vector3 v2 = p3 - p1;
			Vector3 crossed = v1.cross(v2);
			if (crossed.length() > 0.f)
				face_normal[i] = crossed.normalize();
			else
				face_normal[i] = ZERO_NORMAL;
		}

		struct VertexFace {
			std::vector<int> list;
		};
		std::vector<VertexFace> vertex_face_list;
		vertex_face_list.resize(m_vertices.size());
		for (auto i = 0; i < m_faces.size(); ++i) {
			vertex_face_list[m_faces[i].idx[0]].list.push_back(i);
			vertex_face_list[m_faces[i].idx[1]].list.push_back(i);
			vertex_face_list[m_faces[i].idx[2]].list.push_back(i);
		}

		// Compute per vertex normals with smoothing group
		for (int i = 0; i < m_faces.size(); i++) {
			const MeshFace &face = m_faces[i];
			for (auto j = 0; j < 3; j++) {
				int face_count = 0;
				Normal3 normal = ZERO_NORMAL;
				for (auto k = 0; k < vertex_face_list[face.idx[j]].list.size(); k++) {
					int face_idx = vertex_face_list[face.idx[j]].list[k];
					if (face.smoothing_group & m_faces[face_idx].smoothing_group) {
						normal += face_normal[face_idx];
						face_count++;
					}
				}

				if (face_count > 0)
					normal /= float(face_count);
				else
					normal = face_normal[i];

				if (normal.length() > 0.f)
					normal = normal.normalize();
				else
					normal = ZERO_NORMAL;

				MeshVertex &vert = m_vertices[face.idx[j]];
				if (vert.n == ZERO_NORMAL)
					vert.n = normal;
				else if (vert.n != normal) {
					MeshVertex new_vert = vert;
					new_vert.n = normal;
					auto idx = addVertex(face.idx[j], &new_vert);
					m_indices[3 * i + j] = idx;
				}
			}
		}

		m_vertex_count = uint32_t(m_vertices.size());
		m_normaled = true;
	}
}