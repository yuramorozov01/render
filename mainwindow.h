#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <deque>
#include <math.h>
#include <algorithm>

#include <model.h>
#include <workerthread.h>
#include <barrier.h>

#include <QMainWindow>

#include <QtMath>

#include <QPointF>

#include <QThread>
#include <QGraphicsScene>

#include <QKeyEvent>

#include <QTimer>
#include <QTime>

#include <QCursor>

#include <QImage>
#include <QPainter>

#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTimer *moveTimer;

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;

    Model *model;
    bool isLoaded = false;
    bool isLoadedDiffuseMap = false;
    bool isLoadedNormalMap = false;
    bool isLoadedMirrorMap = false;

    std::vector<QVector3D> *vertices;
    std::vector<QVector3D> *uvs;
    std::vector<QVector3D> *normals;

    int timerTimeout = 1;

    float deltaTime = 1.0f;
    float lastFrame = 0.0f;

    std::map<int, bool> pressedKeys;

    QVector3D eye = QVector3D(0.f, 0.f, 100.f);
    QVector3D target = QVector3D(0.f, 0.f, 0.f);
    QVector3D up = QVector3D(0.f, 1.f, 0.f);

    QMatrix4x4 transformMatrix = QMatrix4x4(1, 0, 0, 0,
                                            0, 1, 0, 0,
                                            0, 0, 1, 0,
                                            0, 0, 0, 1);

    std::deque<QMatrix4x4> transformBuffer;

    unsigned long bufferSize = 0;
    unsigned char *buffer;
    unsigned char *fastBufferToDraw;
    QImage bufferToDraw;

    QImage *diffuseMap;
    QImage *normalMap;
    QImage *mirrorMap;

    std::vector<float> *zbuffer;

    int currWidth = 1300;
    int currHeight = 900;
    int currDepth = 250;

    unsigned int amountOfWorkers = 50;
    std::vector<WorkerThread*> workers;
    Barrier barrier = Barrier(0);

    void loadModel();

    void initBuffer();
    void initBarrier();
    void initWorkers();

    void startWorkers();
    void startMoveTimer();

    void baseTransform();

    void resetTransformMatrix();
    void updateTransformMatrix();
    void calcNewTransformMatrix();

    void makeMove();

private slots:
    void processTimer();
    void updateModel();
    void handleResults(QString res);
    void passed();

};
#endif // MAINWINDOW_H
