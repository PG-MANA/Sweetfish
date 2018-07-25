/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "Sweetfish.h"
#include "UI/MainWindow.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  //全般設定
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
  app.setWindowIcon(QIcon(":/icon-normal.png")); //埋め込み

  QString setting_file(DEFAULT_SETTING_FILE_NAME);
  MainWindow window;
  return window.init(setting_file) ? window.show(), app.exec() : EXIT_FAILURE;
}
