#include "VisualizerWindow.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <vector>

VisualizerWindow::VisualizerWindow(AudioEngine* engine, QWidget *parent) 
    : QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint), m_engine(engine), m_mode(0) 
{
    setWindowTitle("Crystal Visualizer");
    resize(600, 200);
    setStyleSheet("background-color: #05050A;");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_timer->start(16); // ~60fps
}

void VisualizerWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    int w = width();
    int h = height();

    if (m_mode == 0) { // Spectrum
        std::vector<float> spectrum;
        m_engine->getSpectrum(spectrum);
        
        int bars = (int)spectrum.size();
        if (bars == 0) return;
        
        int barWidth = w / bars;
        painter.setBrush(QColor("#00F2FF"));
        painter.setPen(Qt::NoPen);
        
        for (int i = 0; i < bars; ++i) {
            int barHeight = (int)(spectrum[i] * h);
            painter.drawRect(i * barWidth, h - barHeight, barWidth - 1, barHeight);
        }
    } else if (m_mode == 1) { // Scope (Oscilloscope)
        std::vector<float> waveform;
        m_engine->getWaveform(waveform);
        
        int count = (int)waveform.size();
        if (count == 0) return;

        QPen pen(QColor("#00FF96"));
        pen.setWidth(1);
        painter.setPen(pen);
        
        QPointF lastPoint;
        for (int i = 0; i < count; ++i) {
            float x = (float)(i * w) / count;
            float y = h / 2.0f + (waveform[i] * h / 2.0f);
            
            if (i > 0) {
                painter.drawLine(lastPoint, QPointF(x, y));
            }
            lastPoint = QPointF(x, y);
        }
    } else if (m_mode == 2) { // VU Meter
        float vL, vR;
        m_engine->getLevels(vL, vR);
        
        int barMaxW = w - 40;
        painter.setBrush(QColor("#FF3232"));
        painter.setPen(Qt::NoPen);
        
        // Left Channel
        painter.drawRect(20, h/2 - 30, (int)(vL * barMaxW), 20);
        // Right Channel
        painter.drawRect(20, h/2 + 10, (int)(vR * barMaxW), 20);
    }
}
