/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "Sweetfish.h"
#include "UI/MainWindow.h"
#include <QTranslator>
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  // hacky translator stuff I picked up off the qt tutorials https://doc.qt.io/qt-5/qtlinguist-arrowpad-example.html
  // attempts to load a translation file for the current locale
  QTranslator translator;
  QString locale = QLocale::system().name();
  translator.load(QString("sweetfish-") + locale);
  app.installTranslator(&translator);
  //全般設定
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  app.setWindowIcon(QIcon(":/icon-normal.png")); //埋め込み

  QString setting_file(DEFAULT_SETTING_FILE_NAME);
  MainWindow window;
  return window.init(setting_file) ? window.show(), app.exec() : EXIT_FAILURE;
}
