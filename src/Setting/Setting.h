/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
/*
 * もともとは暗号化などをするつもりだったがいい案が浮かばないので現在はかんたんなラッパー程度になってる
 */
#pragma once

#include "../Sweetfish.h"
#include <QSettings>

class Setting {
public:
  Setting(const QString &ini_file_name);
  ~Setting();

  // Window関係
  QByteArray getGeometry() const;
  void setGeometry(const QByteArray &geo);

  unsigned int getTootLimit() const;
  void setTootLimit(unsigned int limit);

  // API関係
  QByteArray getClientId() const;
  void setClientId(const QByteArray &id);

  QByteArray getClientSecret() const;
  void setClientSecret(const QByteArray &secret);

  QByteArray getAccessToken() const;
  void setAccessToken(const QByteArray &token);

  QString getInstanceDomain() const;
  void setInstanceDomain(const QString &domain);

  QByteArray getUserId() const;
  void setUserId(const QByteArray &id);

  QString getUserName() const;
  void setUserName(const QString &name);

private:
  QString getFilePath(const QString &ini_file_name) const;
  QSettings setting;
};
