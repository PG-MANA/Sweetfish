/*
 * This software is Licensed under the Apache License Version 2.0
 * See LICENSE
 */

#include "Setting.h"
#include "../Sweetfish.h"
#include <QtCore>

Setting::Setting(const QString &ini_file_name)
    : setting(getFilePath(ini_file_name), QSettings::IniFormat) {}

Setting::~Setting() {}

/*
 * 引数:ini_file_name(設定ファイル名)
 * 戻値:ファイルフルパス
 * 概要:ini_file_nameの保存場所のフルパスを返す
 */
QString Setting::getFilePath(const QString &ini_file_name) const {
  QDir setting_dir(
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
  if (!setting_dir.exists(APP_NAME)) {
    setting_dir.mkdir(APP_NAME);
  }
  setting_dir.cd(APP_NAME);
  return setting_dir.filePath(ini_file_name);
}

/*
 * 引数:なし
 * 戻値:Window座標
 * 概要:Window座標を読み取る
 */
QByteArray Setting::getGeometry() const {
  return setting.value("Window/geometry").toByteArray();
}

/*
 * 引数:geo(QByteArray化した座標)
 * 戻値:なし
 * 概要:Window座標を書き込む
 */
void Setting::setGeometry(const QByteArray &geo) {
  setting.setValue("Window/geometry", geo);
}

/*
 * 引数:なし
 * 戻値:Windowの状態
 * 概要:Windowの状態を読み取る
 */
QByteArray Setting::getState() const {
    return setting.value("Window/state").toByteArray();
}

/*
 * 引数:state(QByteArray化したState)
 * 戻値:なし
 * 概要:WindowStateを書き込む
 */
void Setting::setState(const QByteArray &state) {
    setting.setValue("Window/state", state);
}


/*
 * 引数:なし
 * 戻値:uint(画面に表示する最大トゥート数)
 * 概要:画面に表示する最大のメッセージ数を返す。これ以上の数の表示は行わず、画面から削除すべき。
 */
unsigned int Setting::getTootLimit() const {
  return setting.value("Window/toot_limit", 64).toUInt();
}

/*
 * 引数:uint(画面に表示する最大トゥート数)
 * 戻値:なし
 * 概要:画面に表示する最大のメッセージ数を書き込む。
 */
void Setting::setTootLimit(unsigned int limit) {
  setting.setValue("Window/toot_limit", limit);
}

/*
 * 引数:なし
 * 戻値:client_id
 * 概要:アプリ登録の際返されるクライアントIDを取得。今の所使ってないけどどっかでいるかも。
 */
QByteArray Setting::getClientId() const {
  return setting.value("Mastodon/client_id").toByteArray();
}

/*
 * 引数:client_id
 * 戻値:なし
 * 概要:アプリ登録の際返されるクライアントIDを保存。
 */
void Setting::setClientId(const QByteArray &id) {
  setting.setValue("Mastodon/client_id", id);
}

/*
 * 引数:なし
 * 戻値:client_secret
 * 概要:アプリ登録の際返されるクライアントシークレットキーを取得。暗号化したい。
 */
QByteArray Setting::getClientSecret() const {
  return setting.value("Mastodon/client_secret").toByteArray();
}

/*
 * 引数:client_secret
 * 戻値:なし
 * 概要:アプリ登録の際返されるクライアントシークレットキーを取得。暗号化したい。
 */
void Setting::setClientSecret(const QByteArray &secret) {
  setting.setValue("Mastodon/client_secret", secret);
}

/*
 * 引数:なし
 * 戻値:access_token
 * 概要:普段使用するアクセストークンを取得。暗号化したい。
 */
QByteArray Setting::getAccessToken() const {
  return setting.value("Mastodon/access_token").toByteArray();
}

/*
 * 引数:access_token
 * 戻値:なし
 * 概要:普段使用するアクセストークンを保存。暗号化したい。
 */
void Setting::setAccessToken(const QByteArray &token) {
  setting.setValue("Mastodon/access_token", token);
}

/*
 * 引数:なし
 * 戻値:対象インスタンスのFQDN
 * 概要:アクセスするインスタンスのドメインを取得する。IDN処理上QStringにしてる
 */
QString Setting::getInstanceDomain() const {
  return setting.value("Mastodon/domain").toString();
}

/*
 * 引数:domain(対象インスタンスのFQDN)
 * 戻値:なし
 * 概要:アクセスするインスタンスのドメインを保存する。
 */
void Setting::setInstanceDomain(const QString &domain) {
  setting.setValue("Mastodon/domain", domain);
}

/*
 * 引数:なし
 * 戻値:QByteArray(アプリ認証をしたユーザのID)
 * 概要:アプリ認証をしたユーザのIDを返す。
 */
QByteArray Setting::getUserId() const {
  return setting.value("Mastodon/user_id").toByteArray();
}

/*
 * 引数:id(アプリ認証をしたユーザのID)
 * 戻値:なし
 * 概要:アプリ認証をしたユーザのIDを保存する。
 */
void Setting::setUserId(const QByteArray &id) {
  setting.setValue("Mastodon/user_id", id);
}

/*
 * 引数:なし
 * 戻値:QString(アプリ認証をしたユーザのUserName)
 * 概要:アプリ認証をしたユーザのUserNameを取得する。
 */
QString Setting::getUserName() const {
  return setting.value("Mastodon/user_name").toString();
}

/*
 * 引数:name(アプリ認証をしたユーザのUserName)
 * 戻値:なし
 * 概要:アプリ認証をしたユーザのUserNameを保存する。
 */
void Setting::setUserName(const QString &name) {
  setting.setValue("Mastodon/user_name", name);
}
