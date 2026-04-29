#include "MainWindow.hpp"
#include "SkinManager.hpp"
#include "MIDIManager.hpp"
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
        pixmap = pixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_lblCover->setPixmap(pixmap);
        m_lblCover->setText("");
        m_lblCover->setStyleSheet("QLabel { border: 2px solid #555; }");
        m_coverPixmap = pixmap;
    } else {
        m_lblCover->setText("No Cover");
        m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; font-size: 12px; }");
    }
}

void MainWindow::showCoverFull() {
    if (m_coverPixmap.isNull()) {
        QString path = m_engine->getMetadata().title.c_str();
        if (path.isEmpty()) path = "No Cover";
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("Album Cover");
    dialog.setStyleSheet("QDialog { background-color: #111; }");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QLabel* label = new QLabel(&dialog);
    if (!m_coverPixmap.isNull()) {
        QPixmap scaled = m_coverPixmap.scaled(500, 500, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        label->setPixmap(scaled);
    } else {
        label->setText("No Cover");
        label->setStyleSheet("QLabel { color: #888; font-size: 20px; }");
    }
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    
    QPushButton* btnClose = new QPushButton("Close", &dialog);
    btnClose->setStyleSheet("QPushButton { background-color: #333; color: #ccc; padding: 10px; }");
    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);
    layout->addWidget(btnClose);
    
    dialog.exec();
}

MainWindow::MainWindow(AudioEngine* engine, QWidget *parent)
    : QWidget(parent), m_engine(engine), m_currentTrackIndex(-1)
{
    m_eqWin = std::make_unique<EqualizerWindow>(engine);
    m_visWin = std::make_unique<VisualizerWindow>(engine);
    m_vuWin = std::make_unique<VUMeterWindow>(engine);
    m_sfWin = std::make_unique<SoundFontManager>(this);
    m_skinWin = std::make_unique<SkinWindow>(this);
    m_networkManager = new QNetworkAccessManager(this);
    
    connect(m_sfWin.get(), &SoundFontManager::soundFontChanged, this, [](const QString& path) {
        auto& midi = MIDIManagerSingleton::instance();
        midi.loadSoundFont(path);
    });
    m_fileModel = new QFileSystemModel(this);

    setWindowTitle("Bramble Audio Player");
    resize(750, 400);
    setStyleSheet(SkinManager::getMainWindowStyle());

    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* leftLayout = new QVBoxLayout();

    m_lblInfo = new QLabel("BRAMBLE AUDIO v1.0", this);
    m_lblInfo->setObjectName("DisplayLabel");
    m_lblInfo->setAlignment(Qt::AlignCenter);
    m_lblInfo->setMinimumHeight(100);
    leftLayout->addWidget(m_lblInfo);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnOpen = new QPushButton("OPEN", this);
    m_btnPlay = new QPushButton("PLAY", this);
    m_btnEQ = new QPushButton("EQ", this);
    m_btnVis = new QPushButton("VIS", this);
    m_btnVUMeter = new QPushButton("VU", this);
    m_btnSF = new QPushButton("SF", this);
    m_btnSkin = new QPushButton("SKIN", this);

    btnLayout->addWidget(m_btnOpen);
    btnLayout->addWidget(m_btnPlay);
    btnLayout->addWidget(m_btnEQ);
    btnLayout->addWidget(m_btnVis);
    btnLayout->addWidget(m_btnVUMeter);
    btnLayout->addWidget(m_btnSF);
    btnLayout->addWidget(m_btnSkin);
    leftLayout->addLayout(btnLayout);

    m_sliderSeek = new QSlider(Qt::Horizontal, this);
    leftLayout->addWidget(m_sliderSeek);

    QHBoxLayout* volLayout = new QHBoxLayout();
    QLabel* volLabel = new QLabel("Vol:", this);
    m_sliderVol = new QSlider(Qt::Horizontal, this);
    m_sliderVol->setRange(0, 100);
    m_sliderVol->setValue(100);
    volLayout->addWidget(volLabel);
    volLayout->addWidget(m_sliderVol);
    leftLayout->addLayout(volLayout);

    mainLayout->addLayout(leftLayout);

    QVBoxLayout* rightLayout = new QVBoxLayout();

    QLabel* browserLabel = new QLabel("File Browser", this);
    browserLabel->setAlignment(Qt::AlignCenter);
    browserLabel->setStyleSheet("QLabel { font-weight: bold; color: #0af; }");
    rightLayout->addWidget(browserLabel);

    m_fileBrowser = new QTreeView(this);
    m_fileModel->setRootPath(QDir::homePath());
    m_fileBrowser->setModel(m_fileModel);
    m_fileBrowser->setRootIndex(m_fileModel->index(QDir::homePath()));
    m_fileBrowser->setAnimated(false);
    m_fileBrowser->setIndentation(20);
    m_fileBrowser->setSortingEnabled(true);
    m_fileBrowser->setColumnWidth(0, 200);
    m_fileBrowser->setMinimumWidth(220);
    rightLayout->addWidget(m_fileBrowser);

    QLabel* playlistLabel = new QLabel("Playlist / CD Tracks", this);
    playlistLabel->setAlignment(Qt::AlignCenter);
    playlistLabel->setStyleSheet("QLabel { font-weight: bold; color: #0af; }");
    rightLayout->addWidget(playlistLabel);

    m_fileList = new QListWidget(this);
    m_fileList->setMinimumWidth(220);
    m_fileList->setMaximumHeight(120);
    rightLayout->addWidget(m_fileList);

    mainLayout->addLayout(rightLayout);

    m_lblCover = new QLabel(this);
    m_lblCover->setFixedSize(200, 200);
    m_lblCover->setAlignment(Qt::AlignCenter);
    m_lblCover->setText("No Cover");
    m_lblCover->setStyleSheet("QLabel { background-color: #222; color: #888; border: 2px solid #444; font-size: 12px; }");
    m_lblCover->setScaledContents(false);
    leftLayout->addWidget(m_lblCover);
    
    QPushButton* m_btnCoverFull = new QPushButton("Expand Cover", this);
    m_btnCoverFull->setStyleSheet("QPushButton { background-color: #333; color: #ccc; border: 1px solid #555; padding: 5px; } QPushButton:hover { background-color: #444; }");
    connect(m_btnCoverFull, &QPushButton::clicked, this, &MainWindow::showCoverFull);
    leftLayout->addWidget(m_btnCoverFull);

    m_lblTime = new QLabel("0:00/0:00", this);
    m_lblTime->setAlignment(Qt::AlignCenter);
    m_lblTime->setStyleSheet("QLabel { color: #ccc; font-size: 12px; font-weight: bold; }");
    leftLayout->addWidget(m_lblTime);

    connect(this, &MainWindow::coverDownloaded, this, &MainWindow::setCoverImage);
    m_btnOpen->setContextMenuPolicy(Qt::ActionsContextMenu);
    
    m_openMenu = new QMenu(this);
    QAction* actFile = new QAction("Open File...", this);
    QAction* actFolder = new QAction("Open Folder...", this);
    QAction* actCD = new QAction("Play Audio CD", this);
    
    connect(actFile, &QAction::triggered, this, &MainWindow::openFileDialog);
    connect(actFolder, &QAction::triggered, this, &MainWindow::openFolderDialog);
    connect(actCD, &QAction::triggered, this, &MainWindow::playAudioCD);
    
    m_openMenu->addAction(actFile);
    m_openMenu->addAction(actFolder);
    m_openMenu->addAction(actCD);
    m_btnOpen->setMenu(m_openMenu);
    
    connect(m_btnPlay, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    connect(m_btnEQ, &QPushButton::clicked, this, &MainWindow::toggleEQ);
    connect(m_btnVis, &QPushButton::clicked, this, &MainWindow::toggleVis);
    connect(m_btnVUMeter, &QPushButton::clicked, this, &MainWindow::toggleVUMeter);
    connect(m_btnSF, &QPushButton::clicked, this, &MainWindow::toggleSoundFont);
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

    connect(m_fileBrowser, &QTreeView::doubleClicked, this, &MainWindow::onFileDoubleClicked);
    connect(m_fileList, &QListWidget::itemDoubleClicked, this, &MainWindow::onTrackDoubleClicked);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateUI);
    m_timer->start(50);
}

void MainWindow::openFileDialog() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Audio File", "", "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a *.aac *.wma *.wmv *.asf *.ac3 *.mod *.xm *.s3m *.it);;All Files (*.*)");
    if (!fileName.isEmpty()) {
        loadAudioFile(fileName);
    }
}

QStringList MainWindow::findAvailableCDROM() {
    QStringList cdrives;
    QDir dir("/dev");
    QFileInfoList devices = dir.entryInfoList(QDir::System);
    for (const QFileInfo& info : devices) {
        QString name = info.fileName();
        if (name.startsWith("sr") || name.startsWith("scd") || name.contains("cdrom")) {
            cdrives.append("/dev/" + name);
        }
    }
    return cdrives;
}

void MainWindow::playAudioCD() {
    QStringList cdroms = findAvailableCDROM();
    
    if (cdroms.isEmpty()) {
        m_lblInfo->setText("No CD-ROM found");
        m_fileList->clear();
        m_fileList->addItem("(No CD-ROM detected)");
        return;
    }

    QString selectedCD;
    if (cdroms.size() == 1) {
        selectedCD = cdroms.first();
    } else {
        bool found = false;
        for (const QString& cd : cdroms) {
            QFile file(cd);
            if (file.open(QFile::ReadOnly)) {
                selectedCD = cd;
                file.close();
                found = true;
                break;
            }
        }
        if (!found) {
            selectedCD = cdroms.first();
        }
    }

    m_lblInfo->setText("CD-ROM: " + selectedCD);
    m_currentCDDevice = selectedCD;
    loadAudioFile(selectedCD);
    loadAlbumCover(selectedCD);
    
    QStringList trackList;
    //int trackCount = m_engine->getCDTrackCount();
    //if (trackCount > 0) {
    //    for (int i = 0; i < trackCount; ++i) {
    //        QString info = QString::fromStdString(m_engine->getCDTrackInfo(i));
    //        trackList.append(info.isEmpty() ? QString("Track %1").arg(i + 1) : info);
    //    }
    //} else {
        for (int i = 1; i <= 20; ++i) {
            trackList.append(QString("Track %1").arg(i));
        }
    //}
    setCDTrackList(trackList);
}

void MainWindow::loadAudioFile(const QString& fileName) {
    if (fileName.isEmpty()) {
        m_lblInfo->setText("Error: Empty filename");
        return;
    }

    qDebug() << "Loading file:" << fileName;
    bool loaded = m_engine->loadFile(fileName.toStdString());
    qDebug() << "Load result:" << loaded;
    
    if (loaded) {
        QFileInfo fileInfo(fileName);
        m_lblInfo->setText(QString::fromStdString(m_engine->getFormattedMetadata()));
        m_sliderSeek->setRange(0, (int)m_engine->getDuration());
        qDebug() << "Calling play()";
        m_engine->play();
        qDebug() << "play() returned";
        m_btnPlay->setText("PAUSE");
        loadAlbumCover(fileName);
    } else {
        m_lblInfo->setText("Error loading file");
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

void MainWindow::toggleVUMeter() {
    if (m_vuWin->isVisible()) {
        m_vuWin->hide();
    } else {
        m_vuWin->show();
    }
}

void MainWindow::toggleSoundFont() {
    if (m_sfWin->isVisible()) {
        m_sfWin->hide();
    } else {
        m_sfWin->show();
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

void MainWindow::onFileDoubleClicked(const QModelIndex& index) {
    QString path = m_fileModel->filePath(index);
    QFileInfo info(path);
    
    if (info.isDir()) {
        m_fileBrowser->setRootIndex(index);
        refreshFileList(path);
    } else {
        QStringList audioExts = {"mp3", "wav", "flac", "ogg", "m4a", "aac", "wma", "wmv", "asf", "mod", "xm", "s3m", "it", "ac3", "dts", "ape", "tta"};
        if (audioExts.contains(info.suffix().toLower())) {
            loadAudioFile(path);
            loadAlbumCover(path);
        }
    }
}

void MainWindow::onTrackDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    int index = m_fileList->row(item);
    
    if (index >= 0 && index < m_audioFiles.size()) {
        QString data = m_audioFiles[index];
        
        if (!data.startsWith("/")) {
            int trackIndex = data.toInt();
            //m_engine->setCDTrack(trackIndex);
            m_currentTrackIndex = index;
        } else {
            loadAudioFile(data);
            loadAlbumCover(data);
            m_currentTrackIndex = index;
        }
    }
}

void MainWindow::refreshFileList(const QString& path) {
    m_currentPath = path;
    m_audioFiles.clear();
    m_fileList->clear();
    
    QDir dir(path);
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.ogg" << "*.m4a" << "*.aac" 
            << "*.wma" << "*.wmv" << "*.asf" << "*.ac3" << "*.dts" << "*.ape" << "*.tta"
            << "*.mod" << "*.xm" << "*.s3m" << "*.it" << "*.opus";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
    for (const QFileInfo& file : files) {
        m_audioFiles.append(file.filePath());
        m_fileList->addItem(file.fileName());
    }
    
    if (m_audioFiles.isEmpty()) {
        m_fileList->addItem("(No audio files)");
    }
}

void MainWindow::setCDTrackList(const QStringList& tracks) {
    m_audioFiles.clear();
    m_fileList->clear();
    for (int i = 0; i < tracks.size(); ++i) {
        m_audioFiles.append(QString::number(i));
        m_fileList->addItem(QString("Track %1 - %2").arg(i + 1).arg(tracks[i]));
    }
}

void MainWindow::openFolderDialog() {
    QString folder = QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!folder.isEmpty()) {
        QModelIndex idx = m_fileModel->index(folder);
        m_fileBrowser->setRootIndex(idx);
        refreshFileList(folder);
    }
}
