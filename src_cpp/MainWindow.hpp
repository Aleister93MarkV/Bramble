#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QString>
#include <memory>
#include "AudioEngine.hpp"
#include "EqualizerWindow.hpp"
#include "VisualizerWindow.hpp"
#include "SkinWindow.hpp"

class MainWindow : public QWidget {
    Q_OBJECT
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
    
    QString m_currentFilePath;
    bool m_isDraggingSeek = false;
};

#endif // MAINWINDOW_HPP
