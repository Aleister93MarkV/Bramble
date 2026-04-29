#include "VisualizerWindow.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

VisualizerWindow::VisualizerWindow(AudioEngine* engine, QWidget *parent) 
    : QWidget(parent), m_engine(engine), m_mode(0) 
{
    setWindowTitle("Bramble Spectrum");
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: #0A0A0F;");
    resize(600, 250);
    setMinimumSize(300, 150);
    
    m_timer = new QTimer(this);
    m_timer->setInterval(16);
    connect(m_timer, &QTimer::timeout, this, static_cast<void (QWidget::*)()>(&QWidget::update));
    m_timer->start();
}

VisualizerWindow::~VisualizerWindow() {
    if (m_timer) {
        m_timer->stop();
    }
}

void VisualizerWindow::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
}

void VisualizerWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    if (!m_engine || !isVisible()) return;
    
    QPainter painter(this);
    if (!painter.isActive()) return;
    
    int w = width();
    int h = height();
    if (w <= 0 || h <= 0) return;
    
    std::vector<float> spectrum;
    m_engine->getSpectrum(spectrum);
    
    int bars = 64;
    int barWidth = w / bars;
    if (barWidth < 1) barWidth = 1;
    
    painter.setPen(Qt::NoPen);
    
    for (int i = 0; i < bars; ++i) {
        float val = 0.0f;
        if (!spectrum.empty() && i >= 0 && i < (int)spectrum.size()) {
            val = spectrum[i];
        }
        val = std::min(val, 1.0f);
        int barHeight = (int)(val * h * 0.95f);
        if (barHeight < 1) barHeight = 1;
        
        float t = (float)i / bars;
        QColor col;
        if (t < 0.33f) col = QColor("#00F2FF");
        else if (t < 0.66f) col = QColor("#00FF96");
        else if (t < 0.85f) col = QColor("#FFFF00");
        else col = QColor("#FF3232");
        
        col.setAlpha(230);
        painter.setBrush(col);
        painter.drawRect(i * barWidth + 1, h - barHeight, barWidth - 2, barHeight);
    }
}