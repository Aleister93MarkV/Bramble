#ifndef VISUALIZERWINDOW_HPP
#define VISUALIZERWINDOW_HPP

#include <QWidget>
#include <QTimer>
#include "AudioEngine.hpp"

class VisualizerWindow : public QWidget {
    Q_OBJECT
public:
    explicit VisualizerWindow(AudioEngine* engine, QWidget *parent = nullptr);
    void setMode(int mode) { m_mode = mode; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AudioEngine* m_engine;
    QTimer* m_timer;
    int m_mode; // 0: Spectrum, 1: Scope, 2: VU
};

#endif // VISUALIZERWINDOW_HPP
