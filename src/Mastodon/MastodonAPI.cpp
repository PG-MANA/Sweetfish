/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "../Network/Network.h"
#include "../Sweetfish.h"
#include "MastodonAPI.h"
#include "MastodonUrl.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QtCore>

MastodonAPI::MastodonAPI() {}

MastodonAPI::MastodonAPI(const MastodonAPI &other)
    : user_id(other.user_id), access_token(other.access_token),
      domain(other.domain) {}

MastodonAPI::~MastodonAPI() {}

/*
 * 引数:domain(登録するMastodonAPI インスタンスのFQDN)
 * 戻値:受信用QNetworkReply
 * 概要:対象インスタンスにアプリを登録する。
 */
QNetworkReply *MastodonAPI::registerApp(const QString &domain) {
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
QUrl MastodonAPI::getAuthorizeUrl(const QString &domain,
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
QNetworkReply *MastodonAPI::requestAccessToken(
    const QString &domain, const QByteArray &client_id,
    const QByteArray &client_secret, const QString &authorization_token_code) {
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
void MastodonAPI::setAccessToken(const QByteArray &token) {
  access_token = token;
}

/*
 * 引数:domain(使用するMastodon インスタンスのFQDN)
 * 戻値:なし
 * 概要:使用するインスタンスのドメイン名をセットする。
 */
void MastodonAPI::setInstanceDomain(const QString &instance_domain) {
  domain = instance_domain;
}

/*
 * 引数:user_id(アプリ認証をしたユーザのID)
 * 戻値:なし
 * 概要:アプリ認証をしたユーザのIDをセットする。
 */
void MastodonAPI::setUserId(const QByteArray &id) { user_id = id; }

/*
 * 引数:なし
 * 戻値:QByteArray(アプリ認証をしたユーザのID)
 * 概要:アプリ認証をしたユーザのIDを取得する。
 */
QByteArray MastodonAPI::getUserId() const { return user_id; }

/*
 * 引数:since_id(since_id以降のトゥートを持ってくる)
 * 戻値:getしたあとのQNetworkReply
 * 概要:HomeTimeを取得。
 */
QNetworkReply *MastodonAPI::requestHomeTimeLine(const QByteArray &since_id) {
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
QNetworkReply *MastodonAPI::requestUserStream() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::user_stream);
  return get(req);
}

/*
 * 引数:message(ツイート内容), media_id(メディア投稿用ID[カンマ区切り],
 * reply_id(返信先のID) 戻値:結果取得用のQNetworkReply
 * 概要:messageをトゥートする。
 */
QNetworkReply *MastodonAPI::requestToot(const QString &message,
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
QNetworkReply *MastodonAPI::requestDeleteToot(const QByteArray &id) {
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
QNetworkReply *MastodonAPI::requestBoost(const QByteArray &id) {
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
QNetworkReply *MastodonAPI::requestFavourite(const QByteArray &id) {
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
QNetworkReply *MastodonAPI::requestCurrentAccountInfo() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::current_account);
  return get(req);
}

/*
 * 引数:user_id(投稿を取得するユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDの投稿一覧を取得する。
 */
QNetworkReply *MastodonAPI::requestUserStatuses(const QByteArray &user_id) {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts + user_id +
             MastodonUrl::accounts_status);
  return get(req);
}

/*
 * 引数:user_id(関係を取得するユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDと利用中のアカウントの関係を取得する。(注意:結果は配列で帰る)
 */
QNetworkReply *MastodonAPI::requestUserRelationship(const QByteArray &user_id) {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts_relationship +
             "?id[]=" + user_id);

  return get(req);
}

/*
 * 引数:user_id(フォローするユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDをフォローする
 */
QNetworkReply *MastodonAPI::requestFollow(const QByteArray &user_id) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts + user_id +
             MastodonUrl::accounts_follow);
  return post(req, QByteArray());
}

/*
 * 引数:user_id(フォロー解除するユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDのフォローを解除する
 */
QNetworkReply *MastodonAPI::requestUnfollow(const QByteArray &user_id) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts + user_id +
             MastodonUrl::accounts_unfollow);
  return post(req, QByteArray());
}

/*
 * 引数:user_id(ブロックするユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDをブロックする
 */
QNetworkReply *MastodonAPI::requestBlock(const QByteArray &user_id) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts + user_id +
             MastodonUrl::accounts_block);
  return post(req, QByteArray());
}

/*
 * 引数:user_id(フォロー解除するユーザのID)
 * 戻値:結果取得用のQNetworkReply
 * 概要:指定したUserIDのブロックを解除する
 */
QNetworkReply *MastodonAPI::requestUnblock(const QByteArray &user_id) {
  QNetworkRequest req;
  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::accounts + user_id +
             MastodonUrl::accounts_unblock);
  return post(req, QByteArray());
}

/*
 * 引数:data(アップロードするデータ),mime_type(dataのMIME-TYPE)
 * 戻値:結果取得用のQNetworkReply
 * 概要:メディアのアップロードを行う。postdataはQHttpMultiPartなどを使いmultipart/form-data形式にすること。
 */
QNetworkReply *MastodonAPI::requestMediaUpload(const QByteArray &data,
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
QNetworkReply *MastodonAPI::requestGetLists() {
  QNetworkRequest req;

  req.setUrl(MastodonUrl::scheme + domain + MastodonUrl::lists);
  return get(req);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *MastodonAPI::get(QNetworkRequest &req) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.get(req);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest), data(POSTデータ)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *MastodonAPI::post(QNetworkRequest &req, const QByteArray &data) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.post(req, data);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest),
 * data(POSTデータ[QList<QByteArrayList>]) 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *MastodonAPI::upload(QNetworkRequest &req,
                                   const QList<QByteArrayList> &data) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.upload(req, data);
}

/*
 * 引数:req(URLなどをセットしたQNetworkRequest)
 * 戻値:受信用QNetworkReply
 * 概要:Authorizationヘッダを作成。AccessTokenがないときは使用しない。
 */
QNetworkReply *MastodonAPI::del(QNetworkRequest &req) {
  req.setRawHeader("Authorization", "Bearer " + access_token);
  return net.del(req);
}
