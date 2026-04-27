#include "MainWindow.hpp"
#include "SkinManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDir>
#include <QDebug>

void MainWindow::loadAlbumCover(const QString& audioPath) {
    if (audioPath.isEmpty()) return;
    
    m_currentFilePath = audioPath;
    
    QString basePath = audioPath.left(audioPath.lastIndexOf('/'));
    QString fileName = audioPath.mid(audioPath.lastIndexOf('/') + 1);
    QString baseName = fileName.section('.', 0, 0);
    
    QStringList imageExtensions = {"jpg", "jpeg", "png", "gif", "bmp", "webp"};
    QString coverPath;
    
    QStringList coverNames = {"cover", "folder", "album", "front", "000", "frontcover", "albumart"};
    
    for (const QString& ext : imageExtensions) {
        for (const QString& name : coverNames) {
            QString imgPath = basePath + "/" + name + "." + ext;
            if (QFile::exists(imgPath)) {
                coverPath = imgPath;
                break;
            }
        }
        if (!coverPath.isEmpty()) break;
        
        QString imgPath2 = basePath + "/" + baseName + "." + ext;
        if (QFile::exists(imgPath2)) {
            coverPath = imgPath2;
            break;
        }
    }
    
    if (coverPath.isEmpty() || !QFile::exists(coverPath)) {
        m_lblCover->setText("No Cover");
        m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; font-size: 10px; }");
    } else {
        QPixmap pixmap(coverPath);
        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_lblCover->setPixmap(pixmap);
            m_lblCover->setText("");
            m_lblCover->setStyleSheet("");
        } else {
            m_lblCover->setText("No Cover");
            m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; font-size: 10px; }");
        }
    }
}

MainWindow::MainWindow(AudioEngine* engine, QWidget *parent) 
    : QWidget(parent), m_engine(engine) 
{
    m_eqWin = std::make_unique<EqualizerWindow>(engine);
    m_visWin = std::make_unique<VisualizerWindow>(engine);
    m_skinWin = std::make_unique<SkinWindow>(this);

    setWindowTitle("Crystal Player - Retro Edition");
    resize(550, 300);
    setStyleSheet(SkinManager::getMainWindowStyle());

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    
    QVBoxLayout* leftLayout = new QVBoxLayout();
    
    m_lblInfo = new QLabel("CRYSTAL AUDIO v1.0", this);
    m_lblInfo->setObjectName("DisplayLabel");
    m_lblInfo->setAlignment(Qt::AlignCenter);
    m_lblInfo->setMinimumHeight(50);
    leftLayout->addWidget(m_lblInfo);

    // Botões
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnOpen = new QPushButton("OPEN", this);
    m_btnPlay = new QPushButton("PLAY", this);
    m_btnEQ = new QPushButton("EQ", this);
    m_btnVis = new QPushButton("VIS", this);
    m_btnSkin = new QPushButton("SKIN", this);

    btnLayout->addWidget(m_btnOpen);
    btnLayout->addWidget(m_btnPlay);
    btnLayout->addWidget(m_btnEQ);
    btnLayout->addWidget(m_btnVis);
    btnLayout->addWidget(m_btnSkin);
    leftLayout->addLayout(btnLayout);

    // Seek Bar
    m_sliderSeek = new QSlider(Qt::Horizontal, this);
    leftLayout->addWidget(m_sliderSeek);

    // Volume Layout
    QHBoxLayout* volLayout = new QHBoxLayout();
    QLabel* volLabel = new QLabel("Vol:", this);
    m_sliderVol = new QSlider(Qt::Horizontal, this);
    m_sliderVol->setRange(0, 100);
    m_sliderVol->setValue(100);
    volLayout->addWidget(volLabel);
    volLayout->addWidget(m_sliderVol);
    leftLayout->addLayout(volLayout);
    
    mainLayout->addLayout(leftLayout);
    
    // Album Cover
    m_lblCover = new QLabel(this);
    m_lblCover->setFixedSize(150, 150);
    m_lblCover->setAlignment(Qt::AlignCenter);
    m_lblCover->setText("No Cover");
    m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; border: 2px solid #444; font-size: 12px; }");
    mainLayout->addWidget(m_lblCover);

    // Connect signals
    connect(m_btnOpen, &QPushButton::clicked, this, &MainWindow::openFileDialog);
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    connect(m_btnEQ, &QPushButton::clicked, this, &MainWindow::toggleEQ);
    connect(m_btnVis, &QPushButton::clicked, this, &MainWindow::toggleVis);
    connect(m_btnSkin, &QPushButton::clicked, this, &MainWindow::toggleSkin);
    connect(m_skinWin.get(), &SkinWindow::themeApplied, this, [this](const QString&) {
        setStyleSheet(SkinManager::getMainWindowStyle());
        m_eqWin->setStyleSheet(SkinManager::getEqWindowStyle());
        m_visWin->setStyleSheet(SkinManager::getVisWindowStyle());
    });

    connect(m_sliderVol, &QSlider::valueChanged, this, [this](int value) {
        m_engine->setVolume(value / 100.0f);
    });

    connect(m_sliderSeek, &QSlider::sliderPressed, this, [this]() { m_isDraggingSeek = true; });
    connect(m_sliderSeek, &QSlider::sliderReleased, this, [this]() {
        m_isDraggingSeek = false;
        m_engine->setPosition((float)m_sliderSeek->value());
    });

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateUI);
    m_timer->start(50); // 20fps for UI updates
}

void MainWindow::openFileDialog() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Audio File", "", "Audio Files (*.mp3 *.wav *.flac *.mod *.xm *.s3m *.it);;All Files (*.*)");
    if (!fileName.isEmpty()) {
        if (m_engine->loadFile(fileName.toStdString())) {
            QFileInfo fileInfo(fileName);
            m_lblInfo->setText(fileInfo.fileName());
            m_sliderSeek->setRange(0, (int)m_engine->getDuration());
            m_engine->play();
            m_btnPlay->setText("PAUSE");
            loadAlbumCover(fileName);
        }
    }
}

void MainWindow::togglePlayPause() {
    if (m_engine->isPlaying()) {
        m_engine->pause();
        m_btnPlay->setText("PLAY");
    } else {
        m_engine->play();
        m_btnPlay->setText("PAUSE");
    }
}

void MainWindow::toggleEQ() {
    if (m_eqWin->isVisible()) {
        m_eqWin->hide();
    } else {
        m_eqWin->show();
    }
}

void MainWindow::toggleVis() {
    if (m_visWin->isVisible()) {
        m_visWin->hide();
    } else {
        m_visWin->show();
    }
}

void MainWindow::toggleSkin() {
    if (m_skinWin && m_skinWin->isVisible()) {
        m_skinWin->hide();
    } else {
        m_skinWin.reset();
        m_skinWin = std::make_unique<SkinWindow>(this);
        connect(m_skinWin.get(), &SkinWindow::themeApplied, this, [this](const QString&) {
            setStyleSheet(SkinManager::getMainWindowStyle());
            m_eqWin->setStyleSheet(SkinManager::getEqWindowStyle());
            m_visWin->setStyleSheet(SkinManager::getVisWindowStyle());
        });
        m_skinWin->show();
    }
}

void MainWindow::updateUI() {
    if (m_engine->isPlaying() && !m_isDraggingSeek) {
        m_sliderSeek->setValue((int)m_engine->getPosition());
    }
}
