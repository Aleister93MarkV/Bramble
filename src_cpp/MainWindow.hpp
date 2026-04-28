#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>
#include "AudioEngine.hpp"
#include "EqualizerWindow.hpp"
#include "VisualizerWindow.hpp"
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
    void togglePlayPause();
    void toggleEQ();
    void toggleVis();
    void toggleSkin();
    void updateUI();
    void loadAlbumCover(const QString& audioPath);

private:
    void searchAndDownloadCover(const QString& artist, const QString& album, const QString& basePath);
    void downloadCoverFromReleaseId(const QString& releaseId, const QString& basePath);
    void saveCoverToFile(QNetworkReply* reply, const QString& basePath);
    void setCoverImage(const QString& coverPath);
    AudioEngine* m_engine;

    // Sub-windows
    std::unique_ptr<EqualizerWindow> m_eqWin;
    std::unique_ptr<VisualizerWindow> m_visWin;
    std::unique_ptr<SkinWindow> m_skinWin;

    // UI Elements
    QPushButton* m_btnPlay;
    QPushButton* m_btnOpen;
    QPushButton* m_btnEQ;
    QPushButton* m_btnVis;
    QPushButton* m_btnSkin;
    QLabel* m_lblInfo;
    QLabel* m_lblCover;
    QLabel* m_lblTime;
    QSlider* m_sliderSeek;
    QSlider* m_sliderVol;
    QTimer* m_timer;
    QNetworkAccessManager* m_networkManager;

    QString m_currentFilePath;
    bool m_isDraggingSeek = false;
};

#endif // MAINWINDOW_HPP
