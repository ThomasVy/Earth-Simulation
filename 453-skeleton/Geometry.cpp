#include "Geometry.h"

#include <utility>


GPU_Geometry::GPU_Geometry()
	: vao()
	, vertBuffer(0, 3, GL_FLOAT)
	, textCoordsBuffer(1, 2, GL_FLOAT)
	, normalsBuffer(2, 3, GL_FLOAT)
{}


void GPU_Geometry::setVerts(const std::vector<glm::vec3>& verts) {
	vertBuffer.uploadData(sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}


void GPU_Geometry::setNormals(const std::vector<glm::vec3>& norms) {
	normalsBuffer.uploadData(sizeof(glm::vec3) * norms.size(), norms.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setTextCoordinates(const std::vector<glm::vec2>& textCoords)
{
	textCoordsBuffer.uploadData(sizeof(glm::vec2) * textCoords.size(), textCoords.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setIndices(const std::vector<unsigned int>& indices)
{
	indexSize = indices.size();
	indexBuffer.uploadData(sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
}

int GPU_Geometry::getIndexSize()
{
	return indexSize;
}

