#ifndef VUMETERWINDOW_HPP
#define VUMETERWINDOW_HPP

#include <QWidget>
#include <QTimer>
#include "AudioEngine.hpp"

class VUMeterWindow : public QWidget {
    Q_OBJECT
public:
    explicit VUMeterWindow(AudioEngine* engine, QWidget *parent = nullptr);
    ~VUMeterWindow() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    AudioEngine* m_engine = nullptr;
    QTimer* m_timer = nullptr;
};

#endif // VUMETERWINDOW_HPP