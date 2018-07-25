/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#pragma once

#include <QNetworkAccessManager>

class QNetworkRequest;
class QNetworkReply;
class QUrl;
class QByteArray;

class Network {
public:
  Network();
  ~Network();
  QNetworkReply *get(const QUrl &url);
  QNetworkReply *get(QNetworkRequest &req);
  QNetworkReply *post(const QUrl &url, const QByteArray &data);
  QNetworkReply *post(QNetworkRequest &req, const QByteArray &data);
  QNetworkReply *upload(const QUrl &url, const QList<QByteArrayList> &data);
  QNetworkReply *upload(QNetworkRequest &req,
                        const QList<QByteArrayList> &data);
  QNetworkReply *del(const QUrl &url);
  QNetworkReply *del(QNetworkRequest &req);

private:
  QNetworkAccessManager qnet;
  inline QByteArray getUserAgent() const;
};
