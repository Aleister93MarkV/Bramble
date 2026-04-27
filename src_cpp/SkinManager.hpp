#ifndef SKINMANAGER_HPP
#define SKINMANAGER_HPP

#include <QMap>
#include <QString>

class SkinManager {
public:
  struct Theme {
    QString name;
    QString bgMain;
    QString bgDisplay;
    QString textDigital;
    QString accent;
    QString btnFace;
    QString btnText;
  };

  static QMap<QString, Theme> &getThemes() {
    static QMap<QString, Theme> themes;
    if (themes.isEmpty()) {
      themes["Classic"] = createClassicTheme();
      themes["Neon Night"] = createNeonNightTheme();
      themes["Retro PC"] = createRetroPCTheme();
      themes["Matrix"] = createMatrixTheme();
      themes["Synthwave"] = createSynthwaveTheme();
      themes["Cherry"] = createCherryTheme();
      themes["Ocean"] = createOceanTheme();
      themes["Sunset"] = createSunsetTheme();
      themes["Red Fox"] = createRedFoxTheme();
    }
    return themes;
  }

  static Theme createClassicTheme() {
    Theme t;
    t.name = "Classic";
    t.bgMain = "#242436";
    t.bgDisplay = "#000014";
    t.textDigital = "#00FF41";
    t.accent = "#00F2FF";
    t.btnFace = "#B4B4BE";
    t.btnText = "#14141E";
    return t;
  }

  static Theme createNeonNightTheme() {
    Theme t;
    t.name = "Neon Night";
    t.bgMain = "#0D0D1A";
    t.bgDisplay = "#1A0A1A";
    t.textDigital = "#FF00FF";
    t.accent = "#FF00FF";
    t.btnFace = "#2A1A3A";
    t.btnText = "#E0E0FF";
    return t;
  }

  static Theme createRetroPCTheme() {
    Theme t;
    t.name = "Retro PC";
    t.bgMain = "#008080";
    t.bgDisplay = "#000000";
    t.textDigital = "#00FF00";
    t.accent = "#FFFF00";
    t.btnFace = "#C0C0C0";
    t.btnText = "#000000";
    return t;
  }

  static Theme createMatrixTheme() {
    Theme t;
    t.name = "Matrix";
    t.bgMain = "#000000";
    t.bgDisplay = "#001100";
    t.textDigital = "#00FF00";
    t.accent = "#00FF00";
    t.btnFace = "#003300";
    t.btnText = "#00FF00";
    return t;
  }

  static Theme createSynthwaveTheme() {
    Theme t;
    t.name = "Synthwave";
    t.bgMain = "#1A0A2E";
    t.bgDisplay = "#2D1B4E";
    t.textDigital = "#FF6EC7";
    t.accent = "#FF00FF";
    t.btnFace = "#4B0082";
    t.btnText = "#FFFFFF";
    return t;
  }

  static Theme createCherryTheme() {
    Theme t;
    t.name = "Cherry";
    t.bgMain = "#2B0000";
    t.bgDisplay = "#1A0000";
    t.textDigital = "#FF0000";
    t.accent = "#FF4444";
    t.btnFace = "#4A0000";
    t.btnText = "#FF8888";
    return t;
  }

  static Theme createOceanTheme() {
    Theme t;
    t.name = "Ocean";
    t.bgMain = "#001A33";
    t.bgDisplay = "#000D1A";
    t.textDigital = "#00CCFF";
    t.accent = "#0099FF";
    t.btnFace = "#003366";
    t.btnText = "#66CCFF";
    return t;
  }

  static Theme createSunsetTheme() {
    Theme t;
    t.name = "Sunset";
    t.bgMain = "#331A00";
    t.bgDisplay = "#1A0D00";
    t.textDigital = "#FF9900";
    t.accent = "#FF6600";
    t.btnFace = "#663300";
    t.btnText = "#FFCC66";
    return t;
  }

  static Theme createRedFoxTheme() {
    Theme t;
    t.name = "Red Firefox";
    t.bgMain = "#FF6600";
    t.bgDisplay = "#331A00";
    t.textDigital = "#FFFFFF";
    t.accent = "#FF4400";
    t.btnFace = "#CC4400";
    t.btnText = "#FFFFFF";
    return t;
  }

  static void setCurrentTheme(const QString &name) { getThemeRef() = name; }

  static QString getCurrentTheme() { return getThemeRef(); }

  static QString getMainWindowStyle() {
    Theme t = getThemes().value(getCurrentTheme(), createClassicTheme());
    return QString(
               "QWidget { background-color: %1; color: %2; }"
               "QLabel#DisplayLabel { background-color: %3; color: %4; "
               "font-weight: bold; border: 2px solid %5; font-family: "
               "monospace; font-size: 14px; }"
               "QPushButton { background-color: %6; color: %7; border: 2px "
               "outset %6; font-weight: bold; padding: 2px; }"
               "QPushButton:pressed { border: 2px inset %6; background-color: "
               "#8C8C96; }"
               "QSlider::groove:horizontal { border: 1px solid #999999; "
               "height: 8px; background: %3; margin: 2px 0; }"
               "QSlider::handle:horizontal { background: %6; border: 1px solid "
               "#5c5c5c; width: 18px; margin: -2px 0; border-radius: 3px; }")
        .arg(t.bgMain, t.accent, t.bgDisplay, t.textDigital, t.accent,
             t.btnFace, t.btnText);
  }

  static QString getEqWindowStyle() {
    Theme t = getThemes().value(getCurrentTheme(), createClassicTheme());
    return QString(
               "QWidget { background-color: %1; color: %2; }"
               "QSlider::groove:vertical { border: 1px solid #999999; width: "
               "8px; background: %3; margin: 0 2px; }"
               "QSlider::handle:vertical { background: %6; border: 1px solid "
               "#5c5c5c; height: 18px; margin: 0 -2px; border-radius: 3px; }"
               "QSlider::groove:horizontal { border: 1px solid #999999; "
               "height: 8px; background: %3; margin: 2px 0; }"
               "QSlider::handle:horizontal { background: %6; border: 1px solid "
               "#5c5c5c; width: 18px; margin: -2px 0; border-radius: 3px; }")
        .arg(t.bgMain, t.accent, t.bgDisplay, t.textDigital, t.accent,
             t.btnFace, t.btnText);
  }

  static QString getVisWindowStyle() {
    Theme t = getThemes().value(getCurrentTheme(), createClassicTheme());
    return QString("QWidget { background-color: %1; color: %2; }")
        .arg(t.bgMain, t.accent);
  }

private:
  static QString &getThemeRef() {
    static QString theme = "Classic";
    return theme;
  }
};

#endif // SKINMANAGER_HPP