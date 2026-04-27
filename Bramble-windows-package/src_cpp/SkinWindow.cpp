#include "SkinWindow.hpp"
#include <QGroupBox>

QString getSkinPreviewStyle(const QString& themeName) {
    SkinManager::Theme t = SkinManager::getThemes().value(themeName, SkinManager::createClassicTheme());
    return QString(
        "QWidget { background-color: %1; color: %2; }"
        "QComboBox { background-color: %6; color: %7; border: 1px solid %5; }"
        "QLabel { color: %2; font-weight: bold; }"
        "QGroupBox { background-color: %1; border: 2px solid %5; }"
        "QLabel#DisplayLabel { background-color: %3; color: %4; font-weight: bold; border: 2px solid %5; font-family: monospace; }"
        "QPushButton { background-color: %6; color: %7; border: 2px outset %6; font-weight: bold; padding: 2px; }"
        "QPushButton:pressed { border: 2px inset %6; background-color: #8C8C96; }"
    ).arg(t.bgMain, t.accent, t.bgDisplay, t.textDigital, t.accent, t.btnFace, t.btnText);
}

SkinWindow::SkinWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
{
    setWindowTitle("Skin Selector");
    resize(350, 300);
    setStyleSheet(getSkinPreviewStyle("Classic"));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Select Theme:", this);
    mainLayout->addWidget(titleLabel);

    m_themeSelector = new QComboBox(this);
    QMap<QString, SkinManager::Theme> themes = SkinManager::getThemes();
    for (auto it = themes.constBegin(); it != themes.constEnd(); ++it) {
        m_themeSelector->addItem(it.key());
    }
    connect(m_themeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        QString themeName = m_themeSelector->itemText(index);
        onThemeSelected(themeName);
    });
    mainLayout->addWidget(m_themeSelector);

    QLabel* previewLabel = new QLabel("Preview:", this);
    mainLayout->addWidget(previewLabel);

    QGroupBox* previewGroup = new QGroupBox(this);
    QVBoxLayout* previewLayout = new QVBoxLayout();

    m_previewDisplay = new QLabel("SONG NAME", this);
    m_previewDisplay->setObjectName("DisplayLabel");
    previewLayout->addWidget(m_previewDisplay);

    m_previewBtn = new QPushButton("Button", this);
    previewLayout->addWidget(m_previewBtn);

    previewGroup->setLayout(previewLayout);
    mainLayout->addWidget(previewGroup);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* applyBtn = new QPushButton("Apply", this);
    connect(applyBtn, &QPushButton::clicked, this, &SkinWindow::applyCurrentSkin);
    btnLayout->addWidget(applyBtn);

    QPushButton* closeBtn = new QPushButton("Close", this);
    connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    onThemeSelected("Classic");
}

void SkinWindow::onThemeSelected(const QString& themeName) {
    m_currentTheme = themeName;
    setStyleSheet(getSkinPreviewStyle(themeName));
}

void SkinWindow::applyCurrentSkin() {
    SkinManager::setCurrentTheme(m_currentTheme);
    emit themeApplied(m_currentTheme);
    close();
}