#ifndef VISUALIZERWINDOW_HPP
#define VISUALIZERWINDOW_HPP

#include <QWidget>
#include <QTimer>
#include <QResizeEvent>
#include "AudioEngine.hpp"

class VisualizerWindow : public QWidget {
    Q_OBJECT
public:
    explicit VisualizerWindow(AudioEngine* engine, QWidget *parent = nullptr);
    ~VisualizerWindow() override;
    void setMode(int mode) { m_mode = mode; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    AudioEngine* m_engine = nullptr;
    QTimer* m_timer = nullptr;
    int m_mode = 0;
};

#endif // VISUALIZERWINDOW_HPP