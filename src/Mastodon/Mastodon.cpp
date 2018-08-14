/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "../Network/Network.h"
#include "../Sweetfish.h"
#include "Mastodon.h"
#include "MastodonUrl.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtCore>

Mastodon::Mastodon() {}

Mastodon::Mastodon(const Mastodon &other) {
  access_token = other.access_token;
  domain = other.domain;
  user_id = other.user_id;
  // Networkは新規
}

Mastodon::~Mastodon() {}

/*
 * 引数:domain(登録するMastodon インスタンスのFQDN)
 * 戻値:受信用QNetworkReply
 * 概要:対象インスタンスにアプリを登録する。
 */
QNetworkReply *Mastodon::registerApp(const QString &domain) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::register_app);
  return net.post(req,
                  QByteArray("client_name=" APP_NAME_LONG
                             "&redirect_uris=urn:ietf:wg:oauth:2.0:oob&scopes="
                             "read write follow&website=" APP_HOMEPAGE));
}

/*
 * 引数:domain(登録するMastodon インスタンスのFQDN),
 * client_id(register_appで得たID) 戻値:ブラウザでアクセスしてもらうURL
 * 概要:対象インスタンスでのユーザー認証URLを作成する。
 */
QUrl Mastodon::getAuthorizeUrl(const QString &domain,
                               const QString &client_id) const {
  return QUrl(MastodonUrl::scheme + domain + MastodonUrl::authorize +
              "response_type=code&redirect_uri=urn:ietf:wg:oauth:2.0:oob&scope="
              "read write follow&client_id=" +
              client_id);
}

/*
 * 引数:domain(登録するMastodon インスタンスのFQDN),
 * client_id(register_appで得たID), client_secret(register_appで得たSecret),
 * authorization_token_code(ブラウザで表示された認証コード)
 * 戻値:受信用QNetworkReply
 * 概要:アクセストークンをもらうために認証コードを送る。
 */
QNetworkReply *
Mastodon::requestAccessToken(const QString &domain, const QByteArray &client_id,
                             const QByteArray &client_secret,
                             const QString &authorization_token_code) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::token);
  return net.post(req, "grant_type=authorization_code&redirect_uri=urn:ietf:wg:"
                       "oauth:2.0:oob&client_id=" +
                           client_id + "&client_secret=" + client_secret +
                           "&code=" + authorization_token_code.toUtf8());
}

/*
 * 引数:token(送られてきたアクセストークン
 * 戻値:なし
 * 概要:アクセストークンを保存する。
 */
void Mastodon::setAccessToken(const QByteArray &token) { access_token = token; }

/*
 * 引数:domain(使用するMastodon インスタンスのFQDN)
 * 戻値:なし
 * 概要:使用するインスタンスのドメイン名をセットする。
 */
void Mastodon::setInstanceDomain(const QString &instance_domain) {
  domain = instance_domain;
}

/*
 * 引数:user_id(アプリ認証をしたユーザのID)
 * 戻値:なし
 * 概要:アプリ認証をしたユーザのIDをセットする。
 */
void Mastodon::setUserId(const QByteArray &id) { user_id = id; }

/*
 * 引数:なし
 * 戻値:QByteArray(アプリ認証をしたユーザのID)
 * 概要:アプリ認証をしたユーザのIDを取得する。
 */
QByteArray Mastodon::getUserId() const { return user_id; }

/*
 * 引数:since_id(since_id以降のトゥートを持ってくる)
 * 戻値:getしたあとのQNetworkReply
 * 概要:HomeTimeを取得。
 */
QNetworkReply *Mastodon::requestHomeTimeLine(const QByteArray &since_id) {
  QNetworkRequest req;
  QUrl qurl(MastodonUrl::scheme + domain + MastodonUrl::home_timeline);
  QUrlQuery qurl_query;

  if (!since_id.isEmpty()) {
    qurl_query.addQueryItem("since_id", since_id);
  }
  qurl.setQuery(qurl_query);
  req.setUrl(qurl);
  return get(req);
}

/*
 * 引数:なし
 * 戻値:getしたあとのQNetworkReply
 * 概要:user_Stream、いわゆるタイムラインのストリーム。
 */
QNetworkReply *Mastodon::requestUserStream() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::user_stream);
  return get(req);
}

/*
 * 引数:message(ツイート内容), media_id(メディア投稿用ID[カンマ区切り],
 * reply_id(返信先のID) 戻値:結果取得用のQNetworkReply
 * 概要:messageをトゥートする。
 */
QNetworkReply *Mastodon::requestToot(const QString &message,
                                     const QByteArray &media_id,
                                     const QByteArray &reply_id) {
  QByteArray body("status=" + QUrl::toPercentEncoding(message));
  QNetworkRequest req;

  if (!media_id.isEmpty()) {
    for (const QString &id : media_id.split(',')) {
      if (!id.isEmpty()) {
        body.append("&media_ids[]=" + id);
      }
    }
  }
  if (!reply_id.isEmpty()) {
    body.append("&in_reply_to_id=" + reply_id);
  }
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::statuses);
  return post(req, body);
}

/*
 * 引数:id(削除をするID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:該当idの投稿を削除する。自分の発言かどうかのチェックはしてない。
 */
QNetworkReply *Mastodon::requestDeleteToot(const QByteArray &id) {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::statuses + "/" + id);
  //送信(Delete)
  return del(req);
}

/*
 * 引数:id(ブーストをするID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:idをブーストをする。
 */
QNetworkReply *Mastodon::requestBoost(const QByteArray &id) {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::statuses + "/" + id +
             MastodonUrl::reblog);
  //送信(Delete)
  return post(req, QByteArray());
}

/*
 * 引数:id(お気に入りに登録するツイートのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:idをお気に入りに登録する。
 */
QNetworkReply *Mastodon::requestFavourite(const QByteArray &id) {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::statuses + "/" + id +
             MastodonUrl::favourite);
  //送信(Delete)
  return post(req, QByteArray());
}

/*
 * 引数:なし
 * 戻値:結果取得用のQNetworkReply
 * 概要:アプリ認証をしたアカウントの情報を取得する。
 */
QNetworkReply *Mastodon::requestCurrentAccountInfo() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts_current);
  return get(req);
}

/*
 * 引数:data(アップロードするデータ),mime_type(dataのMIME-TYPE)
 * 戻値:結果取得用のQNetworkReply
 * 概要:メディアのアップロードを行う。postdataはQHttpMultiPartなどを使いmultipart/form-data形式にすること。
 */
QNetworkReply *Mastodon::requestMediaUpload(const QByteArray &data,
                                            const QByteArray &mime_type) {
  QNetworkRequest req;
  QList<QByteArrayList> upload_data;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::media_upload);
  //アップロードリスト作成
  upload_data.push_back(
      QByteArrayList({"file", "upload" /*暫定*/, mime_type, data}));
  //送信
  return upload(req, upload_data);
}

/*
 * 引数:なし
 * 戻値:getしたあとのQNetworkReply
 * 概要:Listの一覧を取得する。
 */
QNetworkReply *Mastodon::requestGetLists() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::lists);
  return get(req);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *Mastodon::get(QNetworkRequest &req) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.get(req);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest), data(POSTデータ)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *Mastodon::post(QNetworkRequest &req, const QByteArray &data) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.post(req, data);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest),
 * data(POSTデータ[QList<QByteArrayList>]) 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *Mastodon::upload(QNetworkRequest &req,
                                const QList<QByteArrayList> &data) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.upload(req, data);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *Mastodon::del(QNetworkRequest &req) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.del(req);
}
