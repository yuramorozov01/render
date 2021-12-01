#include "model.h"

Model::Model() {

}

bool Model::load(const char *path) {
    bool res = false;
    if (path != nullptr) {
        res = ObjReader::loadOBJ(path, this->vertices, this->uvs, this->normals);
    }
    return res;
}

bool Model::loadDiffuseMap(const char *path) {
    bool res = false;

    if (path != nullptr) {
        QImage tmpDiffuseMap = QImage(path);
        this->diffuseMap = new QImage(tmpDiffuseMap.mirrored());
        res = true;
    }
    return res;
}

bool Model::loadNormalMap(const char *path) {
    bool res = false;

    if (path != nullptr) {
        QImage tmpNormalMap = QImage(path);
        this->normalMap = new QImage(tmpNormalMap.mirrored());
        res = true;
    }
    return res;
}

bool Model::loadMirrorMap(const char *path) {
    bool res = false;

    if (path != nullptr) {
        QImage tmpMirrorMap = QImage(path);
        this->mirrorMap = new QImage(tmpMirrorMap.mirrored());
        res = true;
    }
    return res;
}

std::vector<QVector3D>* Model::getVertices() {
    return &this->vertices;
}

std::vector<QVector3D>* Model::getUvs() {
    return &this->uvs;
}

std::vector<QVector3D>* Model::getNormals() {
    return &this->normals;
}

QImage* Model::getDiffuseMap() {
    return this->diffuseMap;
}

QImage* Model::getNormalMap() {
    return this->normalMap;
}

QImage* Model::getMirrorMap() {
    return this->mirrorMap;
}

void Model::applyMatrix(QMatrix4x4 transform) {
    QVector4D temp_vec;
    for (int i = 0; i < this->vertices.size(); i++) {
        temp_vec = this->vertices[i].toVector4D();
        temp_vec.setW(1);
        this->vertices[i] = this->vector4Dto3D(transform * temp_vec);
    }
}

QVector3D Model::vector4Dto3D(QVector4D vector) {
    QVector3D newVector = QVector3D(vector[0] / vector[3], vector[1] / vector[3], vector[2] / vector[3]);
    return newVector;
}

QVector2D Model::vector3Dto2D(QVector3D vector) {
    QVector2D newVector = QVector2D(vector[0] / vector[2], vector[1] / vector[2]);
    return newVector;
}

QVector2D Model::vector4Dto2D(QVector4D vector) {
    QVector3D newVector3D = this->vector4Dto3D(vector);
    QVector2D newVector2D = this->vector3Dto2D(newVector3D);
    return newVector2D;
}

QVector3D Model::cross(QVector3D a, QVector3D b) {
    return QVector3D(a.y() * b.z() - a.z() * b.y(), - (a.x() * b.z() - a.z() * b.x()), a.x() * b.y() - a.y() * b.x());
}

QMatrix4x4 Model::viewport(float xMin, float yMin, float zMin, float width, float height, float depth) {
    QMatrix4x4 matrix = QMatrix4x4(
                width / 2.f, 0, 0, xMin + width / 2.f,
                0, - height / 2.f, 0, yMin + height / 2.f,
                0, 0, 1, 0,
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::projection(float fov, float zNear, float zFar) {
    float radians = qDegreesToRadians(fov / 2);
    QMatrix4x4 matrix = QMatrix4x4(
                1 / ((1300.f / 900.f) * qTan(radians)), 0, 0, 0,
                0, 1 / qTan(radians), 0, 0,
                0, 0, zFar / (zNear - zFar), (zNear * zFar) / (zNear - zFar),
                0, 0, -1, 0
                );
    return matrix;
}

QMatrix4x4 Model::view(QVector3D eye, QVector3D target, QVector3D up) {
    QVector3D zAxis = (eye - target).normalized();
    QVector3D xAxis = cross(up, zAxis).normalized();
    QVector3D yAxis = up;
    QMatrix4x4 matrix = QMatrix4x4(
                xAxis.x(), xAxis.y(), xAxis.z(), -(xAxis.x() * eye.x()),
                yAxis.x(), yAxis.y(), yAxis.z(), -(yAxis.y() * eye.y()),
                zAxis.x(), zAxis.y(), zAxis.z(), -(zAxis.z() * eye.z()),
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::translate(float translateX, float translateY, float translateZ) {
    QMatrix4x4 matrix = QMatrix4x4(
                1, 0, 0, translateX,
                0, 1, 0, translateY,
                0, 0, 1, translateZ,
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::scale(float scaleX, float scaleY, float scaleZ) {
    QMatrix4x4 matrix = QMatrix4x4(
                scaleX, 0, 0, 0,
                0, scaleY, 0, 0,
                0, 0, scaleZ, 0,
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::rotateX(float angle) {
    float radians = qDegreesToRadians(angle);
    QMatrix4x4 matrix = QMatrix4x4(
                1, 0, 0, 0,
                0, qCos(radians), -qSin(radians), 0,
                0, qSin(radians), qCos(radians), 0,
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::rotateY(float angle) {
    float radians = qDegreesToRadians(angle);
    QMatrix4x4 matrix = QMatrix4x4(
                qCos(radians), 0, qSin(radians), 0,
                0, 1, 0, 0,
                -qSin(radians), 0, qCos(radians), 0,
                0, 0, 0, 1
                );
    return matrix;
}

QMatrix4x4 Model::rotateZ(float angle) {
    float radians = qDegreesToRadians(angle);
    QMatrix4x4 matrix = QMatrix4x4(
                qCos(radians), -qSin(radians), 0, 0,
                qSin(radians), qCos(radians), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
                );
    return matrix;
}
