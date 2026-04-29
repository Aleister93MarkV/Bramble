#include "SoundFontManager.hpp"
#include <QSettings>
#include <QFile>
#include <QDir>

SoundFontManager::SoundFontManager(QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint)
{
    setWindowTitle("SoundFont Manager");
    setMinimumSize(450, 350);
    resize(500, 400);
    
    m_configPath = QDir::homePath() + "/.bramble/soundfonts.ini";
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QLabel* lblTitle = new QLabel("SoundFonts Disponíveis:", this);
    mainLayout->addWidget(lblTitle);
    
    m_list = new QListWidget(this);
    mainLayout->addWidget(m_list);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_btnAdd = new QPushButton("Adicionar", this);
    m_btnRemove = new QPushButton("Remover", this);
    btnLayout->addWidget(m_btnAdd);
    btnLayout->addWidget(m_btnRemove);
    mainLayout->addLayout(btnLayout);
    
    connect(m_btnAdd, &QPushButton::clicked, this, &SoundFontManager::onAddClicked);
    connect(m_btnRemove, &QPushButton::clicked, this, &SoundFontManager::onRemoveClicked);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &SoundFontManager::onItemDoubleClicked);
    
    loadSoundFontList();
}

SoundFontManager::~SoundFontManager() {
}

void SoundFontManager::loadSoundFontList() {
    QFile file(m_configPath);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream in(&file);
        in >> m_soundFonts;
        file.close();
    }
    
    m_list->clear();
    for (const QString& sf : m_soundFonts) {
        if (QFile::exists(sf)) {
            m_list->addItem(sf);
        }
    }
    
    if (!m_soundFonts.isEmpty() && QFile::exists(m_soundFonts.first())) {
        m_currentSoundFont = m_soundFonts.first();
    }
}

void SoundFontManager::saveSoundFontList() {
    QFile file(m_configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QDataStream out(&file);
        out << m_soundFonts;
        file.close();
    }
}

void SoundFontManager::onAddClicked() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Selecionar SoundFont",
        QDir::homePath(),
        "SoundFont (*.sf2 *.sf3);;Todos os arquivos (*)"
    );
    
    if (!fileName.isEmpty()) {
        addSoundFont(fileName);
    }
}

void SoundFontManager::addSoundFont(const QString& path) {
    if (!m_soundFonts.contains(path)) {
        m_soundFonts.append(path);
        m_list->addItem(path);
        saveSoundFontList();
        emit soundFontChanged(path);
    }
}

void SoundFontManager::onRemoveClicked() {
    QListWidgetItem* item = m_list->currentItem();
    if (item) {
        removeSoundFont(item->text());
    }
}

void SoundFontManager::removeSoundFont(const QString& path) {
    m_soundFonts.removeAll(path);
    
    int row = m_list->currentRow();
    delete m_list->takeItem(row);
    
    saveSoundFontList();
}

void SoundFontManager::onItemDoubleClicked(QListWidgetItem* item) {
    if (item) {
        m_currentSoundFont = item->text();
        emit soundFontChanged(m_currentSoundFont);
    }
}

void SoundFontManager::setDefaultSoundFont(const QString& path) {
    m_currentSoundFont = path;
    emit soundFontChanged(path);
}