/*******************************************************************************
 *  File   : SwBox_main.ino
 *  Update : 2025/07/17
 *  Bord   : M5Stack CoreS3 SE
 *  Lib    : M5CoreS3.h, M5Unified.h
 *  Overview : 
 *******************************************************************************/


/*******************************************************************************
 * 
 * 開発環境 (ArduinoIDE)
 * Bord    : M5Stack -> M5CoreS3
 * Upload Speed : 115200
 * Library : M5CoreS3 (Ver.1.0.1)
 *           M5Unified (Ver.0.2.5)
 *           M5GFX (Ver.0.2.6)
 *           M5UnitSynth (Ver.1.0.1)
 *           SparkFun_MicroPressure_Library (Ver.1.0.1)
 *           SparkFun_I2C_Mux_Arduino_Library (Ver.1.0.3)
 *           DFRobot_DF1201S (Ver.1.0.2)
 * 
 *******************************************************************************/


/*******************************************************************************
 * Standard library include definition
 *******************************************************************************/
// #include <M5Atom.h>   // M5Stack Atom Lite
// #include <M5Core2.h>  // M5stack core2
  #include <M5CoreS3.h>    // M5stack coreS3 SE

// #include <M5Stack.h>  // M5Unifiedへ移行
  #include <M5Unified.h>

// DFPlyerPro
  #include <DFRobot_DF1201S.h>

// SD Card
  #include <SD.h>
  #include <SPI.h>

// SparkFun MicroPressure Sensor (MPRLS0025PA00001A)
  #include <Wire.h>
  #include <SparkFun_MicroPressure.h>

// M5Stack PortA Unit Hub V2.1 (PAC9548AP)
  #include <SparkFun_I2C_Mux_Arduino_Library.h>


/*******************************************************************************
 * User defined file include definition
 *******************************************************************************/
  #include "option.h"    /*  オプション設定     */
  #include "c.h"         /*  共通の定義ファイル */
  #include "const.h"     /*  定数ファイル       */
  // #include "port_set.h"  /*  ポート設定         */

  enum EN_PAT_SET { EN_PAT_MP3 = 0,   // 0: Sound ON/OFF
                    EN_PAT_MT,        // 1: Motor ON/OFF
                    EN_PAT_SOL };     // 2: Solenoid ON/OFF

  enum EN_PAT_NO {  PAT_0 = 0,        // 0: Pattern 0 選択なし
                    PAT_1,            // 1: Pattern 1 選択１ 心音   (音, 振動)
                    PAT_2,            // 2: Pattern 2 選択２ 呼吸音 (音, ブラダ)
                    PAT_3,            // 3: Pattern 3 選択３ 複合   (音, 振動, ブラダ)
                    PAT_4 };          // 4: Pattern 4 選択４ 音声   (音声, 振動)

  enum EN_VOL_CH {  VOL_SPK = 0,      // 0: スピーカー音量
                    VOL_PWM,          // 1: 振動モーターDUTY比
                    VOL_PRESS,        // 2: ブラダ圧力
                    VOL_NA_0,         // 3: 割り当てなし
                    VOL_NA_1  };      // 4: 割り当てなし

  enum EN_CSV_ROW { EN_TIME = 0,      // 0: 時間
                    EN_BREATHING,     // 1: 呼吸音
                    EN_HEARTBEAT};    // 2: 心音

/* ------------------------ *
 *   Global variables
 * ------------------------ */
// SW BOX
  volatile ST_BYTE st_swbox_stat[2];
  volatile U1 u1_swbox_vol[5];

  volatile U1 f_start;
  volatile U1 u1_pat_no;
                                                          //  MP3  , MT   , SOL
  volatile U1 u1_pat_seting[PAT_4 + 1][EN_PAT_SOL + 1] =  { { OFF  , OFF  , OFF },     // PAT_0 設定 (OFF)
                                                            { ON   , ON   , OFF },     // PAT_1 設定 (MP3 + MT)
                                                            { ON   , OFF  , ON  },     // PAT_2 設定 (MP3 + SOL)
                                                            { ON   , ON   , ON  },     // PAT_3 設定 (MP3 + MT + SOL)
                                                            { ON   , ON   , OFF } };   // PAT_4 設定 (MP3 + MT)

// Motor
  volatile U1 f_synchro_motor;      // 振動モータ駆動中フラグ

// Solenoid
  volatile U1 f_synchro_sol;        // 電磁弁駆動中フラグ

// MP3
  volatile U1 f_synchro_sound;      // 音源再生中フラグ

  DFRobot_DF1201S  DF1201S;
  HardwareSerial   DF1201SSerial(2);  // HardwareSerial2を使用(SwBoxと競合しないよう)

// SD Card -> csv
  volatile S4 s4_vital[CSV_FILE_NUM][U2_CSV_ROW][U1_CSV_COL];
  volatile U4 u4_time_mt_on[U2_CSV_ROW];
  volatile U2 u2_time_index;

// Bladder CSV Synchronization (HeartBeat方式と同じ設計)
  volatile U4 u4_time_sol_on[U2_CSV_ROW];     // ブラダON時間配列 (下降トレンド検出時刻)
  volatile U4 u4_time_sol_off[U2_CSV_ROW];    // ブラダOFF時間配列 (上昇トレンド検出時刻)
  volatile U2 u2_sol_on_index;                // ブラダONインデックス
  volatile U2 u2_sol_off_index;               // ブラダOFFインデックス

// Future: Individual Solenoid Control (currently all use same timing)
// volatile U4 u4_time_sol_on_individual[U1_SOL_NUM][U2_CSV_ROW];  // 各ソレノイド個別タイミング
// volatile U2 u2_sol_index_individual[U1_SOL_NUM];                // 各ソレノイド個別インデックス



/*******************************************************************************
 * User defined functions
 *******************************************************************************/

/* ------------------------ *
 *   Setup function
 * ------------------------ */
void setup()
{
    Serial.begin(115200);     // make sure your Serial Monitor is also set at this baud rate.
    delay(500);

    // M5Stack
    auto cfg = M5.config();    // 設定用の構造体を代入, default値を設定
    // M5.begin(true, false);  // SDカードと関連の初期化を有効にする => M5Unifiedへ移行によりcfgで設定
    M5.begin(cfg);             // M5Unifiedの移植

    // read CSV
    u2_time_index = 0;
    u2_sol_on_index = 0;   // Initialize bladder ON index
    u2_sol_off_index = 0;  // Initialize bladder OFF index
    make_csv_table();

    // Pressure Sensor
    init_MPR();

    // Vibration Motor
    init_Motor();

    // Solenoid Valve
    init_Sol();

    // DFPlayerPro
    init_Sound();

    // SW BOX
    init_swbox();
}


/* ------------------------ *
 *   Main function
 * ------------------------ */
void loop()
{
    static U4 tu4_main_cycle = 0;

    static U4 tu4_ms_sync = 0;
    static U4 tu4_ms_max = 0;

    static U4 tu4_ms_swbox = 0;
    static U4 tu4_ms_press = 0;
    static U4 tu4_ms_sound = 0;
    static U4 tu4_ms_motor = 0;
    static U4 tu4_ms_sol = 0;

    // SW BOX
    // if (DEBUG_CYCLE_TIME) {
    //     tu4_ms_swbox = millis();
    // }

    rcv_UART();
    judge_pattern();

    // if (DEBUG_CYCLE_TIME) {
    //     Serial.print(millis() - tu4_ms_swbox);
    //     Serial.println();
    // }

    tu4_ms_sync = (millis() - tu4_main_cycle);

    if (tu4_ms_sync >= MAIN_CYCLE) {
 
        if (DEBUG_CYCLE_TIME) {
            if (   (tu4_ms_sync < 500) && (tu4_ms_sync > tu4_ms_max)) { tu4_ms_max = tu4_ms_sync; }
        }

        tu4_main_cycle = millis();
        // Serial.print(tu4_main_cycle);
        // Serial.println("ms");

        if (DEBUG_CYCLE_TIME) {
            Serial.print("t:");
            Serial.print(tu4_ms_sync);
            Serial.print(", m:");
            Serial.print(tu4_ms_max);
        }

        /*
         * Pressure Sensor (COMMENTED OUT FOR CSV BLADDER SYNC DEBUG)
         */
        // if (DEBUG_CYCLE_TIME) {
        //     tu4_ms_press = millis();
        // }

        // proc_Pressure();

        // if (DEBUG_CYCLE_TIME) {
        //     Serial.print(", p:");
        //     Serial.print(millis() - tu4_ms_press);
        // }

        /*
         * Sound
         */
        if (DEBUG_CYCLE_TIME) {
            tu4_ms_sound = millis();
        }

        judge_Sound();
        output_Sound();

        if (DEBUG_CYCLE_TIME) {
            Serial.print(", s:");
            Serial.print(millis() - tu4_ms_sound);
        }

        /*
         * Vibration Motor
         */
        if (DEBUG_CYCLE_TIME) {
            tu4_ms_motor = millis();
        }

        judge_Motor();
        output_Motor();

        if (DEBUG_CYCLE_TIME) {
            Serial.print(", mt:");
            Serial.print(millis() - tu4_ms_motor);
        }

        /*
         * Solenoid Valve 
         */
        if (DEBUG_CYCLE_TIME) {
           tu4_ms_sol = millis();
        }

        judge_Sol();
        output_Sol();

        if (DEBUG_CYCLE_TIME) {
            Serial.print(", sol:");
            Serial.print(millis() - tu4_ms_sol);
        }

        if (DEBUG_CYCLE_TIME) {
            Serial.println();
        }
    }
}
