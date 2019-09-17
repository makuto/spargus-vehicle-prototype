#include "ModelRender.hpp"

#include <iostream>
#include <GL/glew.h>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>

#include "tiny_gltf.h"

#include <cstdio>
#define CheckGLErrors(desc)                                                                    \
	{                                                                                          \
		GLenum e = glGetError();                                                               \
		if (e != GL_NO_ERROR)                                                                  \
		{                                                                                      \
			printf("OpenGL error in \"%s\": %d (%d) %s:%d\n", desc, e, e, __FILE__, __LINE__); \
			exit(20);                                                                          \
		}                                                                                      \
	}

void drawModel(const tinygltf::Model& model)
{
	glBegin(GL_TRIANGLES);

	// TODO: Separate lists for separate primitives
	for (const tinygltf::Mesh& mesh : model.meshes)
	{
		for (const tinygltf::Primitive& primitive : mesh.primitives)
		{
			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
			// gBufferState[indexAccessor.bufferView].vb); 		CheckGLErrors("bind buffer"); 		int
			// mode = -1;
			int mode = -1;

			switch (primitive.mode)
			{
				case TINYGLTF_MODE_TRIANGLES:
				{
					mode = GL_TRIANGLES;
					std::cout << "Primitive mode = GL_TRIANGLES\n";
					break;
				}
				case TINYGLTF_MODE_TRIANGLE_STRIP:
				{
					mode = GL_TRIANGLE_STRIP;
					std::cout << "Primitive mode = GL_TRIANGLE_STRIP\n";
					break;
				}
				case TINYGLTF_MODE_TRIANGLE_FAN:
				{
					mode = GL_TRIANGLE_FAN;
					std::cout << "Primitive mode = GL_TRIANGLE_FAN\n";
					break;
				}
				case TINYGLTF_MODE_POINTS:
				{
					mode = GL_POINTS;
					std::cout << "Primitive mode = GL_POINTS\n";
					break;
				}
				case TINYGLTF_MODE_LINE:
				{
					mode = GL_LINES;
					std::cout << "Primitive mode = GL_LINES\n";
					break;
				}
				case TINYGLTF_MODE_LINE_LOOP:
				{
					mode = GL_LINE_LOOP;
					std::cout << "Primitive mode = GL_LINE_LOOP\n";
					break;
				}
			}

			std::cout << indexAccessor.count << " things\n";
			glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
			               reinterpret_cast<void*>(0 + indexAccessor.byteOffset));

			// assert(mode = GL_TRIANGLES);  // Hitting this? Mode not supported
			// for (size_t vertexIndex = 0; vertexIndex < indexAccessor.count; ++vertexIndex)
			// {
			// 	glNormal3f(0, 1, 0);
			// 	float* vertex = reinterpret_cast<float*>(indexAccessor.byteOffset + (sizeof(float) * vertexIndex));
			// 	glVertex3f(vertex[0], vertex[1], vertex[2]);
			// }
		}
	}

	glEnd();
}

typedef struct
{
	std::map<std::string, GLint> attribs;
	std::map<std::string, GLint> uniforms;
} GLProgramState;

GLProgramState gGLProgramState;

typedef struct
{
	GLuint vb;
} GLBufferState;

std::map<int, GLBufferState> gBufferState;

GLCallListIndex buildCallListFromModel(const tinygltf::Model& model)
{
	GLCallListIndex modelCallList = glGenLists(1);
	glNewList(modelCallList, GL_COMPILE);
	glBegin(GL_TRIANGLES);

	// TODO: Separate lists for separate primitives
	for (const tinygltf::Mesh& mesh : model.meshes)
	{
		for (const tinygltf::Primitive& primitive : mesh.primitives)
		{
			for (std::map<std::string, int>::const_iterator it = primitive.attributes.begin();
			     it != primitive.attributes.end(); it++)
			{
				assert(it->second >= 0);
				const tinygltf::Accessor& accessor = model.accessors[it->second];
				glBindBuffer(GL_ARRAY_BUFFER, gBufferState[accessor.bufferView].vb);
				// CheckGLErrors("bind buffer");
				int size = 1;
				if (accessor.type == TINYGLTF_TYPE_SCALAR)
				{
					size = 1;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC2)
				{
					size = 2;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC3)
				{
					size = 3;
				}
				else if (accessor.type == TINYGLTF_TYPE_VEC4)
				{
					size = 4;
				}
				else
				{
					assert(0);
				}
				// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
				if ((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) ||
				    (it->first.compare("TEXCOORD_0") == 0))
				{
					if (gGLProgramState.attribs[it->first] >= 0)
					{
						// Compute byteStride from Accessor + BufferView combination.
						int byteStride =
						    accessor.ByteStride(model.bufferViews[accessor.bufferView]);
						assert(byteStride != -1);
						glVertexAttribPointer(gGLProgramState.attribs[it->first], size,
						                      accessor.componentType,
						                      accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
						                      reinterpret_cast<void*>(0 + accessor.byteOffset));
						// // CheckGLErrors("vertex attrib pointer");
						glEnableVertexAttribArray(gGLProgramState.attribs[it->first]);
						// // CheckGLErrors("enable vertex attrib array");
					}
				}
			}

			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferState[indexAccessor.bufferView].vb);
			// CheckGLErrors("bind buffer");
			int mode = -1;

			switch (primitive.mode)
			{
				case TINYGLTF_MODE_TRIANGLES:
				{
					mode = GL_TRIANGLES;
					std::cout << "Primitive mode = GL_TRIANGLES\n";
					break;
				}
				case TINYGLTF_MODE_TRIANGLE_STRIP:
				{
					mode = GL_TRIANGLE_STRIP;
					std::cout << "Primitive mode = GL_TRIANGLE_STRIP\n";
					break;
				}
				case TINYGLTF_MODE_TRIANGLE_FAN:
				{
					mode = GL_TRIANGLE_FAN;
					std::cout << "Primitive mode = GL_TRIANGLE_FAN\n";
					break;
				}
				case TINYGLTF_MODE_POINTS:
				{
					mode = GL_POINTS;
					std::cout << "Primitive mode = GL_POINTS\n";
					break;
				}
				case TINYGLTF_MODE_LINE:
				{
					mode = GL_LINES;
					std::cout << "Primitive mode = GL_LINES\n";
					break;
				}
				case TINYGLTF_MODE_LINE_LOOP:
				{
					mode = GL_LINE_LOOP;
					std::cout << "Primitive mode = GL_LINE_LOOP\n";
					break;
				}
			}

			std::cout << indexAccessor.count << " things\n";
			glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
			               reinterpret_cast<void*>(0 + indexAccessor.byteOffset));

			// assert(mode = GL_TRIANGLES);  // Hitting this? Mode not supported
			// for (size_t vertexIndex = 0; vertexIndex < indexAccessor.count; ++vertexIndex)
			// {
			// 	glNormal3f(0, 1, 0);
			// 	float* vertex = reinterpret_cast<float*>(indexAccessor.byteOffset + (sizeof(float) * vertexIndex));
			// 	glVertex3f(vertex[0], vertex[1], vertex[2]);
			// }
		}
	}

	glEnd();
	glEndList();

	return modelCallList;
}

// #define BUFFER_OFFSET(i) ((char*)NULL + (i))

// void drawModel(const tinygltf::Model& model)
// {
// 	// for (const tinygltf::Mesh& mesh : model.meshes)
// 	// {
// 	// 	for (const tinygltf::Primitive& primitive : mesh.primitives)
// 	// 	{
// 	// 		const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
	// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferState[indexAccessor.bufferView].vb);
// 	// 		CheckGLErrors("bind buffer");
// 	// 		int mode = -1;
// 	// 		if (primitive.mode == TINYGLTF_MODE_TRIANGLES)
// 	// 		{
// 	// 			mode = GL_TRIANGLES;
// 	// 		}
// 	// 		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
// 	// 		{
// 	// 			mode = GL_TRIANGLE_STRIP;
// 	// 		}
// 	// 		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
// 	// 		{
// 	// 			mode = GL_TRIANGLE_FAN;
// 	// 		}
// 	// 		else if (primitive.mode == TINYGLTF_MODE_POINTS)
// 	// 		{
// 	// 			mode = GL_POINTS;
// 	// 		}
// 	// 		else if (primitive.mode == TINYGLTF_MODE_LINE)
// 	// 		{
// 	// 			mode = GL_LINES;
// 	// 		}
// 	// 		else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP)
// 	// 		{
// 	// 			mode = GL_LINE_LOOP;
// 	// 		}
// 	// 		else
// 	// 		{
// 	// 			assert(0);
// 	// 		}
// 	// 		glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
// 	// 		               BUFFER_OFFSET(indexAccessor.byteOffset));
// 	// 	}
// 	// }
// }

GLCallListIndex buildCallListFromModel2(const tinygltf::Model& model)
{
	GLCallListIndex modelCallList = glGenLists(1);
	glNewList(modelCallList, GL_COMPILE);
	glBegin(GL_TRIANGLES);

	// TODO: Separate lists for separate primitives
	for (const tinygltf::Mesh& mesh : model.meshes)
	{
		for (const tinygltf::Primitive& primitive : mesh.primitives)
		{
			if (primitive.mode != TINYGLTF_MODE_TRIANGLES)
			{
				std::cout << "Warning: Primitive mode not supported\n";
				continue;
			}

			std::string positionAttributeName = "POSITION";
			int vertexPositionIndex = -1;
			
			// But why though
			for (const std::pair<std::string, int>& attributeIndexPair : primitive.attributes)
			{
				if (attributeIndexPair.first == positionAttributeName)
				{
					vertexPositionIndex = attributeIndexPair.second;
					break;
				}
			}
			if (vertexPositionIndex == -1)
			{
				std::cout << "Error: could not find positions\n";
				continue;
			}
			
			const tinygltf::Accessor& accessor = model.accessors[vertexPositionIndex];
			std::cout << accessor.count << " vertices\n";

			// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, nullptr);
		}
	}

	glEnd();
	glEndList();

	return modelCallList;
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static size_t ComponentTypeByteSize(int type) {
  switch (type) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return sizeof(char);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return sizeof(short);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    case TINYGLTF_COMPONENT_TYPE_INT:
      return sizeof(int);
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return sizeof(float);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return sizeof(double);
    default:
      return 0;
  }
}

// static void SetupMeshState(const tinygltf::Model& model, GLuint progId)
// {
// 	// Buffer
// 	{
// 		for (size_t i = 0; i < model.bufferViews.size(); i++)
// 		{
// 			const tinygltf::BufferView& bufferView = model.bufferViews[i];
// 			if (bufferView.target == 0)
// 			{
// 				std::cout << "WARN: bufferView.target is zero" << std::endl;
// 				continue;  // Unsupported bufferView.
// 			}

// 			int sparse_accessor = -1;
// 			for (size_t a_i = 0; a_i < model.accessors.size(); ++a_i)
// 			{
// 				const auto& accessor = model.accessors[a_i];
// 				if (accessor.bufferView == i)
// 				{
// 					std::cout << i << " is used by accessor " << a_i << std::endl;
// 					if (accessor.sparse.isSparse)
// 					{
// 						std::cout << "WARN: this bufferView has at least one sparse accessor to "
// 						             "it. We are going to load the data as patched by this "
// 						             "sparse accessor, not the original data"
// 						          << std::endl;
// 						sparse_accessor = a_i;
// 						break;
// 					}
// 				}
// 			}

// 			const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
// 			GLBufferState state;
// 			glGenBuffers(1, &state.vb);
// 			glBindBuffer(bufferView.target, state.vb);
// 			std::cout << "buffer.size= " << buffer.data.size()
// 			          << ", byteOffset = " << bufferView.byteOffset << std::endl;

// 			if (sparse_accessor < 0)
// 				glBufferData(bufferView.target, bufferView.byteLength,
// 				             &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
// 			else
// 			{
// 				const auto accessor = model.accessors[sparse_accessor];
// 				// copy the buffer to a temporary one for sparse patching
// 				unsigned char* tmp_buffer = new unsigned char[bufferView.byteLength];
// 				memcpy(tmp_buffer, buffer.data.data() + bufferView.byteOffset,
// 				       bufferView.byteLength);

// 				const size_t size_of_object_in_buffer =
// 				    ComponentTypeByteSize(accessor.componentType);
// 				const size_t size_of_sparse_indices =
// 				    ComponentTypeByteSize(accessor.sparse.indices.componentType);

// 				const auto& indices_buffer_view =
// 				    model.bufferViews[accessor.sparse.indices.bufferView];
// 				const auto& indices_buffer = model.buffers[indices_buffer_view.buffer];

// 				const auto& values_buffer_view =
// 				    model.bufferViews[accessor.sparse.values.bufferView];
// 				const auto& values_buffer = model.buffers[values_buffer_view.buffer];

// 				for (size_t sparse_index = 0; sparse_index < accessor.sparse.count; ++sparse_index)
// 				{
// 					int index = 0;
// 					// std::cout << "accessor.sparse.indices.componentType = " <<
// 					// accessor.sparse.indices.componentType << std::endl;
// 					switch (accessor.sparse.indices.componentType)
// 					{
// 						case TINYGLTF_COMPONENT_TYPE_BYTE:
// 						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
// 							index = (int)*(unsigned char*)(indices_buffer.data.data() +
// 							                               indices_buffer_view.byteOffset +
// 							                               accessor.sparse.indices.byteOffset +
// 							                               (sparse_index * size_of_sparse_indices));
// 							break;
// 						case TINYGLTF_COMPONENT_TYPE_SHORT:
// 						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
// 							index =
// 							    (int)*(unsigned short*)(indices_buffer.data.data() +
// 							                            indices_buffer_view.byteOffset +
// 							                            accessor.sparse.indices.byteOffset +
// 							                            (sparse_index * size_of_sparse_indices));
// 							break;
// 						case TINYGLTF_COMPONENT_TYPE_INT:
// 						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
// 							index = (int)*(unsigned int*)(indices_buffer.data.data() +
// 							                              indices_buffer_view.byteOffset +
// 							                              accessor.sparse.indices.byteOffset +
// 							                              (sparse_index * size_of_sparse_indices));
// 							break;
// 					}
// 					std::cout << "updating sparse data at index  : " << index << std::endl;
// 					// index is now the target of the sparse index to patch in
// 					const unsigned char* read_from =
// 					    values_buffer.data.data() +
// 					    (values_buffer_view.byteOffset + accessor.sparse.values.byteOffset) +
// 					    (sparse_index * (size_of_object_in_buffer * accessor.type));

// 					/*
// 					std::cout << ((float*)read_from)[0] << "\n";
// 					std::cout << ((float*)read_from)[1] << "\n";
// 					std::cout << ((float*)read_from)[2] << "\n";
// 					*/

// 					unsigned char* write_to =
// 					    tmp_buffer + index * (size_of_object_in_buffer * accessor.type);

// 					memcpy(write_to, read_from, size_of_object_in_buffer * accessor.type);
// 				}

// 				// debug:
// 				/*for(size_t p = 0; p < bufferView.byteLength/sizeof(float); p++)
// 				{
// 				  float* b = (float*)tmp_buffer;
// 				  std::cout << "modified_buffer [" << p << "] = " << b[p] << '\n';
// 				}*/

// 				glBufferData(bufferView.target, bufferView.byteLength, tmp_buffer, GL_STATIC_DRAW);
// 				delete[] tmp_buffer;
// 			}
// 			glBindBuffer(bufferView.target, 0);

// 			gBufferState[i] = state;
// 		}
// 	}

// #if 0  // TODO(syoyo): Implement
// 	// Texture
// 	{
// 		for (size_t i = 0; i < model.meshes.size(); i++) {
// 			const tinygltf::Mesh &mesh = model.meshes[i];

// 			gMeshState[mesh.name].diffuseTex.resize(mesh.primitives.size());
// 			for (size_t primId = 0; primId < mesh.primitives.size(); primId++) {
// 				const tinygltf::Primitive &primitive = mesh.primitives[primId];

// 				gMeshState[mesh.name].diffuseTex[primId] = 0;

// 				if (primitive.material < 0) {
// 					continue;
// 				}
// 				tinygltf::Material &mat = model.materials[primitive.material];
// 				// printf("material.name = %s\n", mat.name.c_str());
// 				if (mat.values.find("diffuse") != mat.values.end()) {
// 					std::string diffuseTexName = mat.values["diffuse"].string_value;
// 					if (model.textures.find(diffuseTexName) != model.textures.end()) {
// 						tinygltf::Texture &tex = model.textures[diffuseTexName];
// 						if (scene.images.find(tex.source) != model.images.end()) {
// 							tinygltf::Image &image = model.images[tex.source];
// 							GLuint texId;
// 							glGenTextures(1, &texId);
// 							glBindTexture(tex.target, texId);
// 							glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// 							glTexParameterf(tex.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 							glTexParameterf(tex.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// 							// Ignore Texture.fomat.
// 							GLenum format = GL_RGBA;
// 							if (image.component == 3) {
// 								format = GL_RGB;
// 							}
// 							glTexImage2D(tex.target, 0, tex.internalFormat, image.width,
// 									image.height, 0, format, tex.type,
// 									&image.image.at(0));

// 							CheckGLErrors("texImage2D");
// 							glBindTexture(tex.target, 0);

// 							printf("TexId = %d\n", texId);
// 							gMeshState[mesh.name].diffuseTex[primId] = texId;
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
// #endif

// 	glUseProgram(progId);
// 	GLint vtloc = glGetAttribLocation(progId, "in_vertex");
// 	GLint nrmloc = glGetAttribLocation(progId, "in_normal");
// 	GLint uvloc = glGetAttribLocation(progId, "in_texcoord");

// 	// GLint diffuseTexLoc = glGetUniformLocation(progId, "diffuseTex");
// 	GLint isCurvesLoc = glGetUniformLocation(progId, "uIsCurves");

// 	gGLProgramState.attribs["POSITION"] = vtloc;
// 	gGLProgramState.attribs["NORMAL"] = nrmloc;
// 	gGLProgramState.attribs["TEXCOORD_0"] = uvloc;
// 	// gGLProgramState.uniforms["diffuseTex"] = diffuseTexLoc;
// 	gGLProgramState.uniforms["isCurvesLoc"] = isCurvesLoc;
// }

// static void DrawMesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh)
// {
// 	//// Skip curves primitive.
// 	// if (gCurvesMesh.find(mesh.name) != gCurvesMesh.end()) {
// 	//  return;
// 	//}

// 	// if (gGLProgramState.uniforms["diffuseTex"] >= 0) {
// 	//  glUniform1i(gGLProgramState.uniforms["diffuseTex"], 0);  // TEXTURE0
// 	//}

// 	if (gGLProgramState.uniforms["isCurvesLoc"] >= 0)
// 	{
// 		glUniform1i(gGLProgramState.uniforms["isCurvesLoc"], 0);
// 	}

// 	for (size_t i = 0; i < mesh.primitives.size(); i++)
// 	{
// 		const tinygltf::Primitive& primitive = mesh.primitives[i];

// 		if (primitive.indices < 0)
// 			return;

// 		// Assume TEXTURE_2D target for the texture object.
// 		// glBindTexture(GL_TEXTURE_2D, gMeshState[mesh.name].diffuseTex[i]);

// 		std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
// 		std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

// 		for (; it != itEnd; it++)
// 		{
// 			assert(it->second >= 0);
// 			const tinygltf::Accessor& accessor = model.accessors[it->second];
// 			glBindBuffer(GL_ARRAY_BUFFER, gBufferState[accessor.bufferView].vb);
// 			CheckGLErrors("bind buffer");
// 			int size = 1;
// 			if (accessor.type == TINYGLTF_TYPE_SCALAR)
// 			{
// 				size = 1;
// 			}
// 			else if (accessor.type == TINYGLTF_TYPE_VEC2)
// 			{
// 				size = 2;
// 			}
// 			else if (accessor.type == TINYGLTF_TYPE_VEC3)
// 			{
// 				size = 3;
// 			}
// 			else if (accessor.type == TINYGLTF_TYPE_VEC4)
// 			{
// 				size = 4;
// 			}
// 			else
// 			{
// 				assert(0);
// 			}
// 			// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
// 			if ((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) ||
// 			    (it->first.compare("TEXCOORD_0") == 0))
// 			{
// 				if (gGLProgramState.attribs[it->first] >= 0)
// 				{
// 					// Compute byteStride from Accessor + BufferView combination.
// 					int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
// 					assert(byteStride != -1);
// 					glVertexAttribPointer(gGLProgramState.attribs[it->first], size,
// 					                      accessor.componentType,
// 					                      accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
// 					                      BUFFER_OFFSET(accessor.byteOffset));
// 					CheckGLErrors("vertex attrib pointer");
// 					glEnableVertexAttribArray(gGLProgramState.attribs[it->first]);
// 					CheckGLErrors("enable vertex attrib array");
// 				}
// 			}
// 		}

// 		const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferState[indexAccessor.bufferView].vb);
// 		CheckGLErrors("bind buffer");
// 		int mode = -1;
// 		if (primitive.mode == TINYGLTF_MODE_TRIANGLES)
// 		{
// 			mode = GL_TRIANGLES;
// 		}
// 		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP)
// 		{
// 			mode = GL_TRIANGLE_STRIP;
// 		}
// 		else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN)
// 		{
// 			mode = GL_TRIANGLE_FAN;
// 		}
// 		else if (primitive.mode == TINYGLTF_MODE_POINTS)
// 		{
// 			mode = GL_POINTS;
// 		}
// 		else if (primitive.mode == TINYGLTF_MODE_LINE)
// 		{
// 			mode = GL_LINES;
// 		}
// 		else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP)
// 		{
// 			mode = GL_LINE_LOOP;
// 		}
// 		else
// 		{
// 			assert(0);
// 		}
// 		glDrawElements(mode, indexAccessor.count, indexAccessor.componentType,
// 		               BUFFER_OFFSET(indexAccessor.byteOffset));
// 		CheckGLErrors("draw elements");

// 		{
// 			std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
// 			std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

// 			for (; it != itEnd; it++)
// 			{
// 				if ((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) ||
// 				    (it->first.compare("TEXCOORD_0") == 0))
// 				{
// 					if (gGLProgramState.attribs[it->first] >= 0)
// 					{
// 						glDisableVertexAttribArray(gGLProgramState.attribs[it->first]);
// 					}
// 				}
// 			}
// 		}
// 	}
// }

// void drawModel2(const tinygltf::Model& model)
// {
// 	// TODO: Separate lists for separate primitives
// 	for (const tinygltf::Mesh& mesh : model.meshes)
// 	{
// 		DrawMesh(model, mesh);
// 	}
// }

// void initModel(const tinygltf::Model& model)
// {
// 	GLuint progId = 0;
// 	glUseProgram(progId);
// 	CheckGLErrors("useProgram");
// 	SetupMeshState(model, progId);
// 	CheckGLErrors("SetupGLState");
// }
