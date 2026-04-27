#include <QApplication>
#include "MainWindow.hpp"
#include "AudioEngine.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("CrystalAudioPlayer");
    app.setApplicationVersion("1.0.0");

    AudioEngine engine;
    MainWindow mainWin(&engine);

    mainWin.show();

    return app.exec();
}
