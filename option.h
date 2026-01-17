/*******************************************************************************
 *  File   : option.h
 *  Author : Suguru Aoki @ HVD
 *  Update : 2025/04/08
 *  Overview : 
 *******************************************************************************/

#ifndef _option_h
#define _option_h

/* ------------------------ *
 *  Option definition
 * ------------------------ */

#define  DEBUG_CYCLE_TIME      0

// CSV関連デバッグ
#define  DEBUG_CSV_LOAD        1   // CSV読み込み過程の詳細表示
#define  DEBUG_CSV_TABLE       0   // CSV読み込み結果表示（全データ）※大量出力注意
#define  DEBUG_CSV_CONVERT     1   // CSV変換過程の詳細表示（元データ→変換後）
#define  DEBUG_CSV_SUMMARY     1   // CSV読み込みサマリー表示

// 出力系デバッグ
#define  DEBUG_MOTOR_OUTPUT    0
#define  DEBUG_SOUND_OUTPUT    0
#define  DEBUG_PRESS_OUTPUT    0

// タイミング系デバッグ
#define  DEBUG_TIME_MTON       1   // 心音タイミング表示
#define  DEBUG_SYNC_MOTOR      1   // モーター同期状況表示

#define  DEBUG_TIME_SOL_ON     1   // ブラダタイミング表示
#define  DEBUG_SYNC_SOL        1   // ブラダ制御状況表示

#endif
