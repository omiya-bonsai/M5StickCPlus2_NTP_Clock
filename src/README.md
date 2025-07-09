# M5StickCPlus2 MQTTモニター & NTPデジタル時計

このプロジェクトは、一台のM5StickCPlus2を使用して、2つの役割を同時にこなすIoTデバイスの作例です。

1.  **MQTTセンサーモニター:** MQTTブローカーから受信したセンサーデータ（CO2濃度、不快指数など）を本体のカラー液晶画面に表示します。
2.  **NTPデジタル時計:** Groveポートに接続した外部の7セグメントLED（Digi-Clock Unit）に、インターネット経由で取得した正確な日本時刻を「HH:MM」形式で表示します。

---

## 主な機能

- **MQTTデータ受信:** 指定したMQTTトピックを購読し、JSON形式のセンサーデータを受信・解析します。
- **本体LCD表示:** 受信したCO2濃度と不快指数(THI)を、3秒ごとに交互に大きく表示します。
- **NTP時刻同期:** Wi-Fi接続後、NTPサーバーから正確な時刻を取得し、内部時計を同期させます。
- **外部時計表示:** Grove接続のDigi-Clock Unitに、同期した時刻をHH:MM形式（24時間表記）で安定して表示します（表示更新は1分ごと）。
- **ステータス表示:** WiFiやMQTTの接続状態、現在時刻などを本体画面のステータスバーに表示します。
- **設定の外部化:** Wi-FiのSSIDやパスワード、MQTTブローカー情報などの個人設定を`config.h`に分離しており、安全にコードを共有できます。

---

## 必要なもの

### ハードウェア
- M5StickCPlus2
- M5Stack Digi-Clock Unit
- Grove - HY2.0 4ピンケーブル

### ソフトウェア
- Arduino IDE (バージョン 2.x を推奨)
- M5Stackボード定義 (Arduino IDEのボードマネージャからインストール)
- 以下のArduinoライブラリ:
  - `M5StickCPlus2` (M5Stackボード定義に含まれます)
  - `PubSubClient` (ライブラリマネージャからインストール)
  - `ArduinoJson` (ライブラリマネージャからインストール)
  - `NTPClient` (ライブラリマネージャからインストール)
  - `M5Unit-DigiClock` (M5Stack公式。ライブラリマネージャで見つからない場合はZIPでインストール)

---

## セットアップ方法

1.  **ハードウェアの接続:**
    M5StickCPlus2のGroveポート（本体上部の4ピンコネクタ）と、Digi-Clock UnitをGroveケーブルで接続します。

2.  **Arduino IDEの準備:**
    - Arduino IDEをインストールします。
    - ボードマネージャから「M5Stack」で検索し、M5Stackのボード定義をインストールします。
    - ライブラリマネージャから、上記の必要なライブラリを全てインストールします。

3.  **設定ファイルの準備 (最重要):**
    - このプロジェクトのフォルダに、`config.example.h`というファイルが含まれています。
    - そのファイルをコピー＆ペーストして、複製したファイルの名前を **`config.h`** に変更します。
    - 新しく作成した`config.h`をエディタで開き、ご自身の環境に合わせて**`"YOUR_..."`**の部分を全て書き換えてください。
    - **注意:** `config.h`ファイルは、`.gitignore`によってGitの管理対象から除外されています。ご自身のパスワードなどの情報が誤ってGitHubに公開されることはありません。

    **`config.h` の設定例:**
    ```cpp
    #ifndef CONFIG_H
    #define CONFIG_H

    // ========== WiFi設定 ==========
    #define WIFI_NETWORK_NAME "YOUR_WIFI_SSID"      // ご自身のWiFiのSSID
    #define WIFI_NETWORK_PASSWORD "YOUR_WIFI_PASSWORD"  // ご自身のWiFiのパスワード

    // ========== MQTTブローカ設定 ==========
    #define MQTT_BROKER_ADDRESS "192.168.1.100" // MQTTブローカのIPアドレスまたはホスト名
    #define MQTT_BROKER_PORT 1883               // MQTTブローカのポート番号
    #define MQTT_TOPIC_NAME "sensors/livingroom"// 購読するトピック名
    #define MQTT_CLIENT_ID_PREFIX "m5stick-monitor" // MQTTクライアントIDの接頭辞

    // ========== NTP時刻同期設定 ==========
    #define TIME_SERVER_ADDRESS "ntp.nict.jp"     // 日本の公式NTPサーバー
    #define JAPAN_TIME_OFFSET_SECONDS (9 * 3600)    // 日本標準時(JST)の時差（9時間）
    #define TIME_UPDATE_INTERVAL_MILLISECONDS (60 * 60 * 1000) // 時刻を再同期する間隔（ミリ秒）。これは1時間。

    // ========== ディスプレイレイアウト設定 ==========
    #define TITLE_POSITION_X 5
    #define TITLE_POSITION_Y 5
    #define TIME_DISPLAY_X 180
    #define TIME_DISPLAY_Y 5
    #define CONNECTION_STATUS_X 170
    #define CONNECTION_STATUS_Y 15
    #define LARGE_LABEL_X 10
    #define LARGE_LABEL_Y 30
    #define LARGE_VALUE_Y 45
    #define NO_DATA_MESSAGE_X 40
    #define NO_DATA_MESSAGE_Y 60
    #define DISPLAY_RIGHT_MARGIN 10
    #define VERTICAL_OFFSET 0

    // ========== タイミング設定 ==========
    #define CONNECTION_SUCCESS_DISPLAY_TIME 2000 // 接続成功メッセージの表示時間 (2秒)
    #define MAXIMUM_NTP_RETRY_ATTEMPTS 5       // NTP同期の最大リトライ回数
    #define MQTT_RECONNECTION_DELAY_MILLISECONDS 5000 // MQTT再接続までの待機時間 (5秒)
    #define INTERACTIVE_DISPLAY_INTERVAL_MILLISECONDS 3000 // CO2/THI交互表示の切り替え間隔 (3秒)
    #define MAIN_LOOP_DELAY_MILLISECONDS 100 // メインループの待機時間 (0.1秒)

    // ========== その他設定 ==========
    #define JSON_PARSING_MEMORY_SIZE 1024 // JSON解析に割り当てるメモリサイズ

    #endif // CONFIG_H
    ```

---

## 使い方

1.  上記の手順に従って、ハードウェアの接続とソフトウェア・設定ファイルの準備を完了させます。
2.  Arduino IDEで`m5stick_mqtt_digiclock.ino`（メインスケッチ）を開きます。
3.  ボードとして「M5StickCPlus2」を選択し、正しいシリアルポートを指定します。
4.  スケッチをM5StickCPlus2に書き込みます。
5.  起動後、デバイスは自動的にWi-Fiに接続し、時刻同期とMQTTブローカへの接続を開始します。成功すれば、本体画面とDigi-Clock Unitの両方が機能し始めます。

---

## ライセンス

このプロジェクトは[MITライセンス](LICENSE)の下で公開されています。