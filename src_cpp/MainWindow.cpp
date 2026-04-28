#include "MainWindow.hpp"
#include "SkinManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMouseEvent>
#include <QDir>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

void MainWindow::searchAndDownloadCover(const QString& artist, const QString& album, const QString& basePath) {
    if (artist.isEmpty() && album.isEmpty()) return;

    QString query = "https://musicbrainz.org/ws/2/release/?query=";
    QString searchTerm;

    if (!album.isEmpty()) {
        searchTerm += "release:\"" + album + "\"";
    }
    if (!artist.isEmpty()) {
        if (!searchTerm.isEmpty()) searchTerm += "+AND+";
        searchTerm += "artist:\"" + artist + "\"";
    }

    query += searchTerm + "&fmt=json&limit=1";

    QUrl url(query);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "BrambleAudioPlayer/1.0 (contact@bramble.player)");

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, basePath, album, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return;

        QJsonObject root = doc.object();
        QJsonArray releases = root["releases"].toArray();
        if (releases.isEmpty()) return;

        QString releaseId = releases[0].toObject()["id"].toString();
        if (releaseId.isEmpty()) return;

        downloadCoverFromReleaseId(releaseId, basePath);
    });
}

void MainWindow::downloadCoverFromReleaseId(const QString& releaseId, const QString& basePath) {
    QString coverUrl = "https://coverartarchive.org/release/" + releaseId + "/front-500";

    QUrl url(coverUrl);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, "BrambleAudioPlayer/1.0 (contact@bramble.player)");

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, basePath, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        saveCoverToFile(reply, basePath);
    });
}

void MainWindow::saveCoverToFile(QNetworkReply* reply, const QString& basePath) {
    QByteArray imageData = reply->readAll();
    if (imageData.isEmpty()) return;

    QStringList extensions = {"jpg", "png"};
    QStringList names = {"cover", "folder", "album", "front", "000"};

    for (const QString& name : names) {
        for (const QString& ext : extensions) {
            QString filePath = basePath + "/" + name + "." + ext;
            QFile file(filePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(imageData);
                file.close();
                setCoverImage(filePath);
                return;
            }
        }
    }
}

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
        m_lblCover->setText("Downloading...");
        m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; font-size: 10px; }");

        AudioMetadata meta = m_engine->getMetadata();
        QString artist = QString::fromStdString(meta.artist);
        QString titleOrAlbum = QString::fromStdString(meta.title.empty() ? meta.album : meta.title);
        searchAndDownloadCover(artist, titleOrAlbum, basePath);
    } else {
        setCoverImage(coverPath);
    }
}

void MainWindow::setCoverImage(const QString& coverPath) {
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

MainWindow::MainWindow(AudioEngine* engine, QWidget *parent)
    : QWidget(parent), m_engine(engine)
{
    m_eqWin = std::make_unique<EqualizerWindow>(engine);
    m_visWin = std::make_unique<VisualizerWindow>(engine);
    m_skinWin = std::make_unique<SkinWindow>(this);
    m_networkManager = new QNetworkAccessManager(this);

    setWindowTitle("Bramble Audio Player");
    resize(550, 300);
    setStyleSheet(SkinManager::getMainWindowStyle());

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* leftLayout = new QVBoxLayout();

    m_lblInfo = new QLabel("BRAMBLE AUDIO v1.0", this);
    m_lblInfo->setObjectName("DisplayLabel");
    m_lblInfo->setAlignment(Qt::AlignCenter);
    m_lblInfo->setMinimumHeight(100);
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

    // Time label below cover
    m_lblTime = new QLabel("0:00/0:00", this);
    m_lblTime->setAlignment(Qt::AlignCenter);
    m_lblTime->setStyleSheet("QLabel { color: #ccc; font-size: 12px; font-weight: bold; }");
    mainLayout->addWidget(m_lblTime);

    // Connect signals
    connect(this, &MainWindow::coverDownloaded, this, &MainWindow::setCoverImage);
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
            m_lblInfo->setText(QString::fromStdString(m_engine->getFormattedMetadata()));
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
        if (m_engine->hasValidDecoder()) {
            m_engine->play();
            m_btnPlay->setText("PAUSE");
        } else {
            openFileDialog();
        }
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
    if (m_engine->hasValidDecoder()) {
        float duration = m_engine->getDuration();
        float position = m_engine->getPosition();
        m_sliderSeek->setRange(0, (int)duration);

        auto formatTime = [](float t) {
            int mins = (int)t / 60;
            int secs = (int)t % 60;
            if (mins >= 60) {
                return QString("%1:%2").arg(mins / 60).arg(mins % 60, 2, 10, QChar('0')).append(":").append(QString::number(secs).rightJustified(2, '0'));
            }
            return QString("%1:%2").arg(mins).arg(secs, 2, 10, QChar('0'));
        };

        m_lblTime->setText(QString("%1/%2").arg(formatTime(position)).arg(formatTime(duration)));

        if (!m_isDraggingSeek && !m_engine->isPlaying()) {
            m_sliderSeek->setValue((int)position);
        } else if (m_engine->isPlaying() && !m_isDraggingSeek) {
            m_sliderSeek->setValue((int)position);
        }
    }
}
