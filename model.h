#ifndef MODEL_H
#define MODEL_H

#include <objreader.h>

#include <QtMath>

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

#include <QImage>

class Model {
public:
    Model();
    bool load(const char *path=nullptr);
    bool loadDiffuseMap(const char *path=nullptr);
    bool loadNormalMap(const char *path=nullptr);
    bool loadMirrorMap(const char *path=nullptr);
    bool loadEmissiveMap(const char *path=nullptr);

    std::vector<QVector3D>* getVertices();
    std::vector<QVector3D>* getUvs();
    std::vector<QVector3D>* getNormals();
    QImage *getDiffuseMap();
    QImage *getNormalMap();
    QImage *getMirrorMap();
    QImage *getEmissiveMap();

    void applyMatrix(QMatrix4x4 transform);

    QVector3D vector4Dto3D(QVector4D vector);
    QVector2D vector3Dto2D(QVector3D vector);
    QVector2D vector4Dto2D(QVector4D vector);

    QVector3D cross(QVector3D a, QVector3D b);

    QMatrix4x4 viewport(float xMin, float yMin, float zMin, float width, float height, float depth);
    QMatrix4x4 projection(float fov, float zNear, float zFar);
    QMatrix4x4 view(QVector3D eye, QVector3D target, QVector3D up);

    QMatrix4x4 translate(float translateX, float translateY, float translateZ);
    QMatrix4x4 scale(float scaleX, float scaleY, float scaleZ);
    QMatrix4x4 rotateX(float angle);
    QMatrix4x4 rotateY(float angle);
    QMatrix4x4 rotateZ(float angle);

private:
    std::vector<QVector3D> vertices;
    std::vector<QVector3D> uvs;
    std::vector<QVector3D> normals;

    QImage *diffuseMap;
    QImage *normalMap;
    QImage *mirrorMap;
    QImage *emissiveMap;

};

#endif // MODEL_H
