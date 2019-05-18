/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */
#include "TootData.h"
#include <QtCore>

//自分の使用しているアカウントのidリスト
QByteArrayList TootData::static_owner_user_id_list;

TootAccountData::TootAccountData(const QJsonObject &target) {
  if (target.isEmpty())
    return;
  id = target["id"].toString().toLatin1();
  user_name = target["username"].toString();
  display_name = target["display_name"].toString();
  if (display_name.isEmpty()) {
    display_name = user_name;
  }
  acct = target["acct"].toString();
  avatar = target["avatar"].toString(); //アイコン
  following_count = target["following_count"].toInt();
  followers_count = target["followers_count"].toInt();
  locked = target["locked"].toBool();
  description = target["note"].toString();
}

/*
 * 引数:なし
 * 戻値:TootAccountData(本来の投稿したユーザ情報)
 * 概要:投稿したユーザ情報を返す。ブーストなどはブースト元のアカウント情報を返す。
 */
const TootAccountData &TootData::getOriginalAccountData() const {
  if (reblog != nullptr)
    return reblog->account;
  return account;
}

TootRelationshipData::TootRelationshipData(const QJsonObject &target) {
  following = target["following"].toBool();
  followed = target["followed_by"].toBool();
  muting = target["muting"].toBool();
  blocking = target["blocking"].toBool();
}

/*
 * 引数:display_url(表示してるURLなど), full_url(完全なリンク)
 * 戻値:なし
 * 概要:エントリーを追加する。TootDataからの呼び出しを想定。
 */
void TootUrlData::addUrlPair(const QString &display_url,
                             const QString &full_url) {
  data.append(QPair<QString, QString>(display_url, full_url));
}

/*
 * 引数:index(0から始まるエントリー数)
 * 戻値:QString(表示してるURLなど)
 * 概要:指定されたindexの表示用URLを返す。indexが範囲外ならば空文字を返す。
 */
QString TootUrlData::getDisplayUrl(unsigned int index) const {
  return (index < data.count()) ? data.at(index).first : QString();
}

/*
 * 引数:index(0から始まるエントリー数)
 * 戻値:QString(完全なリンク)
 * 概要:指定されたindexの完全なリンクを返す。indexが範囲外ならば空文字を返す。
 */
QString TootUrlData::getFullUrl(unsigned int index) const {
  return (index < data.count()) ? data.at(index).second : QString();
}

TootMediaData::TootMediaData(const QJsonArray &target) {
  for (const QJsonValue &entry : target) {
    const QJsonObject entry_object = entry.toObject();
    TootMediaDataEntry s;
    s.type = entry_object["type"].toString();
    s.url = entry_object["url"].toString();
    s.remote_url = entry_object["remote_url"].toString();
    s.preview_url = entry_object["preview_url"].toString();
    s.text_url = entry_object["text_url"].toString();
    media_list.append(s);
  }
}

TootData::TootData(const QJsonObject &target) {
  if (target.find("id") == target.end())
    return;
  id = target["id"].toString().toLatin1();
  created_at.setTimeSpec(Qt::UTC);
  created_at =
      QDateTime::fromString(target["created_at"].toString(), Qt::ISODateWithMs);

  QJsonObject application_json = target["application"].toObject();
  application = QPair<QString, QString>(application_json["name"].toString(),
                                        application_json["website"].toString());
  uri = target["uri"].toString();
  url = target["url"].toString();
  analyzeContent(target["content"].toString());

  account = TootAccountData(target["account"].toObject());
  media = TootMediaData(target["media_attachments"].toArray());

  flag = 0;
  if (target["reblogged"].toBool()) {
    flag |= 1 << 0; //一般化
  }
  if (target["favourited"].toBool()) {
    flag |= 1 << 1;
  }
  if (static_owner_user_id_list.contains(account.getId())) {
    flag |= 1 << 2;
  }
  if (target["visibility"].toString() == "private") {
    flag |= 1 << 3;
  }
  if (target["visibility"].toString() == "direct") {
    flag |= 1 << 4;
  }

  if (QJsonValue reblog_status = target["reblog"]; reblog_status.isObject()) {
    QJsonObject reblog_object = reblog_status.toObject(); // constならいらない
    reblog = new TootData(reblog_object);
    media = reblog->getMediaData(); //扱いやすいように
    content = reblog->getContent();
  }
  //製作中
}

TootData::~TootData() { delete reblog; }

/*
 * 引数:なし
 * 戻値:bool(ブーストしていたらtrue、それ以外はfalse)
 * 概要:自分がブーストしているか返す。
 */
bool TootData::isBoosted() const { return flag & (1 << 0); }

/*
 * 引数:なし
 * 戻値:bool(お気に入りに登録していたらtrue、それ以外はfalse)
 * 概要:自分がお気に入りに登録しているか返す。
 */
bool TootData::isFavourited() const { return flag & (1 << 1); }

/*
 * 引数:なし
 * 戻値:bool(自分の投稿ならtrue、それ以外はfalse)
 * 概要:自分の投稿かどうかを返す。
 */
bool TootData::isTootOwner() const { return flag & (1 << 2); }

/*
 * 引数:なし
 * 戻値:bool(非公開の投稿ならtrue、それ以外はfalse)
 * 概要:非公開の投稿(フォローしてないと見れない)かどうかを返す。
 */
bool TootData::isPrivateToot() const { return flag & (1 << 3); }

/*
 * 引数:なし
 * 戻値:bool(ダイレクトメッセージならtrue、それ以外はfalse)
 * 概要:ダイレクトメッセージかどうかを返す。
 */
bool TootData::iSDirectMessage() const { return flag & (1 << 4); }

/*
 * 引数:なし
 * 戻値:QString(投稿に使用されたアプリケーションの名前)
 * 概要:viaソフトウェアの名前を返す。(空であることあり)
 */
QString TootData::getApplicationName() const { return application.first; }

/*
 * 引数:なし
 * 戻値:QString(投稿に使用されたアプリケーションのウェブページのURL)
 * 概要:viaソフトウェアのホームページを返す。(空であることあり)
 */
QString TootData::getApplicationSite() const { return application.second; }

/*
 * 引数:なし
 * 戻値:なし
 * 概要:MastodonはHTMLで本文投げてくるのでHTMLタグ取り除いてURL抽出などをする。絶対重い。
 */
void TootData::analyzeContent(QString c /*remove使うため参照ではない*/) {
  // spanとpを消す
  c.remove(QRegExp("<\\/?(span|p)[^>]*>"));
  c.replace("<br>", "\n").replace("<br />", "\n");
  c.replace("&gt;", ">")
      .replace("&lt;", "<")
      .replace("&amp;", "&"); //見やすいように連結した。

  QRegularExpressionMatchIterator &&link_tags =
      QRegularExpression("<a[^>]* href=\"([^\"]*)\"[^>]*>([^<]*)<\\/a>")
          .globalMatch(c);
  while (link_tags.hasNext()) {
    QRegularExpressionMatch entry = link_tags.next();
    url_list.addUrlPair(entry.captured(2), entry.captured(1));
  }
  c.replace(QRegExp("<a[^>]* href=\"[^\"]*\"[^>]*>([^<]*)<\\/a>"), "\\1");
  content = c;
}

/*
 * 引数:id(使用しているアカウントのid)
 * 戻値:なし
 * 概要:トゥートが自分のものか判定するためのidを登録する。
 */
void TootData::addOwnerUserId(const QByteArray &id) {
  static_owner_user_id_list += id;
}

TootNotificationData::TootNotificationData(const QJsonObject &target) {
  if (target.find("id") == target.end())
    return;
  QString &&type_str = target["type"].toString();

  // type判定
  if (type_str == "mention") {
    type = Event::Mention;
  } else if (type_str == "reblog") {
    type = Event::Boost;
  } else if (type_str == "favourite") {
    type = Event::Favourite;
  } else if (type_str == "follow") {
    type = Event::Follow;
  } else {
    type = Event::NoEvent;
  }

  account = TootAccountData(target["account"].toObject());
  if (target["status"].isObject()) {
    status = TootData(target["status"].toObject());
  }
}
