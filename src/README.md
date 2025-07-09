# M5StickCPlus2 MQTTモニター & NTPデジタル時計

このプロジェクトは、一台のM5StickCPlus2を使用して、2つの役割を同時にこなすIoTデバイスの作例です。

1.  **MQTTセンサーモニター:** MQTTブローカーから受信したセンサーデータ（CO2濃度、不快指数など）を本体のカラー液晶画面に表示します。
2.  **NTPデジタル時計:** Groveポートに接続した外部の7セグメントLED（Digi-Clock Unit）に、インターネット経由で取得した正確な日本時刻を「HH:MM」形式で表示します。

-----

## 主な機能

  - **MQTTデータ受信:** 指定したMQTTトピックを購読し、JSON形式のセンサーデータを受信・解析します。
  - **本体LCD表示:** 受信したCO2濃度と不快指数(THI)を、3秒ごとに交互に大きく表示します。
  - **NTP時刻同期:** Wi-Fi接続後、NTPサーバーから正確な時刻を取得し、内部時計を同期させます。
  - **外部時計表示:** Grove接続のDigi-Clock Unitに、同期した時刻をHH:MM形式（24時間表記）で安定して表示します（表示更新は1分ごと）。
  - **ステータス表示:** WiFiやMQTTの接続状態、現在時刻などを本体画面のステータスバーに表示します。
  - **設定の外部化:** Wi-FiのSSIDやパスワード、MQTTブローカー情報などの個人設定を`config.h`に分離しており、安全にコードを共有できます。

-----

## 必要なもの

### ハードウェア

  - M5StickCPlus2
  - M5Stack Digi-Clock Unit
  - Grove - HY2.0 4ピンケーブル

### ソフトウェア

  - **Visual Studio Code (VSCode)**
  - **PlatformIO IDE Extension** (VSCodeにインストール)
  - **Git** (バージョン管理システム)

### PlatformIOで自動的にインストールされるライブラリとフレームワーク:

プロジェクトの`platformio.ini`ファイルに依存関係として記述されます。

  - **Framework:** `arduino` (Arduino Core for ESP32)
  - **Platform:** `espressif32` (ESP32ボードのPlatformIOプラットフォーム)
  - **ライブラリ:**
      - `M5StickCPlus2` (M5Stack公式ライブラリ - GitHub URLで直接指定)
      - `M5Unit-DigiClock` (M5Stack公式ライブラリ - GitHub URLで直接指定)
      - `PubSubClient`
      - `ArduinoJson`
      - `NTPClient`

-----

## セットアップ方法

1.  **ハードウェアの接続:**
    M5StickCPlus2のGroveポート（本体上部の4ピンコネクタ）と、Digi-Clock UnitをGroveケーブルで接続します。

2.  **VSCodeとPlatformIOの準備:**

      - VSCodeをインストールします。
      - VSCodeを起動し、左側のアクティビティバーにある「拡張機能」アイコン（四角が4つ並んだアイコン）をクリックします。
      - 検索窓に `PlatformIO IDE` と入力し、表示された拡張機能をインストールします。
      - PlatformIO Coreのインストールが完了するまで待ちます。（VSCodeの右下に進捗が表示されることがあります）

3.  **新しいPlatformIOプロジェクトの作成:**

      - VSCodeの左側のアクティビティバーにあるPlatformIOのアイコン（アリの絵柄のアイコン）をクリックし、PlatformIO Homeを開きます。
      - 「PROJECTS」または「Quick Access」セクションにある「**+ New Project**」をクリックします。
      - 以下の情報を入力します。
          - **Name:** `M5StickCPlus2_NTP_Clock` (任意のプロジェクト名)
          - **Board:** `ESP32 Dev Module` を選択します。(M5StickCPlus2は現状PlatformIOのボードリストに直接表示されないため、汎用ESP32ボードを使用し、後で設定を調整します。)
          - **Framework:** `Arduino` を選択します。
          - **Location:** デフォルトのままか、任意の保存先を指定します。
      - 「Finish」をクリックしてプロジェクトを作成します。

4.  **`platformio.ini` の設定とライブラリの指定 (最重要):**

      - 作成されたプロジェクトフォルダ内の `platformio.ini` ファイルを開き、以下のように内容を修正・追記します。
      - 特に `board` のIDをM5StickCPlus2の内部IDである `m5stickc_plus2` に変更し、PlatformIOがGitHubからライブラリを直接取得するように指定します。

    <!-- end list -->

    ```ini
    [env:m5stick-c-plus2] ; 環境名は任意で変更可能
    platform = espressif32
    board = m5stickc_plus2  ; ★ M5StickCPlus2のボードIDを直接指定
    framework = arduino
    monitor_speed = 115200 ; シリアルモニタのボーレートを設定（スケッチのSerial.begin()に合わせてください）

    lib_deps =
        https://github.com/m5stack/M5StickCPlus2.git ; M5StickCPlus2ライブラリをGitHubから直接取得
        https://github.com/m5stack/M5Unit-DigiClock.git ; Digi-Clock UnitライブラリをGitHubから直接取得
        knolleary/PubSubClient@^2.8
        bblanchon/ArduinoJson@^6.19.0
        arduino-libraries/NTPClient@^3.2.1
    ```

      - ファイルを保存します。PlatformIOが自動的に依存関係の解決（ライブラリのダウンロードなど）を開始します。

5.  **ソースコードの移行:**

      - プロジェクトフォルダ内の `src` フォルダに、あなたの既存の`.ino`ファイルと`config.example.h`をコピーします。
      - `.ino`ファイルは、**`main.cpp`** という名前にリネームすることをお勧めします。

6.  **設定ファイルの準備 (最重要):**

      - `src` フォルダにコピーした `config.example.h` をコピー＆ペーストして、複製したファイルの名前を **`config.h`** に変更します。
      - 新しく作成した`config.h`をエディタで開き、ご自身の環境に合わせて\*\*`"YOUR_..."`\*\*の部分を全て書き換えてください。
      - **注意:** `config.h`ファイルは、`.gitignore`によってGitの管理対象から除外されています。ご自身のパスワードなどの情報が誤ってGitHubに公開されることはありません。Gitの追跡から確実に除外するには、`git rm --cached src/config.h` を実行後、変更をコミットしてください。

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

-----

## 使い方

1.  上記の手順に従って、ハードウェアの接続とソフトウェア・設定ファイルの準備を完了させます。
2.  VSCodeでプロジェクトフォルダを開きます。
3.  VSCodeの下部ステータスバーにある**チェックマークアイコン**（Build）をクリックしてプロジェクトをビルドします。
4.  ビルドが成功したら、M5StickCPlus2をUSBケーブルでPCに接続します。
5.  VSCodeの左側のアクティビティバーにあるPlatformIOアイコン（アリの絵柄）をクリックし、「PROJECT TASKS」セクションの「**Upload**」をクリックしてスケッチをM5StickCPlus2に書き込みます。
6.  起動後、デバイスは自動的にWi-Fiに接続し、時刻同期とMQTTブローカへの接続を開始します。成功すれば、本体画面とDigi-Clock Unitの両方が機能し始めます。

-----

## ライセンス

このプロジェクトは[MITライセンス](https://www.google.com/search?q=LICENSE)の下で公開されています。