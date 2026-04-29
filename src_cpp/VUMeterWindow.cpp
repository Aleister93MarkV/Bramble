#include "VUMeterWindow.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <cmath>

VUMeterWindow::VUMeterWindow(AudioEngine* engine, QWidget *parent) 
    : QWidget(parent), m_engine(engine) 
{
    setWindowTitle("Bramble VU Meter");
    setStyleSheet("background-color: #0A0A0F;");
    resize(400, 200);
    setMinimumSize(300, 150);
    
    m_timer = new QTimer(this);
    m_timer->setInterval(16);
    connect(m_timer, &QTimer::timeout, this, static_cast<void (QWidget::*)()>(&QWidget::update));
    m_timer->start();
}

VUMeterWindow::~VUMeterWindow() {
    if (m_timer) {
        m_timer->stop();
    }
}

void VUMeterWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    
    if (!m_engine || !isVisible()) return;
    
    QPainter painter(this);
    if (!painter.isActive()) return;
    
    int w = width();
    int h = height();
    if (w <= 0 || h <= 0) return;
    
    float vL, vR;
    m_engine->getLevels(vL, vR);
    
    vL = std::min(1.0f, vL * 1.5f);
    vR = std::min(1.0f, vR * 1.5f);
    
    int numBarsL = 30;
    int numBarsR = 30;
    int totalBars = numBarsL + numBarsR + 1;
    int barW = (w - 20) / totalBars;
    if (barW < 3) barW = 3;
    int barH = h - 40;
    int startX = 10;
    int bottomY = h - 20;
    
    painter.setPen(Qt::NoPen);
    
    // Left channel (bottom to top, left to right)
    for (int i = 0; i < numBarsL; ++i) {
        float threshold = (float)i / numBarsL;
        bool active = vL > threshold;
        
        QColor col;
        if (active) {
            float intensity = (float)i / numBarsL;
            if (intensity > 0.85f) col = QColor("#FF3232");
            else if (intensity > 0.7f) col = QColor("#FFFF00");
            else col = QColor("#00FF96");
        } else {
            col = QColor("#1A1A1F");
        }
        
        painter.setBrush(col);
        int barHeight = (int)(vL * barH);
        if (barHeight < 1) barHeight = 1;
        painter.drawRect(startX + i * (barW + 1), bottomY - barHeight, barW, barHeight);
    }
    
    // Center gap
    painter.setBrush(QColor("#0A0A0F"));
    painter.drawRect(startX + numBarsL * (barW + 1), bottomY - barH, barW + 1, barH);
    
    // Right channel (bottom to top, right to left)
    for (int i = 0; i < numBarsR; ++i) {
        float threshold = (float)(numBarsR - 1 - i) / numBarsR;
        bool active = vR > threshold;
        
        QColor col;
        if (active) {
            float intensity = (float)(numBarsR - 1 - i) / numBarsR;
            if (intensity > 0.85f) col = QColor("#FF3232");
            else if (intensity > 0.7f) col = QColor("#FFFF00");
            else col = QColor("#00FF96");
        } else {
            col = QColor("#1A1A1F");
        }
        
        painter.setBrush(col);
        int barHeight = (int)(vR * barH);
        if (barHeight < 1) barHeight = 1;
        int xPos = startX + (numBarsL + 1 + i) * (barW + 1);
        painter.drawRect(xPos, bottomY - barHeight, barW, barHeight);
    }
    
    // Labels
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(QColor("#666"));
    painter.drawText(startX, h - 2, "L");
    painter.drawText(startX + (numBarsL + 1 + numBarsR) * (barW + 1), h - 2, "R");
}