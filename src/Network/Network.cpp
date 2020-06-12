/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "Network.h"
#include "../Sweetfish.h"
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

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
 * 引数:req(QNetworkRequestで、最低URLは設定しておく),
 *      info(file_nameやMIMEなどの情報[QByteArrayList])
 *      data(読み込み用のQIODevice)
 * 概要:渡されたQNetworkRequestlとdataを使ってmultipart/form-data形式のPOSTリクエストを送る。
 * info=>QByteArrayList(0:title, 1:file_name, 2:mime_type)
 */
QNetworkReply *Network::upload(QNetworkRequest &req, const QByteArrayList &info,
                               QIODevice &data) {
  req.setHeader(QNetworkRequest::UserAgentHeader, getUserAgent());
  QHttpMultiPart *multiformPart =
      new QHttpMultiPart(QHttpMultiPart::FormDataType);
  if (info.size() != 3)
    return nullptr; //無効
  QHttpPart dataPart;
  if (!info.at(1).isEmpty()) {
    dataPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"" + info.at(0) +
                                "\"; filename=\"" + info.at(1) + "\""));
  } else {
    dataPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant("form-data; name=\"" + info.at(0) + "\""));
  }
  dataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(info.at(2)));
  dataPart.setBodyDevice(&data);
  multiformPart->append(dataPart);
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
