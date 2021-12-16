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
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/Drachen_1.0_obj.obj");
//    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head.obj");
    bool res = this->model->load("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Models/Diablo/Model.obj");
    this->vertices = this->model->getVertices();
    this->uvs = this->model->getUvs();
    this->normals = this->model->getNormals();
    this->isLoaded = res;

//    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/textures/dragon_C.jpg");
//    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head_diffuse.tga");
    res = this->model->loadDiffuseMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Models/Diablo/Albedo Map.png");
    this->diffuseMap = this->model->getDiffuseMap();
    this->isLoadedDiffuseMap = res;

//    res = this->model->loadNormalMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/textures/dragon_N.jpg");
//    res = this->model->loadNormalMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head_nm.tga");
    res = this->model->loadNormalMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Models/Diablo/Normal Map.png");
    this->normalMap = this->model->getNormalMap();
    this->isLoadedNormalMap = res;

//    res = this->model->loadMirrorMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/uploads-files-133955-Drachen_1/textures/dragon_S.jpg");
//    res = this->model->loadMirrorMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/head/head_spec.tga");
    res = this->model->loadMirrorMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Models/Diablo/Specular Map.png");
    this->mirrorMap = this->model->getMirrorMap();
    this->isLoadedMirrorMap = res;

    res = this->model->loadEmissiveMap("/Users/yura-pc/Files/Study/4курс/1сем/АКГ/лабы/obj3d/obj/Models/Diablo/Emissive Map.png");
    this->emissiveMap = this->model->getEmissiveMap();
    this->lightMap = new bool[this->bufferSize / 4];
    std::fill_n(this->lightMap, this->bufferSize / 4, false);
    this->lightMapCached = new bool[this->bufferSize / 4];
    std::fill_n(this->lightMapCached, this->bufferSize / 4, false);
    this->isLoadedEmissiveMap = res;
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
        workerThread->setNormalMap(this->normalMap);
        workerThread->setMirrorMap(this->mirrorMap);
        workerThread->setEmissiveMap(this->emissiveMap, this->lightMap);
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
    if (this->isBloom) {
        this->makeBufferBloomed();
        p.drawPixmap(0, 0, QPixmap::fromImage(this->bloomedBuffer));
    } else {
        p.drawPixmap(0, 0, QPixmap::fromImage(this->bufferToDraw));
    }
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
    std::copy_n(this->lightMap, this->bufferSize / 4, this->lightMapCached);
    std::fill_n(this->lightMap, this->bufferSize / 4, false);
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

void MainWindow::makeBufferBloomed() {
    int blurRadius = 10;
    int brightness = 100;
    int opacity = 192;
    QPainter::CompositionMode mode = QPainter::CompositionMode_Plus;

    QImage brightenedImage = this->brightImage(this->bufferToDraw, brightness);
    QImage blurredImage = this->blurImage(brightenedImage, brightenedImage.rect(), blurRadius);
    this->bloomedBuffer = this->combineImages(this->bufferToDraw, blurredImage, opacity, mode);
//    this->bloomedBuffer = blurredImage;

}

QImage MainWindow::blurImage(const QImage& image, const QRect& rect, int radius) {
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image;
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = 0; i < 3; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = 0; i < 3; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }
    return result;
}

QImage MainWindow::brightImage(const QImage &image, int brightness) {

    QImage brightenedImage = image.copy();
    brightenedImage.fill(0);
    for (int i = 0; i < this->currHeight; i++) {
        for (int j = 0; j < this->currWidth; j ++) {
            if (this->lightMapCached[i * this->currWidth + j]) {
                brightenedImage.setPixel(j, i, image.pixel(j, i));
            }
        }
    }

    int tab[256];
    for (int i = 0; i < 256; ++i)
        tab[i] = qMin(i + brightness, 255);

    for (int y = 0; y < brightenedImage.height(); y++) {
        QRgb* line = (QRgb*)(brightenedImage.scanLine(y));
        for (int x = 0; x < brightenedImage.width(); x++)
            if (this->lightMapCached[y * this->currWidth + x]) {
                line[x] = qRgb(tab[qRed(line[x])], tab[qGreen(line[x])], tab[qBlue(line[x])]);
            }
    }
    return brightenedImage;
}

QImage MainWindow::combineImages(const QImage &img1, const QImage &img2, int opacity, QPainter::CompositionMode mode) {
    QImage result = img1.copy();
    QPainter painter(&result);
    painter.setCompositionMode(mode);
//    painter.setOpacity((qreal)(opacity) / 256.0);
    painter.drawImage(0, 0, img2);
    painter.end();
    return result;
}

