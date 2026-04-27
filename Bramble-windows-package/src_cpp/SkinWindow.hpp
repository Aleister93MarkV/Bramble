#ifndef SKINWINDOW_HPP
#define SKINWINDOW_HPP

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include "SkinManager.hpp"

QString getSkinPreviewStyle(const QString& themeName);

class SkinWindow : public QWidget {
    Q_OBJECT
public:
    explicit SkinWindow(QWidget *parent = nullptr);

signals:
    void themeApplied(const QString& themeName);

private slots:
    void onThemeSelected(const QString& themeName);
    void applyCurrentSkin();

private:
    QComboBox* m_themeSelector;
    QLabel* m_previewDisplay;
    QPushButton* m_previewBtn;
    QString m_currentTheme;
};

#endif // SKINWINDOW_HPP