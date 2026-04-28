#include "EqualizerWindow.hpp"
#include "SkinManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

EqualizerWindow::EqualizerWindow(AudioEngine* engine, QWidget *parent)
    : QWidget(parent, Qt::Window), m_engine(engine) 
{
    setWindowTitle("Bramble Equalizer & Crystallizer");
    resize(600, 350);
    setStyleSheet(SkinManager::getEqWindowStyle());

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* eqLayout = new QHBoxLayout();
    
    // Create 18 sliders for the EQ
    for (int i = 0; i < 18; ++i) {
        QSlider* slider = new QSlider(Qt::Vertical, this);
        slider->setRange(-12, 12);
        slider->setValue(0);
        
        connect(slider, &QSlider::valueChanged, this, [this, i](int value) {
            m_engine->setEQBand(i, (float)value);
        });
        
        eqLayout->addWidget(slider);
        m_sliders.push_back(slider);
    }
    
    mainLayout->addLayout(eqLayout);
    
    // Crystallizer slider
    QHBoxLayout* crystLayout = new QHBoxLayout();
    QLabel* crystLabel = new QLabel("Crystallizer", this);
    m_crystSlider = new QSlider(Qt::Horizontal, this);
    m_crystSlider->setRange(0, 20);
    m_crystSlider->setValue(0);
    
    connect(m_crystSlider, &QSlider::valueChanged, this, [this](int value) {
        m_engine->setCrystallizer((float)value);
    });
    
    crystLayout->addWidget(crystLabel);
    crystLayout->addWidget(m_crystSlider);
    
    mainLayout->addLayout(crystLayout);
}
