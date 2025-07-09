/**
 * @file m5stick_mqtt_digiclock.ino
 * @brief M5StickCPlus2とDigi-Clock Unitを使った、MQTTセンサーモニター兼NTPデジタル時計
 *
 * @details
 * このプログラムは、2つの役割を同時にこなします。
 * 1. M5StickCPlus2本体のカラー液晶画面に、MQTTプロトコルで受信したセンサー情報（CO2濃度など）を表示します。
 * 2. Groveポートに接続した外部の7セグメントLED（Digi-Clock Unit）に、インターネット経由で取得した
 * 正確な日本時刻を「HH:MM」形式（24時間表記）で安定して表示します。
 *
 * Wi-Fiの接続情報などは、プライバシー保護のため、同じフォルダにある `config.h` という別のファイルに記述されています。
 *
 * @author Your Name
 * @date 2025-07-09
 */

// =================================================================
// 1. ライブラリと設定ファイルの読み込み
// =================================================================
// 機能を拡張するための「ライブラリ」と呼ばれる、便利なプログラム部品を読み込みます。

#include <M5StickCPlus2.h>     // M5StickCPlus2本体の機能（画面、電源など）を簡単に使うためのライブラリ
#include <WiFi.h>              // Wi-Fi機能を使うためのライブラリ
#include <PubSubClient.h>      // MQTTという通信方法を使うためのライブラリ
#include <ArduinoJson.h>       // JSONというデータ形式を簡単に扱うためのライブラリ
#include <NTPClient.h>         // NTPという、時刻を合わせるための通信方法を使うライブラリ
#include <WiFiUdp.h>           // NTP通信の基礎となるUDP通信を使うためのライブラリ
#include "config.h"            // Wi-FiやMQTTの接続情報など、個人情報を記述した設定ファイルを読み込みます
#include <M5UNIT_DIGI_CLOCK.h> // M5Stackの「Digi-Clock Unit」を制御するための専用ライブラリ

// =================================================================
// 2. データ構造体の定義
// =================================================================
/**
 * @brief MQTTで受信するセンサーデータをまとめて管理するための「設計図」
 */
struct SensorDataPacket
{
  int carbonDioxideLevel;
  float thermalComfortIndex;
  float ambientTemperature;
  float relativeHumidity;
  String comfortLevelDescription;
  unsigned long dataTimestamp;
  bool hasValidData;
};

// =================================================================
// 3. グローバル変数の定義
// =================================================================
// プログラムの様々な場所から参照・変更される変数を「グローバル変数」としてここで宣言します。

// --- ネットワーク関連 ---
WiFiUDP networkUdpClient;
NTPClient timeClient(networkUdpClient, TIME_SERVER_ADDRESS, JAPAN_TIME_OFFSET_SECONDS, TIME_UPDATE_INTERVAL_MILLISECONDS);
WiFiClient networkWifiClient;
PubSubClient mqttCommunicationClient(networkWifiClient);

// --- センサーデータ関連 ---
SensorDataPacket currentSensorReading = {0, 0.0, 0.0, 0.0, "", 0, false};

// --- 表示制御関連 ---
unsigned long lastDisplayUpdateTime = 0;
unsigned long lastInteractiveDisplayTime = 0;
bool displayCO2 = true;

// --- Digi-Clock Unit 関連 ---
M5UNIT_DIGI_CLOCK digi_clock;
int last_digiclock_minute = -1; // 最後にDigi-Clockに表示した「分」を記憶する変数（チラツキ防止用）

// =================================================================
// 4. 関数の前方宣言
// =================================================================
// C++では、関数は呼び出される前に「こういう名前の関数がありますよ」と宣言しておく必要があります。

void initializeDisplaySystem();
void showSystemStartupMessage();
void establishWiFiConnection();
bool checkWiFiConnectionStatus();
void displayWiFiConnectionSuccess();
void synchronizeSystemTimeWithNTP();
bool attemptNTPTimeSynchronization();
void displayNTPSynchronizationResult(bool wasSuccessful);
void configureMQTTConnection();
void establishMQTTBrokerConnection();
String generateUniqueMQTTClientId();
bool attemptMQTTBrokerConnection(const String &clientIdentifier);
void subscribeToMQTTDataTopic();
void displayMQTTConnectionSuccess();
void displayMQTTConnectionFailure();
void handleIncomingMQTTMessage(char *topicName, byte *messagePayload, unsigned int messageLength);
bool validateJSONDataIntegrity(const String &jsonData);
String convertRawPayloadToString(byte *rawPayload, unsigned int payloadLength);
SensorDataPacket parseJSONSensorData(const String &jsonString);
void updateCurrentSensorData(const SensorDataPacket &newSensorData);
void maintainMQTTBrokerConnection();
void processIncomingMQTTMessages();
void updateDisplayIfIntervalElapsed();
void updateSystemNetworkTime();
void refreshEntireDisplay();
void displayApplicationTitle();
void displayCurrentSystemTime();
void displaySensorDataOrErrorMessage();
void displayCO2ConcentrationData();
void displayTHIComfortData();
void displayNoDataAvailableMessage();
void displayNetworkConnectionStatus();
void displayJSONParsingError(const char *errorDescription);
void showConnectionStatusMessage(const char *statusMessage);
void clearDisplayScreenWithColor(uint16_t backgroundColor);
void printMQTTSubscriptionDebugInfo();
void initializeDigiClock();
void updateDigiClockDisplay();

// =================================================================
// 5. メインの初期化関数 (setup)
// =================================================================
/**
 * @brief セットアップ関数。マイコンの電源が入った時に一度だけ実行されます。
 * @details 各種ハードウェアや通信機能の初期化を順番に行います。
 */
void setup()
{
  // PCとの通信（シリアルモニタ）を開始。デバッグ情報の表示に使う。
  Serial.begin(115200);
  Serial.println("\n========== M5StickCPlus2 & Digi-Clock Monitor 起動 ==========");

  // Step 1: M5StickCPlus2本体のディスプレイを初期化
  initializeDisplaySystem();
  showSystemStartupMessage();

  // Step 2: 外部接続したDigi-Clock Unitを初期化
  initializeDigiClock();

  // Step 3: Wi-Fiネットワークへの接続
  establishWiFiConnection();

  // Step 4: インターネット上の時刻サーバーと時刻を同期
  synchronizeSystemTimeWithNTP();

  // Step 5: MQTT通信の準備とサーバーへの接続
  configureMQTTConnection();
  establishMQTTBrokerConnection();

  // Step 6: 全ての準備が整ったので、メインの表示画面を描画
  refreshEntireDisplay();

  Serial.println("========== 初期化処理完了：システム稼働開始 ==========");
}

// =================================================================
// 6. メインのループ関数 (loop)
// =================================================================
/**
 * @brief メインループ関数。setup()の実行後、電源が切れるまでずっと繰り返し実行されます。
 */
void loop()
{
  // 1. MQTTサーバーとの接続が切れていないか確認し、切れていたら再接続する
  maintainMQTTBrokerConnection();

  // 2. MQTTサーバーから新しいメッセージが届いていないか確認し、届いていれば処理する
  processIncomingMQTTMessages();

  // 3. M5StickCPlus2本体の画面を、一定時間ごとに更新する（CO2とTHIの交互表示）
  updateDisplayIfIntervalElapsed();

  // 4. NTP時刻を、内部で定期的に更新する
  updateSystemNetworkTime();

  // 5. Digi-Clock Unitの時刻表示を、必要に応じて更新する
  updateDigiClockDisplay();

  // 6. 次のループまで少し待機する（CPUを少し休ませて、消費電力を抑える）
  delay(MAIN_LOOP_DELAY_MILLISECONDS); // (この値はconfig.hで定義)
}

// =================================================================
// 7. 各関数の具体的な処理内容
// =================================================================

// -----------------------------------------------------------------
// Digi-Clock Unit関連の関数
// -----------------------------------------------------------------

/**
 * @brief Digi-Clock Unitを初期化する
 */
void initializeDigiClock()
{
  // M5StickCPlus2のGroveポート(G32, G33)でI2C通信を開始
  Wire.begin(32, 33);
  Serial.println("⚙️  I2C for Digi-Clock Unit starting...");

  // ユニットの初期化を試み、失敗した場合はエラーメッセージを表示する
  if (!digi_clock.begin(&Wire))
  {
    Serial.println("❌ Digi-Clock Unit not found!");
    // 本体画面にもエラーを表示
    M5.Display.setCursor(10, 50);
    M5.Display.setTextColor(RED);
    M5.Display.println("DigiClock ERR");
    delay(2000); // 2秒間エラーを表示
  }
  else
  {
    Serial.println("✅ Digi-Clock Unit found and initialized.");
    digi_clock.setBrightness(80); // 明るさを設定 (0-100)
    digi_clock.setString("----"); // 起動時はハイフンを表示しておく
  }
}

/**
 * @brief Digi-Clock Unitの時刻表示を更新する（チラツキ防止・安定版）
 */
void updateDigiClockDisplay()
{
  // NTPで時刻が正しく同期されている場合のみ、処理を実行
  if (timeClient.getEpochTime() > 1672531200)
  { // 2023年以降の時刻ならOK

    int minute = timeClient.getMinutes();

    // 「分」が変わった時にだけ、ディスプレイの表示を更新する
    if (minute != last_digiclock_minute)
    {
      int hour = timeClient.getHours();
      char time_string[6];

      // HH:MM形式でコロンを常時点灯させる
      sprintf(time_string, "%02d:%02d", hour, minute);
      digi_clock.setString(time_string);

      // 更新した「分」の値を記憶しておく
      last_digiclock_minute = minute;
    }
  }
}

// -----------------------------------------------------------------
// M5StickCPlus2 本体画面関連の関数
// -----------------------------------------------------------------

void initializeDisplaySystem()
{
  M5.begin();
  M5.Display.setRotation(1);
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setTextColor(WHITE);
  M5.Display.setTextSize(2);
  Serial.println("✅ M5StickCPlus2 Display Initialized.");
}

void showSystemStartupMessage()
{
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  M5.Display.println("Starting...");
  Serial.println("📱 Displaying startup message.");
}

void refreshEntireDisplay()
{
  clearDisplayScreenWithColor(BLACK);
  displayApplicationTitle();
  displayCurrentSystemTime();
  displayNetworkConnectionStatus();
  if (currentSensorReading.hasValidData)
  {
    if (displayCO2)
    {
      displayCO2ConcentrationData();
    }
    else
    {
      displayTHIComfortData();
    }
  }
  else
  {
    displayNoDataAvailableMessage();
  }
}

void updateDisplayIfIntervalElapsed()
{
  unsigned long currentSystemTime = millis();
  if (currentSystemTime - lastInteractiveDisplayTime >= INTERACTIVE_DISPLAY_INTERVAL_MILLISECONDS)
  {
    clearDisplayScreenWithColor(BLACK);
    displayApplicationTitle();
    displayCurrentSystemTime();
    displayNetworkConnectionStatus();
    if (currentSensorReading.hasValidData)
    {
      if (displayCO2)
      {
        displayCO2ConcentrationData();
      }
      else
      {
        displayTHIComfortData();
      }
      displayCO2 = !displayCO2;
    }
    else
    {
      displayNoDataAvailableMessage();
    }
    lastInteractiveDisplayTime = currentSystemTime;
  }
}

void displayApplicationTitle()
{
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(CYAN);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  M5.Display.println("Sensor Monitor");
}

void displayCurrentSystemTime()
{
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(TIME_DISPLAY_X, TIME_DISPLAY_Y);
  M5.Display.println(timeClient.getFormattedTime());
}

void displayNetworkConnectionStatus()
{
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(mqttCommunicationClient.connected() ? GREEN : RED);
  M5.Display.setCursor(CONNECTION_STATUS_X, CONNECTION_STATUS_Y);
  M5.Display.println(mqttCommunicationClient.connected() ? "MQTT:OK" : "MQTT:NG");
}

void displayCO2ConcentrationData()
{
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(GREEN);
  M5.Display.setCursor(LARGE_LABEL_X, LARGE_LABEL_Y);
  M5.Display.println("CO2:");
  M5.Display.setTextSize(8);
  M5.Display.setTextColor(GREEN);
  M5.Display.setTextDatum(TR_DATUM);
  String co2Value = String(currentSensorReading.carbonDioxideLevel);
  M5.Display.drawString(co2Value, M5.Display.width() - DISPLAY_RIGHT_MARGIN, LARGE_VALUE_Y);
  M5.Display.setTextDatum(TL_DATUM);
}

void displayTHIComfortData()
{
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(ORANGE);
  M5.Display.setCursor(LARGE_LABEL_X, LARGE_LABEL_Y);
  M5.Display.println("THI:");
  M5.Display.setTextSize(8);
  M5.Display.setTextColor(ORANGE);
  M5.Display.setTextDatum(TR_DATUM);
  String thiValue = String(currentSensorReading.thermalComfortIndex, 1);
  M5.Display.drawString(thiValue, M5.Display.width() - DISPLAY_RIGHT_MARGIN, LARGE_VALUE_Y);
  M5.Display.setTextDatum(TL_DATUM);
}

void displayNoDataAvailableMessage()
{
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(RED);
  M5.Display.setCursor(NO_DATA_MESSAGE_X, NO_DATA_MESSAGE_Y);
  M5.Display.println("No Data");
}

void displayJSONParsingError(const char *errorDescription)
{
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(CYAN);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  M5.Display.println("Sensor Monitor");
  M5.Display.setTextColor(WHITE);
  M5.Display.setCursor(TIME_DISPLAY_X, TIME_DISPLAY_Y);
  M5.Display.println(timeClient.getFormattedTime());
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(mqttCommunicationClient.connected() ? GREEN : RED);
  M5.Display.setCursor(CONNECTION_STATUS_X, CONNECTION_STATUS_Y);
  M5.Display.println(mqttCommunicationClient.connected() ? "MQTT:OK" : "MQTT:NG");
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(RED);
  M5.Display.setCursor(20, 50 + VERTICAL_OFFSET);
  M5.Display.println("JSON Error");
  M5.Display.setTextSize(1);
  M5.Display.setCursor(20, 80 + VERTICAL_OFFSET);
  M5.Display.println(errorDescription);
}

// -----------------------------------------------------------------
// ネットワーク関連の関数
// -----------------------------------------------------------------

void establishWiFiConnection()
{
  Serial.println("🌐 Attempting to connect to WiFi...");
  showConnectionStatusMessage("WiFi connecting...");
  WiFi.begin(WIFI_NETWORK_NAME, WIFI_NETWORK_PASSWORD);
  while (!checkWiFiConnectionStatus())
  {
    delay(500);
    M5.Display.print(".");
    Serial.print(".");
  }
  displayWiFiConnectionSuccess();
  Serial.println("\n✅ WiFi Connection Successful.");
  Serial.print("   IP Address: ");
  Serial.println(WiFi.localIP());
}

bool checkWiFiConnectionStatus()
{
  return WiFi.status() == WL_CONNECTED;
}

void displayWiFiConnectionSuccess()
{
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  M5.Display.println("WiFi Connected!");
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y + 20);
  M5.Display.println(WiFi.localIP());
  delay(CONNECTION_SUCCESS_DISPLAY_TIME);
}

void synchronizeSystemTimeWithNTP()
{
  Serial.println("🕐 Starting NTP time synchronization...");
  showConnectionStatusMessage("NTP Sync...");
  timeClient.begin();
  bool synchronizationSuccess = attemptNTPTimeSynchronization();
  displayNTPSynchronizationResult(synchronizationSuccess);
}

bool attemptNTPTimeSynchronization()
{
  for (int i = 0; i < MAXIMUM_NTP_RETRY_ATTEMPTS; i++)
  {
    if (timeClient.update())
    {
      Serial.println("✅ NTP Time Synced Successfully.");
      return true;
    }
    timeClient.forceUpdate();
    delay(1000);
    M5.Display.print(".");
    Serial.print(".");
  }
  Serial.println("\n❌ NTP Time Sync Failed.");
  return false;
}

void displayNTPSynchronizationResult(bool wasSuccessful)
{
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  if (wasSuccessful)
  {
    M5.Display.println("NTP Synced!");
    M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y + 20);
    M5.Display.println(timeClient.getFormattedTime());
    Serial.print("   Synced Time: ");
    Serial.println(timeClient.getFormattedTime());
  }
  else
  {
    M5.Display.println("NTP Failed!");
  }
  delay(CONNECTION_SUCCESS_DISPLAY_TIME);
}

void configureMQTTConnection()
{
  mqttCommunicationClient.setServer(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
  mqttCommunicationClient.setCallback(handleIncomingMQTTMessage);
  Serial.println("⚙️ MQTT Connection Configured.");
}

void establishMQTTBrokerConnection()
{
  Serial.println("📡 Attempting to connect to MQTT broker...");
  showConnectionStatusMessage("MQTT connecting...");
  while (!mqttCommunicationClient.connected())
  {
    String uniqueClientId = generateUniqueMQTTClientId();
    if (attemptMQTTBrokerConnection(uniqueClientId))
    {
      subscribeToMQTTDataTopic();
      displayMQTTConnectionSuccess();
      break;
    }
    else
    {
      displayMQTTConnectionFailure();
    }
  }
}

String generateUniqueMQTTClientId()
{
  return String(MQTT_CLIENT_ID_PREFIX) + String(random(0xffff), HEX);
}

bool attemptMQTTBrokerConnection(const String &clientIdentifier)
{
  bool connectionEstablished = mqttCommunicationClient.connect(clientIdentifier.c_str());
  if (connectionEstablished)
  {
    Serial.println("✅ MQTT Connection Successful.");
    Serial.print("   Client ID: ");
    Serial.println(clientIdentifier);
  }
  else
  {
    Serial.print("❌ MQTT Connection Failed, rc=");
    Serial.println(mqttCommunicationClient.state());
  }
  return connectionEstablished;
}

void subscribeToMQTTDataTopic()
{
  mqttCommunicationClient.subscribe(MQTT_TOPIC_NAME);
  Serial.print("📬 Subscribed to MQTT topic: ");
  Serial.println(MQTT_TOPIC_NAME);
}

void displayMQTTConnectionSuccess()
{
  M5.Display.println("MQTT Connected!");
  delay(1000);
}

void displayMQTTConnectionFailure()
{
  M5.Display.print("Failed, rc=");
  M5.Display.print(mqttCommunicationClient.state());
  M5.Display.println(" retry in 5s");
  delay(MQTT_RECONNECTION_DELAY_MILLISECONDS);
}

// -----------------------------------------------------------------
// MQTT データ処理関連の関数
// -----------------------------------------------------------------

void handleIncomingMQTTMessage(char *topicName, byte *messagePayload, unsigned int messageLength)
{
  String jsonMessageString = convertRawPayloadToString(messagePayload, messageLength);
  Serial.println("\n--- New MQTT Message Received ---");
  Serial.printf("Topic: %s\n", topicName);
  Serial.printf("Payload: '%s'\n", jsonMessageString.c_str());
  if (!validateJSONDataIntegrity(jsonMessageString))
  {
    Serial.println("❌ Invalid JSON data detected.");
    displayJSONParsingError("Invalid JSON");
    return;
  }
  SensorDataPacket parsedSensorData = parseJSONSensorData(jsonMessageString);
  if (parsedSensorData.hasValidData)
  {
    updateCurrentSensorData(parsedSensorData);
    Serial.printf("✅ Sensor data updated: CO2=%d, THI=%.1f\n", parsedSensorData.carbonDioxideLevel, parsedSensorData.thermalComfortIndex);
    refreshEntireDisplay();
  }
  else
  {
    Serial.println("❌ Sensor data parsing failed.");
    displayJSONParsingError("Parse Failed");
  }
  Serial.println("---------------------------------");
}

bool validateJSONDataIntegrity(const String &jsonData)
{
  String trimmedData = jsonData;
  trimmedData.trim();
  if (trimmedData.length() == 0)
    return false;
  if (!trimmedData.startsWith("{"))
    return false;
  if (!trimmedData.endsWith("}"))
    return false;
  return true;
}

String convertRawPayloadToString(byte *rawPayload, unsigned int payloadLength)
{
  String convertedMessage;
  convertedMessage.reserve(payloadLength + 1);
  for (unsigned int i = 0; i < payloadLength; i++)
  {
    if (rawPayload[i] >= 32 && rawPayload[i] <= 126)
    {
      convertedMessage += (char)rawPayload[i];
    }
  }
  return convertedMessage;
}

SensorDataPacket parseJSONSensorData(const String &jsonString)
{
  SensorDataPacket extractedData = {0, 0.0, 0.0, 0.0, "", 0, false};
  DynamicJsonDocument jsonDocument(JSON_PARSING_MEMORY_SIZE);
  DeserializationError parseError = deserializeJson(jsonDocument, jsonString);
  if (parseError)
  {
    Serial.printf("❌ JSON parsing failed: %s\n", parseError.c_str());
    return extractedData;
  }
  if (jsonDocument.containsKey("co2"))
    extractedData.carbonDioxideLevel = jsonDocument["co2"];
  if (jsonDocument.containsKey("thi"))
    extractedData.thermalComfortIndex = jsonDocument["thi"];
  if (jsonDocument.containsKey("temperature"))
    extractedData.ambientTemperature = jsonDocument["temperature"];
  if (jsonDocument.containsKey("humidity"))
    extractedData.relativeHumidity = jsonDocument["humidity"];
  if (jsonDocument.containsKey("comfort_level"))
    extractedData.comfortLevelDescription = jsonDocument["comfort_level"].as<String>();
  if (jsonDocument.containsKey("timestamp"))
    extractedData.dataTimestamp = jsonDocument["timestamp"];
  extractedData.hasValidData = true;
  return extractedData;
}

void updateCurrentSensorData(const SensorDataPacket &newSensorData)
{
  currentSensorReading = newSensorData;
}

void maintainMQTTBrokerConnection()
{
  if (!mqttCommunicationClient.connected())
  {
    Serial.println("⚠️ MQTT connection lost. Reconnecting...");
    establishMQTTBrokerConnection();
  }
}

void processIncomingMQTTMessages()
{
  mqttCommunicationClient.loop();
}

void updateSystemNetworkTime()
{
  timeClient.update();
}

// -----------------------------------------------------------------
// ユーティリティ関数
// -----------------------------------------------------------------

void showConnectionStatusMessage(const char *statusMessage)
{
  clearDisplayScreenWithColor(BLACK);
  M5.Display.setCursor(TITLE_POSITION_X, TITLE_POSITION_Y);
  M5.Display.println(statusMessage);
}

void clearDisplayScreenWithColor(uint16_t backgroundColor)
{
  M5.Display.fillScreen(backgroundColor);
}

void printMQTTSubscriptionDebugInfo()
{
  Serial.println("--- MQTT Subscription Status ---");
  Serial.printf("Broker: %s:%d\n", MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
  Serial.printf("Topic: %s\n", MQTT_TOPIC_NAME);
  Serial.printf("Connected: %s\n", mqttCommunicationClient.connected() ? "Yes" : "No");
  Serial.printf("Client State Code: %d\n", mqttCommunicationClient.state());
  Serial.println("------------------------------");
}

void displaySensorDataOrErrorMessage()
{ /* Redundant with new logic, can be removed if desired */
}