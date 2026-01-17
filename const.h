/*******************************************************************************
 *  File   : const.h
 *  Author : Suguru Aoki, Hideki Sakakibara @ HVD
 *  Update : 2025/07/15
 *  Overview : 
 *******************************************************************************/

#ifndef _const_h
#define _const_h

/*******************************************************************************
 * User defined constant macro
 *******************************************************************************/

/* ------------------------ *
 *   MP3
 * ------------------------ */
  #define U1_VOL_MIN                  0     // {0-15} DFPlayerPro Volume (min)
  #define U1_VOL_DEF                 10     // {0-15} DFPlayerPro Volume (default) 
  #define U1_VOL_MAX                 15     // {0-15} DFPlayerPro Volume (max) ~30 

/* ------------------------ *
 *   Heart Beat (心音)
 * ------------------------ */
  #define TIME_MT_ON                 30     // [ms] Vibration Motor drive time
  #define TIME_MT_OFF               980     // [ms] Vibration Motor stop time

  #define U1_PWM_MIN                 50     // {0-255} Motor Duty 20% 相当 ( 50 / 255 * 100 ≒ 20%)
  #define U1_PWM_MAX                200     // {0-255} Motor Duty 80% 相当 (200 / 255 * 100 ≒ 80%)

/* ------------------------ *
 *   Breathing (呼吸音)
 * ------------------------ */
  #define TIME_MPR_CYC              100     // [ms] MPR read cycle
  #define TIME_SOL_OFF             5000     // [ms] Solenoid OFF time

  #define MPR_MAX_PRESS           10.00     // [kPa] MPR Pressure (70.00[kPa] 以下で設定)
  #define MPR_MIN_PRESS            0.50     // [kPa] MPR Pressure
  #define MPR_OFF_PRESS            0.10     // [kPa] MPR Pressure


/* ------------------------ *
 *   SD card
 * ------------------------ */
  #define CSV_FILE_NUM                1     // [個] 読込むcsvファイルの個数
  #define CSV_FILE_00   "/00_breathing_heartbeat.csv"    // 読込むcsvファイル名

  #define U2_CSV_ROW               4000     // csvを格納する配列の行サイズ
  #define U1_CSV_COL                  3     // csvを格納する配列の列サイズ


/* ------------------------ *
 *   csv 浮動小数->整数変換 閾値
 * ------------------------ */
  # define INPUT_MAX_BREATH         0.3
  # define INPUT_MIN_BREATH        -0.3
  # define OUTPUT_MAX_BREATH        2000    // 小数点第一位まで保持 (200.0 * 10)
  # define OUTPUT_MIN_BREATH       -2000    // 小数点第一位まで保持 (-200.0 * 10)

  # define INPUT_MAX_HEART          0.5
  # define INPUT_MIN_HEART         -0.5
  # define OUTPUT_MAX_HEART         200
  # define OUTPUT_MIN_HEART        -200

/* ------------------------ *
 *   極値検出 閾値
 * ------------------------ */
  # define BREATH_PEAK_THRESHOLD     1     // 呼吸音の山/谷検出最小変化量
  # define HEART_PEAK_THRESHOLD      8     // 心音の山検出最小変化量


/*******************************************************************************
 * Developer defined constant macro
 *******************************************************************************/

  #define MAIN_CYCLE                 10     // [ms] メイン処理時間

  #define ENABLE                      1     // 許可
  #define DISABLE                     0     // 不許可

#endif
