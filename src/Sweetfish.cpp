/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "Sweetfish.h"
#include "UI/MainWindow.h"
#include <QApplication>
#include <QStandardPaths>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  // Load Translations
  QTranslator translator;
  QString locale_file =
      QString("translations/lang_") + QLocale::system().name() + QString(".qm");
  QString locale_path =
      QStandardPaths::locate(QStandardPaths::AppDataLocation, locale_file);
  if (locale_path.isEmpty() || !translator.load(locale_path)) {
    qDebug() << "Failed to load translation file:  " << locale_path << "("
             << locale_file << ")";
  }
  app.installTranslator(&translator);
  app.setWindowIcon(QIcon(":/icon-normal.png")); // 埋め込み

  QString setting_file(DEFAULT_SETTING_FILE_NAME);
  MainWindow window;
  return window.init(setting_file) ? window.show(), app.exec() : EXIT_FAILURE;
}
