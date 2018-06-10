/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "../Sweetfish.h"
#include "Network.h"

Network::Network() {}

Network::~Network() {}

/*
 * 引数:url(送信するURL)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlにGETリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::get(const QUrl &url) {
  QNetworkRequest req;
  req.setUrl(url);
  return get(req);
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestを使ってGETリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::get(QNetworkRequest &req) {
  req.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
  return qnet.get(req);
}

/*
 * 引数:url(送信するURL), data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::post(const QUrl &url, const QByteArray &data) {
  QNetworkRequest req;
  req.setUrl(url);
  return post(req, data);
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく),
 * data(本体、POSTするデータ) 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::post(QNetworkRequest &req, const QByteArray &data) {
  req.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
  // req.setRawHeader("Content-Length", data.size());
  req.setHeader(QNetworkRequest::ContentTypeHeader,
                "application/x-www-form-urlencoded");
  return qnet.post(req, data);
}

/*
 * 引数:url(送信するURL), data(本体、POSTするデータ)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlとdataを使ってPOSTリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::upload(const QUrl &url,
                               const QList<QByteArrayList> &data) {
  QNetworkRequest req;
  req.setUrl(url);
  return upload(req, data);
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく),
 * data(本体、POSTするデータ) 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestlとdataを使ってmultipart/form-data形式のPOSTリクエストを送る。
 * data=>QByteArrayList(0:title, 1:file_name, 2:mime_type, 3:data)
 */
QNetworkReply *Network::upload(QNetworkRequest &req,
                               const QList<QByteArrayList> &data) {
  req.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
  QHttpMultiPart *multiformPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);

  for (const QByteArrayList &entry : data) {
    if (entry.size() != 4) continue;  //無効
    QHttpPart dataPart;
    if (!entry.at(1).isEmpty()) {
      dataPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant("form-data; name=\"" + entry.at(0) +
                                  "\"; filename=\"" + entry.at(1) + "\""));
    } else {
      dataPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                         QVariant("form-data; name=\"" + entry.at(0) + "\""));
    }
    dataPart.setHeader(QNetworkRequest::ContentTypeHeader,
                       QVariant(entry.at(2)));
    dataPart.setBody(entry.at(3));
    multiformPart->append(dataPart);
  }
  QNetworkReply *rep = qnet.post(req, multiformPart);
  multiformPart->setParent(rep);
  return rep;
}

/*
 * 引数:url(送信するURL)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたurlにDELETEリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::del(const QUrl &url) {
  QNetworkRequest req;
  req.setUrl(url);
  return del(req);
}

/*
 * 引数:req(QNetworkRequestで、最低URLは設定しておく)
 * 戻値:データを受け取るためのQNetworkReplyのポインタ
 * 概要:渡されたQNetworkRequestを使ってDELETEリクエストを送る。受信は戻り値であるQNetworkReplyポインタを使う。
 */
QNetworkReply *Network::del(QNetworkRequest &req) {
  req.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
  return qnet.deleteResource(req);
}

/*
 * 引数:なし
 * 戻値:ユーザーエージェント(QByteArray)
 * 概要:マクロ定数を使用するのを避けるために作られたメンバ
 */
QByteArray Network::getUserAgent() const { return QByteArray(USER_AGENT); }
