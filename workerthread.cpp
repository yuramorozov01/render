#include "workerthread.h"

WorkerThread::WorkerThread(QObject * parent, unsigned int threadNumber, unsigned int range, unsigned char *buf, Model *model, Barrier barrier, std::vector<float> *zbuffer):
    QThread(parent), barrier(barrier)
{
    this->threadNumber = threadNumber;
    this->range = range;
    this->buffer = buf;
    this->model = model;
    this->zbuffer = zbuffer;
}

void WorkerThread::setEnabled(bool flag) {
    this->isEnabled = flag;
}

void WorkerThread::setBuffer(unsigned char *buf) {
    this->buffer = buf;
}

void WorkerThread::setZbuffer(std::vector<float> *zbuffer) {
    this->zbuffer = zbuffer;
}

void WorkerThread::setWidth(int width) {
    this->currWidth = width;
}

void WorkerThread::setHeight(int height) {
    this->currHeight = height;
}

void WorkerThread::setDepth(int depth) {
    this->currDepth = depth;
}

void WorkerThread::setVertices(std::vector<QVector3D> *vertices) {
    this->vertices = vertices;
}

void WorkerThread::setUvs(std::vector<QVector3D> *uvs) {
    this->uvs = uvs;
}

void WorkerThread::setNormals(std::vector<QVector3D> *normals) {
    this->normals = normals;
}

void WorkerThread::setDiffuseMap(QImage *diffuseMap) {
    this->diffuseMap = diffuseMap;
}

void WorkerThread::setBaseTransformMatrix(QMatrix4x4 baseTransformMatrix) {
    this->baseTransformMatrix = baseTransformMatrix;
}

void WorkerThread::setTransformMatrix(QMatrix4x4 transformMatrix) {
    this->transformMatrix = transformMatrix;
}

void WorkerThread::run() {
    QString result;

    QVector4D temp_vect_4d;
    QVector3D temp_vect_3d, temp_vect_3d_addit;

    std::vector<QVector3D> *rawPoints = new std::vector<QVector3D>(3);

//    float *points = new float[9];
//    float *uvPoints = new float[9];

    std::vector<QVector3D> *points = new std::vector<QVector3D>(3);
    std::vector<QVector3D> *uvPoints = new std::vector<QVector3D>(3);

    QVector3D light = QVector3D(0, 1, 1);
    QVector3D eye = QVector3D(0, 0, 100);

    float intensity = 1.f;

    int temp_ind = 0;

    while (this->isEnabled) {
        unsigned int i = this->threadNumber * this->range;
        while (i < (this->threadNumber + 1) * this->range) {
            temp_ind = 0;
            rawPoints->clear();
            points->clear();
            uvPoints->clear();
            for (unsigned int j = i; j < i + 3; j++) {
                // Apply transform matrix to the source vector
                // And convert it to 3d vector
                temp_vect_4d = this->vertices->at(j).toVector4D();
                temp_vect_4d.setW(1);
                temp_vect_4d = this->transformMatrix * temp_vect_4d;
                temp_vect_3d = this->model->vector4Dto3D(temp_vect_4d);

                // Get vector in raw format
                temp_vect_3d_addit = this->model->vector4Dto3D(this->baseTransformMatrix.inverted() * temp_vect_4d);
                rawPoints->push_back(temp_vect_3d_addit);

                // Get raw z coordinate (effecient work with zbuffer)
                temp_vect_3d.setZ(temp_vect_3d_addit.z());
                points->push_back(temp_vect_3d);

                // Get and scale texture vectors
                temp_vect_3d = this->uvs->at(j);
                temp_vect_3d.setX(temp_vect_3d.x() * this->diffuseMap->width());
                temp_vect_3d.setY(temp_vect_3d.y() * this->diffuseMap->height());
                temp_vect_3d.setZ(temp_vect_3d.z() * 1000);
                uvPoints->push_back(temp_vect_3d);

                temp_ind += 3;
            }
            intensity = this->calcIntensity(rawPoints, light, eye);
            if (intensity >= 0.f) {
                this->drawTriangle(points, uvPoints, intensity);
            }
            i += 3;
            if (i >= this->vertices->size()) {
                break;
            }
        }
        this->barrier.wait();
    }
    delete [] points;
    delete [] uvPoints;
    emit this->resultReady(result);
}

void WorkerThread::setPixel(QVector3D point, QVector3D uvPoint, float intensity, int color) {
    if (point.x() <= 0 || uvPoint.x() <= 0) {
        return;
    }
    if (point.x() >= this->currWidth || uvPoint.x() >= this->diffuseMap->width()) {
        return;
    }
     if (point.y() <= 0 || uvPoint.y() <= 0) {
        return;
    }
    if (point.y() >= this->currHeight || uvPoint.y() >= this->diffuseMap->height()) {
        return;
    }

    int x = point.x();
    int y = point.y();
    float z = point.z();
    int uv_x = uvPoint.x();
    int uv_y = uvPoint.y();

    int index = y * this->currWidth + x;
    if (index >= 0  && index < this->currHeight * this->currWidth) {
        if (z > this->zbuffer->at(index)) {
            this->zbuffer->at(index) = z;
            QColor pixelColor = this->diffuseMap->pixelColor(uv_x, uv_y);
            this->buffer[4 * index + 3] = -1;
            this->buffer[4 * index + 2] = intensity * pixelColor.red();
            this->buffer[4 * index + 1] = intensity * pixelColor.green();
            this->buffer[4 * index + 0] = intensity * pixelColor.blue();
        }
    }
}

float WorkerThread::calcIntensity(std::vector<QVector3D> *points, QVector3D light, QVector3D eye) {
    QVector3D norm = this->model->cross(points->at(1) - points->at(0), points->at(2) - points->at(0)).normalized();
    if (QVector3D::dotProduct(norm, eye) < 0) {
        return -1;
    }
    float intensity = QVector3D::dotProduct(norm, light.normalized());
    return intensity < 0.f ? 0.f : intensity;
}


void WorkerThread::drawTriangle(std::vector<QVector3D> *points, std::vector<QVector3D> *uvPoints, float intensity) {
    for (int i = 0; i < points->size(); i++) {
        points->at(i).setX(int(points->at(i).x()));
        points->at(i).setY(int(points->at(i).y()));
    }

    QVector3D point0 = points->at(0);
    QVector3D point1 = points->at(1);
    QVector3D point2 = points->at(2);

    QVector3D uvPoint0 = uvPoints->at(0);
    QVector3D uvPoint1 = uvPoints->at(1);
    QVector3D uvPoint2 = uvPoints->at(2);

    if (point0.y() > point1.y()) {
        std::swap(point0, point1);
        std::swap(uvPoint0, uvPoint1);
    }
    if (point0.y() > point2.y()) {
        std::swap(point0, point2);
        std::swap(uvPoint0, uvPoint2);
    }
    if (point1.y() > point2.y()) {
        std::swap(point1, point2);
        std::swap(uvPoint1, uvPoint2);
    }

    int totalHeight = 1;
    if (int(point2.y()) != int(point0.y())) {
        totalHeight = point2.y() - point0.y();
    }

    for (int i = 0; i < totalHeight; i++) {
        bool secondHalf = (i > point1.y() - point0.y()) || (int(point1.y()) == int(point0.y()));
        int segmentHeight = secondHalf ? point2.y() - point1.y() : point1.y() - point0.y();
        if (segmentHeight == 0) {
            segmentHeight = 1;
        }
        float alpha = float(i) / totalHeight;
        float beta = float(i - (secondHalf ? point1.y() - point0.y() : 0)) / segmentHeight;
        QVector3D A = point0 + (point2 - point0) * alpha;
        QVector3D B = secondHalf ? point1 + (point2 - point1) * beta : point0 + (point1 - point0) * beta;

        QVector3D uv_A = uvPoint0 + (uvPoint2 - uvPoint0) * alpha;
        QVector3D uv_B = secondHalf ? uvPoint1 + (uvPoint2 - uvPoint1) * beta : uvPoint0 + (uvPoint1 - uvPoint0) * beta;

        if (A.x() > B.x()) {
            std::swap(A, B);
            std::swap(uv_A, uv_B);
        }

        for (int j = A.x(); j <= B.x(); j++) {
            float phi = int(B.x()) == int(A.x()) ? 1. : float(j - A.x()) / float(B.x() - A.x());
            QVector3D new_P = A + (B - A) * phi;
            QVector3D new_uv_P = uv_A + (uv_B - uv_A) * phi;
            this->setPixel(new_P, new_uv_P, intensity);
        }
    }
}

