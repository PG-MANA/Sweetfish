/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "Sweetfish.h"
#include "UI/MainWindow.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  // Load Translations
  QTranslator translator;
  QString locale = QLocale::system().name();
  if (!translator.load(QString("/usr/lib/") + QString(APP_NAME).toLower() +
                       QString("/locales/") + locale)) { // RPMでの配置
    if (!translator.load(QString("locales/") + locale)) {
      qDebug() << "Failed to load translator for " << locale;
    }
  }
  app.installTranslator(&translator);
  app.setWindowIcon(QIcon(":/icon-normal.png")); // 埋め込み

  QString setting_file(DEFAULT_SETTING_FILE_NAME);
  MainWindow window;
  return window.init(setting_file) ? window.show(), app.exec() : EXIT_FAILURE;
}
