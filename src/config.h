#ifndef CONFIG_H
#define CONFIG_H

// ================================================================================
// M5StickCPlus2 センサーモニター 設定ファイル
// 匿名性、再利用性向上のため、ネットワーク・MQTT関連設定をここに集約
// ================================================================================

// ========== ネットワーク設定 ==========
const char* WIFI_NETWORK_NAME = "A0957FA4E825-2G";     // 接続するWiFiのSSID（ネットワーク名）
const char* WIFI_NETWORK_PASSWORD = "6fh62nh25h72xc";  // WiFiのパスワード

// ========== MQTT設定 ==========
const char* MQTT_BROKER_ADDRESS = "192.168.3.82";      // MQTTブローカ（サーバ）のIPアドレス
const char* MQTT_TOPIC_NAME = "sensor_data";           // 購読するトピック名（データのカテゴリ）
const int MQTT_BROKER_PORT = 1883;                     // MQTTブローカのポート番号（標準は1883）
const char* MQTT_CLIENT_ID_PREFIX = "M5StickCPlus2-";  // MQTT接続時のクライアントID接頭辞

// ========== 時刻同期設定 ==========
const char* TIME_SERVER_ADDRESS = "pool.ntp.org";               // NTPサーバのアドレス
const long JAPAN_TIME_OFFSET_SECONDS = 32400;                   // 日本時間のオフセット（+9時間を秒換算）
const unsigned long TIME_UPDATE_INTERVAL_MILLISECONDS = 60000;  // 時刻更新間隔（1分）

// ========== 表示更新設定 ==========
// 画面更新間隔は全体の表示更新に使い、CO2/THI交互表示は別の変数で制御します
const unsigned long MAIN_LOOP_DELAY_MILLISECONDS = 100;  // メインループの待機時間

// ========== 画面表示位置の設定 ==========
// 画面サイズ：240x135 ピクセル

// 共通位置調整：画面全体の垂直方向を微調整
const int VERTICAL_OFFSET = 5;  // Y座標全体を下に5ピクセルずらす (微調整用)

// タイトル、時刻、接続ステータス (上部の表示)
const int TITLE_POSITION_X = 5;                       // タイトル表示のX座標 (左上)
const int TITLE_POSITION_Y = 2 + VERTICAL_OFFSET;     // タイトル表示のY座標 (左上)
const int TIME_DISPLAY_X = 140;                       // 時刻表示のX座標 (左上)
const int TIME_DISPLAY_Y = 2 + VERTICAL_OFFSET;       // 時刻表示のY座標 (左上)
const int CONNECTION_STATUS_X = 190;                  // 接続状態表示のX座標 (左上)
const int CONNECTION_STATUS_Y = 2 + VERTICAL_OFFSET;  // 接続状態表示のY座標 (左上)

// CO2/THIメイン数値表示
// フォントサイズ7の数値の高さは、約42ピクセル (M5GFXのフォントサイズ換算に基づく概算)
// ラベル(サイズ2)の高さは約16ピクセル
const int LARGE_LABEL_X = 15;                    // ラベルのX座標 (左寄せ)
const int LARGE_LABEL_Y = 30 + VERTICAL_OFFSET;  // ラベルのY座標 (中央上部を意識)

// 【修正】数値表示のY座標 (TR_DATUM基準)
// LARGE_VALUE_Y は数値の「上端」のY座標になります。
// 画面高さ135pxに対して、ラベルの下に適切なマージンを開けつつ、
// 画面中央よりやや上に配置されるよう調整します。
const int LARGE_VALUE_Y = 50 + VERTICAL_OFFSET;  // 数値の上端がこの位置に来るように調整

// 【修正】右揃え時の余白
const int DISPLAY_RIGHT_MARGIN = 15;  // ディスプレイ右端からの余白（ピクセル）

// エラー/メッセージ表示 (位置調整)
const int NO_DATA_MESSAGE_X = 40;                    // データなしメッセージのX座標
const int NO_DATA_MESSAGE_Y = 55 + VERTICAL_OFFSET;  // データなしメッセージのY座標


// ========== 再試行・タイムアウト設定 ==========
const int MAXIMUM_NTP_RETRY_ATTEMPTS = 10;                        // NTP同期の最大試行回数
const unsigned long MQTT_RECONNECTION_DELAY_MILLISECONDS = 5000;  // MQTT再接続待機時間（5秒）
const unsigned long CONNECTION_SUCCESS_DISPLAY_TIME = 2000;       // 接続成功メッセージ表示時間（2秒）

// ========== JSON解析設定 ==========
const size_t JSON_PARSING_MEMORY_SIZE = 2048;  // JSON解析用メモリサイズ（バイト）

// ========== 新規追加：交互表示のための設定 ==========
const unsigned long INTERACTIVE_DISPLAY_INTERVAL_MILLISECONDS = 3000;  // 3秒ごとに交互表示

#endif  // CONFIG_H