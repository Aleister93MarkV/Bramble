#ifndef SOUNDFONTMANAGER_HPP
#define SOUNDFONTMANAGER_HPP

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <QStringList>
#include <vector>
#include <string>

class SoundFontManager : public QWidget {
    Q_OBJECT
public:
    explicit SoundFontManager(QWidget *parent = nullptr);
    ~SoundFontManager() override;
    
    QString getCurrentSoundFont() const { return m_currentSoundFont; }
    QStringList getAvailableSoundFonts() const { return m_soundFonts; }
    
    void addSoundFont(const QString& path);
    void removeSoundFont(const QString& path);
    void setDefaultSoundFont(const QString& path);

signals:
    void soundFontChanged(const QString& path);

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    void loadSoundFontList();
    void saveSoundFontList();
    
    QListWidget* m_list;
    QPushButton* m_btnAdd;
    QPushButton* m_btnRemove;
    QStringList m_soundFonts;
    QString m_currentSoundFont;
    QString m_configPath;
};

#endif // SOUNDFONTMANAGER_HPP