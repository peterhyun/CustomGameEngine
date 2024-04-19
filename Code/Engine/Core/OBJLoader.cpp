#include "Engine/Core/OBJLoader.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"

void OBJLoader::ParseFile(const std::string& filePath, const Mat44& transform, std::vector<Vertex_PCUTBN>& out_vertices, std::vector<unsigned int>& out_indices, OBJLoaderMetaData* out_metaData)
{
	std::string fileString;
	FileReadToString(fileString, filePath);
	std::istringstream iss(fileString);
	std::string line;

	std::vector<Vec3> vertexPosList;
	std::vector<Vec2> vertexUVList;
	std::vector<Vec3> vertexNormalList;

	bool hasNoFaces = true;
	bool hasNoNormals = true;
	int numFacesCounter = 0;
	int numTrianglesCounter = 0;

	double parseStartTime = GetCurrentTimeSeconds();

	while (std::getline(iss, line)) {
		std::string firstWord;
		std::istringstream lineSS(line);
		lineSS >> firstWord;
		if (firstWord == "v") {
			Vec3 vertexPos;
			lineSS >> vertexPos.x >> vertexPos.y >> vertexPos.z;
			Vec3 transformedPos = transform.TransformPosition3D(vertexPos);
			vertexPosList.push_back(transformedPos);
		}
		else if (firstWord == "vt") {
			Vec2 vertexUV;
			lineSS >> vertexUV.x >> vertexUV.y;
			vertexUVList.push_back(vertexUV);
		}
		else if (firstWord == "vn") {
			if (hasNoNormals)
				hasNoNormals = false;

			Vec3 vertexNormal;
			lineSS >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;

			Vec3 iBasis = transform.GetIBasis3D();
			Vec3 jBasis = transform.GetJBasis3D();
			Vec3 kBasis = transform.GetKBasis3D();
			iBasis.Normalize();
			jBasis.Normalize();
			kBasis.Normalize();

			Mat44 orthogonalTransform(iBasis, jBasis, kBasis, Vec3(0.0f, 0.0f, 0.0f));

			Vec3 transformedNormal = orthogonalTransform.TransformVectorQuantity3D(vertexNormal);	//This code assumes the transform matrix only has no non-uniform scaling!
			vertexNormalList.push_back(transformedNormal);
		}
		else if (firstWord == "f") {
			if (hasNoFaces)
				hasNoFaces = false;

			numFacesCounter++;
			//The dumb way...
			numTrianglesCounter++;

			std::vector<Vertex_PCUTBN> vertexCandidatesForThisFace;
			std::vector<unsigned int> indexCandidatesForThisFace;	//We have to store it separately just in case the face is an ngon

			std::string indicesChunk;
			while (lineSS >> indicesChunk) {
				Strings splitIndexStrings = SplitStringOnDelimeter(indicesChunk, '/');
				int posIndex = -1, uvIndex = -1, normalIndex = -1;
				if (splitIndexStrings.size() > 0)
					posIndex = atoi(splitIndexStrings[0].c_str()) - 1;
				if (splitIndexStrings.size() > 1)
					uvIndex = atoi(splitIndexStrings[1].c_str()) - 1;
				if (splitIndexStrings.size() > 2)
					normalIndex = atoi(splitIndexStrings[2].c_str()) - 1;

				Vec3 vertexPos = (posIndex >= 0) ? vertexPosList[posIndex] : Vec3(0.0f, 0.0f, 0.0f);
				Vec2 vertexUV = (uvIndex >= 0) ? vertexUVList[uvIndex] : Vec2(0.0f, 0.0f);
				Vec3 vertexNormal = (normalIndex >= 0) ? vertexNormalList[normalIndex] : Vec3(1.0f, 0.0f, 0.0f);

				indexCandidatesForThisFace.push_back(unsigned int(out_vertices.size() + vertexCandidatesForThisFace.size()));
				vertexCandidatesForThisFace.push_back(Vertex_PCUTBN(vertexPos, vertexNormal, Rgba8::WHITE, vertexUV));
			}

			//Dealing with ngons
			if (vertexCandidatesForThisFace.size() > 3) {
				for (int indexOffset = 1; indexOffset < vertexCandidatesForThisFace.size() - 1; indexOffset++) {
					out_indices.push_back((unsigned int)out_vertices.size());
					out_vertices.push_back(vertexCandidatesForThisFace[0]);
					
					out_indices.push_back((unsigned int)out_vertices.size());
					out_vertices.push_back(vertexCandidatesForThisFace[indexOffset]);

					out_indices.push_back((unsigned int)out_vertices.size());
					out_vertices.push_back(vertexCandidatesForThisFace[indexOffset + 1]);

					numTrianglesCounter++;
				}
				numTrianglesCounter--;
			}
			else {
				out_indices.insert(out_indices.end(), indexCandidatesForThisFace.begin(), indexCandidatesForThisFace.end());
				out_vertices.insert(out_vertices.end(), vertexCandidatesForThisFace.begin(), vertexCandidatesForThisFace.end());
			}

			if (out_metaData) {
				out_metaData->m_numFaces++;
			}
		}
	}

	if (hasNoFaces) {
		//Fill in out_vertices and out_indices based on vertexPosList
		for (int i = 0; i < vertexPosList.size() ; i++) {
			out_indices.push_back((unsigned int)out_vertices.size());
			out_vertices.push_back(Vertex_PCUTBN(vertexPosList[i], Vec3(1.0f, 0.0f, 0.0f), Rgba8::WHITE, Vec2(0.0f, 0.0f)));
		}
	}

	if (hasNoNormals) {
		for (int i = 0; i < out_vertices.size(); i+=3) {
			const Vec3& triangleFirstPos =  out_vertices[i].m_position;
			const Vec3& triangleSecondPos = out_vertices[i + 1].m_position;
			const Vec3& triangleThirdPos =  out_vertices[i + 2].m_position;

			const Vec3 firstToSecond = triangleSecondPos - triangleFirstPos;
			const Vec3 secondToThird = triangleThirdPos - triangleSecondPos;

			Vec3 normal = CrossProduct3D(firstToSecond, secondToThird).GetNormalized();

			out_vertices[i].m_normal = normal;
			out_vertices[i + 1].m_normal = normal;
			out_vertices[i + 2].m_normal = normal;
		}
	}

	double parseEndTime = GetCurrentTimeSeconds();

	if (out_metaData) {
		out_metaData->m_numPositions = (int)vertexPosList.size();
		out_metaData->m_numUVs = (int)vertexUVList.size();
		out_metaData->m_numNormals = (int)vertexNormalList.size();
		out_metaData->m_numFaces = numFacesCounter;
		out_metaData->m_numTriangles = numTrianglesCounter;
		out_metaData->m_numVertices = (int)out_vertices.size();
		out_metaData->m_numIndices = (int)out_indices.size();
		out_metaData->m_totalParseAndLoadTime = float(parseEndTime - parseStartTime);
	}
}
