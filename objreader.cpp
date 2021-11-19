#include "objreader.h"


ObjReader::ObjReader()
{ }

bool ObjReader::loadOBJ(
            const char *path,
            std::vector<QVector3D> &out_vertices,
            std::vector<QVector3D> &out_uvs,
            std::vector<QVector3D> &out_normals
            )
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<QVector3D> temp_vertices;
    std::vector<QVector3D> temp_uvs;
    std::vector<QVector3D> temp_normals;

    FILE * file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file !\n");
        return false;
    }

    while(1) {
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;

        if (strcmp(lineHeader, "v") == 0)
        {
            QVector3D vertex;
            float x, y, z;
            fscanf(file, "%f %f %f\n", &x, &y, &z);
            vertex.setX(x);
            vertex.setY(y);
            vertex.setZ(z);

            temp_vertices.push_back(vertex);

        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            QVector3D uv;
            float x, y, z;
//            fscanf(file, "%f %f %f\n", &x, &y, &z);
            fscanf(file, "%f %f\n", &x, &y);
            uv.setX(x);
            uv.setY(y);
            uv.setZ(0);
            temp_uvs.push_back(uv);

        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            QVector3D normal;
            float x, y, z;
            fscanf(file, "%f %f %f\n", &x, &y, &z);
            normal.setX(x);
            normal.setY(y);
            normal.setZ(z);
            temp_normals.push_back(normal);

        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
            if (matches != 9)
            {
                printf("try another obj file\n");
                return false;
            }
//            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);

//            int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }

    for (unsigned int i = 0; i < vertexIndices.size(); i++)
    {
        unsigned int vertexIndex = vertexIndices[i];
        QVector3D vertex = temp_vertices[vertexIndex - 1];
        out_vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < uvIndices.size(); i++)
    {
        unsigned int uvIndex = uvIndices[i];
        QVector3D vertex = temp_uvs[uvIndex - 1];
        out_uvs.push_back(vertex);
    }

    for (unsigned int i = 0; i < normalIndices.size(); i++)
    {
        unsigned int normalIndex = normalIndices[i];
        QVector3D vertex = temp_normals[normalIndex-1];
        out_normals.push_back(vertex);
    }
    return true;
}
