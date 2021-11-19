#ifndef OBJREADER_H
#define OBJREADER_H

#include <vector>
#include <QVector2D>
#include <QVector3D>

#include <iostream>



class ObjReader
{
public:
    ObjReader();
    static bool loadOBJ(
        const char * path,
        std::vector<QVector3D> & out_vertices,
        std::vector<QVector3D> & out_uvs,
        std::vector<QVector3D> & out_normals
    );
};

#endif // OBJREADER_H
