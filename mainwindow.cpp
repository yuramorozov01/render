#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->initBuffer();

    this->loadModel();

    this->setFixedSize(this->currWidth, this->currHeight);

    this->baseTransform();
    this->calcNewTransformMatrix();

    this->initBarrier();

    this->initWorkers();

    this->updateTransformMatrix();

    this->startWorkers();
    this->startMoveTimer();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::loadModel() {
    this->model = new Model();
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Merged_Demon9Pose1.obj");
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/Drachen_1.0_obj.obj");
    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head.obj");
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/SlayerToy.obj");
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/ogloc/source/ogloc/ogloc.obj");
    this->vertices = this->model->getVertices();
    this->uvs = this->model->getUvs();
    this->normals = this->model->getNormals();
    this->isLoaded = res;

//    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/textures/dragon_C.jpg");
    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head_diffuse.jpg");
//    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/ogloc/textures/ogloc.png");
    this->diffuseMap = this->model->getDiffuseMap();
    this->isLoadedDiffuseMap = res;
}

void MainWindow::initBuffer() {
    this->bufferSize = 4 * this->currWidth * this->currHeight;
    this->buffer = new unsigned char[this->bufferSize];
    std::fill_n(this->buffer, this->bufferSize, 0);
    this->fastBufferToDraw = new unsigned char[this->bufferSize];
    std::copy_n(this->buffer, this->bufferSize, this->fastBufferToDraw);

    this->bufferToDraw = QImage(this->fastBufferToDraw, this->currWidth, this->currHeight, QImage::Format_RGB32);

    this->zbuffer = new std::vector<float>(this->bufferSize / 4, -std::numeric_limits<float>::max());
}

void MainWindow::initBarrier() {
    this->barrier = Barrier(this->amountOfWorkers + 1);
    connect(this->barrier.getBarrierData()->data(), SIGNAL(passed()), this, SLOT(passed()));
}

void MainWindow::initWorkers() {
    unsigned int rangeLen = (unsigned int) ceil(this->vertices->size() / this->amountOfWorkers);
    while (rangeLen % 3 != 0) {
        rangeLen++;
    }
    for (unsigned int i = 0; i < this->amountOfWorkers; i++) {
        WorkerThread *workerThread = new WorkerThread(this, i, rangeLen, this->buffer, this->model, this->barrier, this->zbuffer);
        this->workers.push_back(workerThread);
        connect(workerThread, SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));
        connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
        workerThread->setWidth(this->currWidth);
        workerThread->setHeight(this->currHeight);
        workerThread->setDepth(this->currDepth);
        workerThread->setVertices(this->vertices);
        workerThread->setUvs(this->uvs);
        workerThread->setNormals(this->normals);
        workerThread->setDiffuseMap(this->diffuseMap);
        workerThread->setBaseTransformMatrix(this->transformMatrix);
    }
}

void MainWindow::startWorkers() {
    for (unsigned int i = 0; i < this->amountOfWorkers; i++) {
        this->workers[i]->setEnabled(true);
        this->workers[i]->start();
    }
}

void MainWindow::startMoveTimer() {
    this->moveTimer = new QTimer();
    connect(this->moveTimer, SIGNAL(timeout()), this, SLOT(processTimer()));
    this->moveTimer->start(this->timerTimeout);
}

void MainWindow::baseTransform() {
    this->transformBuffer.push_back(this->model->viewport(0.f, 0.f, 0.f, this->currWidth, this->currHeight, this->currDepth));
    this->transformBuffer.push_back(this->model->projection(45.f, 0.1f, this->currDepth));
    this->transformBuffer.push_back(this->model->view(this->eye, this->target, this->up));
}

void MainWindow::calcNewTransformMatrix() {
    while (not this->transformBuffer.empty()) {
        this->transformMatrix *= this->transformBuffer.front();
        this->transformBuffer.pop_front();
    }
}

void MainWindow::updateTransformMatrix() {
    for (unsigned int i = 0; i < this->workers.size(); i++) {
        this->workers[i]->setTransformMatrix(this->transformMatrix);
    }
}

void MainWindow::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawPixmap(0, 0, QPixmap::fromImage(this->bufferToDraw));
    p.end();
}

void MainWindow::resetTransformMatrix() {
    this->transformMatrix = QMatrix4x4(1, 0, 0, 0,
                                       0, 1, 0, 0,
                                       0, 0, 1, 0,
                                       0, 0, 0, 1);
}

void MainWindow::makeMove() {
    /*
     * TODO:
     * change deltatime
    */

    float cameraSpeed = 0.05f * this->deltaTime;
    if (this->pressedKeys[Qt::Key_W]) {
        this->transformBuffer.push_back(this->model->translate(0, 1 * cameraSpeed, 0));
    }

    if (this->pressedKeys[Qt::Key_S]) {
        this->transformBuffer.push_back(this->model->translate(0, -1 * cameraSpeed, 0));
    }

    if (this->pressedKeys[Qt::Key_A]) {
        this->transformBuffer.push_back(this->model->translate(-1 * cameraSpeed, 0, 0));
    }

    if (this->pressedKeys[Qt::Key_D]) {
        this->transformBuffer.push_back(this->model->translate(1 * cameraSpeed, 0, 0));
    }

    if (this->pressedKeys[Qt::Key_R]) {
        this->transformBuffer.push_back(this->model->translate(0, 0, 1 * cameraSpeed));
    }

    if (this->pressedKeys[Qt::Key_T]) {
        this->transformBuffer.push_back(this->model->translate(0, 0, -1 * cameraSpeed));
    }

    if (this->pressedKeys[Qt::Key_Up]) {
        this->transformBuffer.push_back(this->model->rotateX(-1));
    }

    if (this->pressedKeys[Qt::Key_Down]) {
        this->transformBuffer.push_back(this->model->rotateX(1));
    }

    if (this->pressedKeys[Qt::Key_Left]) {
        this->transformBuffer.push_back(this->model->rotateY(1));
    }

    if (this->pressedKeys[Qt::Key_Right]) {
        this->transformBuffer.push_back(this->model->rotateY(-1));
    }

    if (this->pressedKeys[Qt::Key_N]) {
        this->transformBuffer.push_back(this->model->rotateZ(1));
    }

    if (this->pressedKeys[Qt::Key_M]) {
        this->transformBuffer.push_back(this->model->rotateZ(-1));
    }

    if (this->pressedKeys[Qt::Key_Minus]) {
        this->transformBuffer.push_back(this->model->scale(0.99f, 0.99f, 0.99f));
    }

    if (this->pressedKeys[Qt::Key_Equal]) {
        this->transformBuffer.push_back(this->model->scale(1.01f, 1.01f, 1.01f));
    }
}

void MainWindow::processTimer() {
    this->calcNewTransformMatrix();
    this->makeMove();
    this->update();
}

void MainWindow::updateModel() {
    std::copy_n(this->buffer, this->bufferSize, this->fastBufferToDraw);
    std::fill_n(this->buffer, this->bufferSize, 0);
    std::fill(this->zbuffer->begin(), this->zbuffer->end(), -std::numeric_limits<float>::max());
}

void MainWindow::handleResults(QString res) {
    qDebug() << res;
}

void MainWindow::passed() {
    this->updateTransformMatrix();
    this->updateModel();
    this->barrier.wait();
}

void MainWindow::keyPressEvent(QKeyEvent *keyEvent) {
    int key = keyEvent->key();
    this->pressedKeys[key] = true;
}

void MainWindow::keyReleaseEvent(QKeyEvent *keyEvent) {
    int key = keyEvent->key();
    this->pressedKeys[key] = false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{

}
