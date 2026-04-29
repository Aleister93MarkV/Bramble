#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QString>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QListWidget>
#include <QFileSystemModel>
#include <QTreeView>
#include <QPixmap>
#include <QDialog>
#include <memory>

#include "AudioEngine.hpp"
#include "EqualizerWindow.hpp"
#include "VisualizerWindow.hpp"
#include "VUMeterWindow.hpp"
#include "SoundFontManager.hpp"
#include "MIDIManager.hpp"
#include "SkinWindow.hpp"

class MainWindow : public QWidget {
    Q_OBJECT
signals:
    void coverDownloaded(const QString& coverPath);
public:
    explicit MainWindow(AudioEngine* engine, QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void openFileDialog();
    void openFolderDialog();
    void playAudioCD();
    void togglePlayPause();
    void toggleEQ();
    void toggleVis();
    void toggleVUMeter();
    void toggleSoundFont();
    void toggleSkin();
    void updateUI();
    void loadAlbumCover(const QString& audioPath);
    void onFileDoubleClicked(const QModelIndex& index);
    void onTrackDoubleClicked(QListWidgetItem* item);
    void refreshFileList(const QString& path);
    void setCDTrackList(const QStringList& tracks);

private:
    void loadAudioFile(const QString& fileName);
    QStringList findAvailableCDROM();
    void searchAndDownloadCover(const QString& artist, const QString& album, const QString& basePath);
    void downloadCoverFromReleaseId(const QString& releaseId, const QString& basePath);
    void saveCoverToFile(QNetworkReply* reply, const QString& basePath);
    void setCoverImage(const QString& coverPath);
    void showCoverFull();
    
    QPixmap m_coverPixmap;
    AudioEngine* m_engine;

    // Sub-windows
    std::unique_ptr<EqualizerWindow> m_eqWin;
    std::unique_ptr<VisualizerWindow> m_visWin;
    std::unique_ptr<VUMeterWindow> m_vuWin;
    std::unique_ptr<SoundFontManager> m_sfWin;
    std::unique_ptr<SkinWindow> m_skinWin;

    // UI Elements
    QPushButton* m_btnPlay;
    QPushButton* m_btnOpen;
    QPushButton* m_btnEQ;
    QPushButton* m_btnVis;
    QPushButton* m_btnVUMeter;
    QPushButton* m_btnSF;
    QPushButton* m_btnSkin;
    QMenu* m_openMenu;
    QLabel* m_lblInfo;
    QLabel* m_lblCover;
    QLabel* m_lblTime;
    QSlider* m_sliderSeek;
    QSlider* m_sliderVol;
    QTimer* m_timer;
    QNetworkAccessManager* m_networkManager;
    QListWidget* m_fileList;
    QFileSystemModel* m_fileModel;
    QTreeView* m_fileBrowser;
    QString m_currentPath;
    QStringList m_audioFiles;
    int m_currentTrackIndex = -1;
    QString m_currentFilePath;
    QString m_currentCDDevice;
    bool m_isDraggingSeek = false;
};

#endif // MAINWINDOW_HPP
