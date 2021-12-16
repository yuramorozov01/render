#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <iostream>

#include <QMainWindow>

#include <QThread>

#include <QVector2D>
#include <QVector3D>

#include <QMatrix4x4>

#include <QImage>
#include <QColor>

#include <model.h>
#include <barrier.h>

class WorkerThread: public QThread
{
    Q_OBJECT
public:
    WorkerThread(QObject *parent, unsigned int threadNumber, unsigned int range, unsigned char *buf, Model *model, Barrier barrier, std::vector<float> *zbuffer);

    void setEnabled(bool flag);

    void run() override;

    void setBuffer(unsigned char *buf);
    void setZbuffer(std::vector<float> *zbuffer);

    void setWidth(int width);
    void setHeight(int height);
    void setDepth(int depth);

    void setVertices(std::vector<QVector3D> *vertices);
    void setUvs(std::vector<QVector3D> *uvs);
    void setNormals(std::vector<QVector3D> *normals);
    void setDiffuseMap(QImage *diffuseMap);
    void setNormalMap(QImage *normalMap);
    void setMirrorMap(QImage *mirrorMap);
    void setEmissiveMap(QImage *emissiveMap, bool *lightMap);

    void setBaseTransformMatrix(QMatrix4x4 baseTransformMatrix);
    void setTransformMatrix(QMatrix4x4 transformMatrix);

private:
    QMainWindow *parentWindow;

    bool isEnabled = false;
    unsigned int threadNumber;
    unsigned int range;

    Model *model;

    QVector3D light = QVector3D(0.f, 1.f, 1.f);
    QVector3D eye = QVector3D(0.f, 0.f, 10.f);

    std::vector<QVector3D> *vertices;
    std::vector<QVector3D> *uvs;
    std::vector<QVector3D> *normals;
    QImage *diffuseMap;
    QImage *normalMap;
    QImage *mirrorMap;
    QImage *emissiveMap;
    bool *lightMap;

    QMatrix4x4 transformMatrix = QMatrix4x4(1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, 0,
                                            0, 0, 0, 1);

    QMatrix4x4 baseTransformMatrix = QMatrix4x4(1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, 0,
                                            0, 0, 0, 1);

    int currWidth = 1300;
    int currHeight = 900;
    int currDepth = 250;

    unsigned char *buffer;

    std::vector<float> *zbuffer;

    QVector3D getRawPoint(QVector3D point);

    void setPixel(QVector3D point, QVector3D uvPoint, QVector3D currNormal, int color=-1);
    void drawTriangle (std::vector<QVector3D> *points, std::vector<QVector3D> *uvPoints, std::vector<QVector3D> *currNormals);

    QVector3D calcTriangleNormal(std::vector<QVector3D> *points);

    bool isBackfaceCulling(std::vector<QVector3D> *points);

    float calcDiffuseLight(QVector3D point, QVector3D normal);
    QVector3D calcMirrorlight(QVector3D point, QVector3D normal, QVector3D mirrorCoeffFromMap);

    float calcBackgroundLight();
    float backgroundLight = 0.1f * 1;

    Barrier barrier;
signals:
    void resultReady(const QString &s);
};

#endif // WORKERTHREAD_H
