#ifndef EQUALIZERWINDOW_HPP
#define EQUALIZERWINDOW_HPP

#include <QWidget>
#include <QSlider>
#include <vector>
#include "AudioEngine.hpp"

class EqualizerWindow : public QWidget {
    Q_OBJECT
public:
    explicit EqualizerWindow(AudioEngine* engine, QWidget *parent = nullptr);

private:
    AudioEngine* m_engine;
    std::vector<QSlider*> m_sliders;
    QSlider* m_crystSlider;
};

#endif // EQUALIZERWINDOW_HPP
