/*******************************************************************************
 *  File   : Sol.ino
 *  Update : 2025/04/17
 *  Bord   : M5Stack CoreS3 SE
 *  Lib    : -
 *  Overview : 
 *******************************************************************************/

/*******************************************************************************
 * User defined constant/variables definition
 *******************************************************************************/

/* ------------------------ *
 *   Macro definition
 * ------------------------ */
  // M5stack coreS3 : DOUT
  #define P_SOL1      5         // G5 : GPIO
  #define P_SOL2      6         // G6 : GPIO
  #define P_SOL3      7         // G7 : GPIO
  // #define P_SOL4  10         // G10: ADC (予備)

  #define OPEN        1         // 電磁弁：開
  #define CLOSE       0         // 電磁弁：閉


/* ------------------------ *
 *   Constant
 * ------------------------ */
  enum EN_SOL_NO {  SOL_NO1 = 0,        // 0: SOL 1
                    SOL_NO2,            // 1: SOL 2
                    SOL_NO3, };         // 2: SOL 3

  enum EN_SOL_SET { EN_SOL_ON = 0,      // 0: Sol ON/OFF
                    EN_SOL_PIN  };      // 1: Sol Port No(Gxx)

  const U1 U1_SOL_NUM = SOL_NO3 + 1;    // ブラダ数

  const U2 U2_CNT_SOL_OFF = (U2)(TIME_SOL_OFF / MAIN_CYCLE);

/* ------------------------ *
 *   Global variables
 * ------------------------ */
                                                          // Use ,  Port No
  volatile U1 u1_sol_seting[U1_SOL_NUM][EN_SOL_PIN + 1] = { { ON ,  P_SOL1 },      // SOL 1 設定
                                                            { ON ,  P_SOL2 },      // SOL 2 設定
                                                            { ON ,  P_SOL3 } };    // SOL 3 設定

  volatile U1 f_sol_req;                   // Solenoid drive request
  volatile U2 u2_cnt_sol_off[U1_SOL_NUM];  // Solenoid drive time cnt



/*******************************************************************************
 * User defined functions
 *******************************************************************************/

/* ------------------------ *
 *   GPIO出力ポート初期化
 * ------------------------ */
 void init_Sol(void)
 {

    Serial.println("Solenoid init start...");

    for (int i = 0; i < U1_SOL_NUM; i++) {
        pinMode(u1_sol_seting[i][EN_SOL_PIN], OUTPUT);
        digitalWrite(u1_sol_seting[i][EN_SOL_PIN], OFF);

        // ピン設定情報を表示
        Serial.print("SOL");
        Serial.print(i + 1);
        Serial.print(" - Pin:");
        Serial.print(u1_sol_seting[i][EN_SOL_PIN]);
        Serial.print(", Enable:");
        Serial.println(u1_sol_seting[i][EN_SOL_ON] ? "ON" : "OFF");
    }

    Serial.println("Solenoid init finish !");
    Serial.println("");

    f_sol_req = OFF;
    for (int i = 0; i < U1_SOL_NUM; i++) {
        u2_cnt_sol_off[i] = 0;
    }
 }

/* ------------------------ *
 *   ソレノイドバルブ駆動要求判定 (CSV同期版)
 * ------------------------ */
void judge_Sol(void)
{
    static U1 tf_start_sol_last = OFF;

    if (f_start != tf_start_sol_last) {
        if (f_start == ON) {
            f_sol_req = u1_pat_seting[u1_pat_no][EN_PAT_SOL];
            Serial.print("[SOL START] Pattern:");
            Serial.print(u1_pat_no);
            Serial.print(" SOL Enable:");
            Serial.print(f_sol_req ? "ON" : "OFF");
            Serial.print(" (value=");
            Serial.print(u1_pat_seting[u1_pat_no][EN_PAT_SOL]);
            Serial.print(") f_sol_req=");
            Serial.println(f_sol_req);
        }
        else {
            f_sol_req = OFF;
            Serial.println("[SOL STOP] f_start=OFF");
        }
    }
    tf_start_sol_last = f_start;
}

/* ------------------------ *
 *   CSV同期ブラダ駆動判定 (HeartBeat方式)
 *   ON/OFFタイミング配列と現在時間を比較
 * ------------------------ */
U1 judge_Sol_CSV_sync(U1 tu1_sol_ch)
{
    static U2 tu2_on_csv_index[U1_SOL_NUM] = {0};   // 各ソレノイドのONインデックス
    static U2 tu2_off_csv_index[U1_SOL_NUM] = {0};  // 各ソレノイドのOFFインデックス
    static U1 tu1_sol_state[U1_SOL_NUM] = {OFF};    // 各ソレノイドの現在状態
    static U1 f_synchro_sound_last = OFF;
    static U4 tu4_time_sound_start = 0;
    U4 tu4_current_time;

    // ソレノイドチャンネル範囲チェック
    if (tu1_sol_ch >= U1_SOL_NUM) {
        return OFF;
    }

    // 音声再生開始時間記憶（モーターCSVと同じタイミング基準）
    if ((f_synchro_sound == ON) && (f_synchro_sound_last == OFF)) {
        tu4_time_sound_start = millis();
        for (int i = 0; i < U1_SOL_NUM; i++) {
            tu2_on_csv_index[i] = 0;
            tu2_off_csv_index[i] = 0;
        }

        if (DEBUG_SYNC_SOL) {
            Serial.print("[SOL CSV SYNC] Sound start time: ");
            Serial.println(tu4_time_sound_start);
        }

        // ソレノイドCSVデータ数の確認
        Serial.print("[SOL CSV INFO] ON entries: ");
        Serial.print(u2_sol_on_index);
        Serial.print(", OFF entries: ");
        Serial.println(u2_sol_off_index);
    }

    // 音声再生中でない場合は常にOFF
    if (f_synchro_sound == OFF) {
        tu2_on_csv_index[tu1_sol_ch] = 0;
        tu2_off_csv_index[tu1_sol_ch] = 0;
        tu1_sol_state[tu1_sol_ch] = OFF;
        f_synchro_sound_last = f_synchro_sound;
        return OFF;
    }

    // 現在時間をモーターCSVと同じ方式で計算 (millis基準)
    tu4_current_time = millis() - tu4_time_sound_start;

    // デバッグ: CSV チェック状況を定期的に出力（ソレノイド0のみ、5秒間隔）
    static U4 last_debug_time = 0;
    if ((tu1_sol_ch == 0) && (tu4_current_time > last_debug_time + 5000)) {
        Serial.print("[SOL CSV DEBUG] Ch0 time:");
        Serial.print(tu4_current_time);
        Serial.print("ms, checking ON idx:");
        Serial.print(tu2_on_csv_index[0]);
        Serial.print("/");
        Serial.print(u2_sol_on_index);
        Serial.print(", OFF idx:");
        Serial.print(tu2_off_csv_index[0]);
        Serial.print("/");
        Serial.println(u2_sol_off_index);
        last_debug_time = tu4_current_time;
    }

    // ON タイミングチェック (下降トレンド検出時刻)
    for (U2 i = tu2_on_csv_index[tu1_sol_ch]; i < u2_sol_on_index; i++) {
        if (tu4_current_time >= u4_time_sol_on[i]) {
            tu1_sol_state[tu1_sol_ch] = ON;
            tu2_on_csv_index[tu1_sol_ch] = i + 1;

            if (DEBUG_SYNC_SOL) {
                Serial.print("SOL");
                Serial.print(tu1_sol_ch + 1);
                Serial.print(" ON: time=");
                Serial.print(tu4_current_time);
                Serial.print("ms, tgt=");
                Serial.print(u4_time_sol_on[i]);
                Serial.print("ms, idx=");
                Serial.print(i);
                Serial.print(", abs_time=");
                Serial.println(millis());
            }
            break;
        }
    }

    // OFF タイミングチェック (上昇トレンド検出時刻)
    for (U2 i = tu2_off_csv_index[tu1_sol_ch]; i < u2_sol_off_index; i++) {
        if (tu4_current_time >= u4_time_sol_off[i]) {
            tu1_sol_state[tu1_sol_ch] = OFF;
            tu2_off_csv_index[tu1_sol_ch] = i + 1;

            if (DEBUG_SYNC_SOL) {
                Serial.print("SOL");
                Serial.print(tu1_sol_ch + 1);
                Serial.print(" OFF: time=");
                Serial.print(tu4_current_time);
                Serial.print("ms, tgt=");
                Serial.print(u4_time_sol_off[i]);
                Serial.print("ms, idx=");
                Serial.print(i);
                Serial.print(", abs_time=");
                Serial.println(millis());
            }
            break;
        }
    }

    f_synchro_sound_last = f_synchro_sound;
    return tu1_sol_state[tu1_sol_ch];
}


/* ------------------------ *
 *   ソレノイドドライバ
 * ------------------------ */
void drv_Sol(U1 tu1_ch, U1 tu1_req)
{
    static U1 last_sol_state[U1_SOL_NUM] = {OFF, OFF, OFF}; // 前回の状態を保存

    if (u1_sol_seting[tu1_ch][EN_SOL_ON] == ON) {
        if (tu1_req == ON) {
            // Sol Valve Open
            digitalWrite(u1_sol_seting[tu1_ch][EN_SOL_PIN], OPEN);

            // 状態が変化した時のみデバッグ出力
            if (last_sol_state[tu1_ch] != ON) {
                Serial.print("[SOL DEBUG] SOL");
                Serial.print(tu1_ch + 1);
                Serial.print(" OPEN - Pin:");
                Serial.print(u1_sol_seting[tu1_ch][EN_SOL_PIN]);
                Serial.print(" Time:");
                Serial.println(millis());
                last_sol_state[tu1_ch] = ON;
            }
        }
        else {
            // Sol Valve Close
            digitalWrite(u1_sol_seting[tu1_ch][EN_SOL_PIN], CLOSE);

            // 状態が変化した時のみデバッグ出力
            if (last_sol_state[tu1_ch] != OFF) {
                Serial.print("[SOL DEBUG] SOL");
                Serial.print(tu1_ch + 1);
                Serial.print(" CLOSE - Pin:");
                Serial.print(u1_sol_seting[tu1_ch][EN_SOL_PIN]);
                Serial.print(" Time:");
                Serial.println(millis());
                last_sol_state[tu1_ch] = OFF;
            }
        }
    }
    else {
        // Sol Valve Close (設定で無効)
        digitalWrite(u1_sol_seting[tu1_ch][EN_SOL_PIN], CLOSE);

        if (last_sol_state[tu1_ch] != OFF) {
            Serial.print("[SOL DEBUG] SOL");
            Serial.print(tu1_ch + 1);
            Serial.print(" DISABLED - Pin:");
            Serial.print(u1_sol_seting[tu1_ch][EN_SOL_PIN]);
            Serial.print(" Time:");
            Serial.println(millis());
            last_sol_state[tu1_ch] = OFF;
        }
    }
}


/* ------------------------ *
 *   ソレノイドバルブ出力判定
 * ------------------------ */
void output_Sol(void)
{
    static  U1 tu1_sol_drv[U1_SOL_NUM] = {OFF};
    static  U1 tu1_sol_drv_last[U1_SOL_NUM] = {OFF};
            U1 tf_sol_ena[U1_SOL_NUM] = {DISABLE};
            U1 tf_cnt_clr = OFF;
            U1 tf_cnt_start = OFF;

    static U1 tf_sol_req_last = OFF;

    // f_sol_req の状態変化をログ出力
    if (f_sol_req != tf_sol_req_last) {
        Serial.print("[SOL OUTPUT] f_sol_req=");
        Serial.print(f_sol_req ? "ON" : "OFF");
        Serial.print(" Time:");
        Serial.println(millis());
        tf_sol_req_last = f_sol_req;
    }

    if (f_sol_req == ON) {

        // 各ソレノイドを個別にCSV同期制御
        U1 tu1_any_sol_active = OFF;

        for (int i = 0; i < U1_SOL_NUM; i++) {

            // 各ソレノイドのCSV同期判定
            U1 tu1_csv_sol_enable = judge_Sol_CSV_sync(i);

            // CSV同期によるブラダ制御
            if (tu1_csv_sol_enable == ON) {
                tu1_sol_drv[i] = ON;
                tu1_any_sol_active = ON;

                // CSV同期でONになった場合のデバッグ出力
                if (DEBUG_SYNC_SOL && tu1_sol_drv_last[i] != ON) {
                    Serial.print("[CSV SYNC] SOL");
                    Serial.print(i + 1);
                    Serial.print(" CSV ON - Time:");
                    Serial.println(millis());
                }
            } else {
                tu1_sol_drv[i] = OFF;

                // CSV同期でOFFになった場合のデバッグ出力
                if (DEBUG_SYNC_SOL && tu1_sol_drv_last[i] != OFF) {
                    Serial.print("[CSV SYNC] SOL");
                    Serial.print(i + 1);
                    Serial.print(" CSV OFF - Time:");
                    Serial.println(millis());
                }
            }
        }

        // 同期フラグ制御（いずれかのソレノイドが動作中の場合ON）
        if (tu1_any_sol_active == ON) {
            f_synchro_sol = ON;
        } else {
            f_synchro_sol = OFF;
        }
    }
    else {
        for (int i = 0; i < U1_SOL_NUM; i++) {
            tu1_sol_drv[i] = OFF;
            u2_cnt_sol_off[i] = 0;
        }
        f_synchro_sol = OFF;
    }


    for (int i = 0; i < U1_SOL_NUM; i++) {
        drv_Sol(i, tu1_sol_drv[i]);
        tu1_sol_drv_last[i] = tu1_sol_drv[i];
    }
}