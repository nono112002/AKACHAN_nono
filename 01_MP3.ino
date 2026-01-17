/*******************************************************************************
 *  File   : MP3.ino
 *  Update : 2025/07/17
 *  Bord   : M5Stack CoreS3 SE
 *  Lib    : DFRobot_DF1201S
 *  Overview : 
 *******************************************************************************/

/*******************************************************************************
 * User defined constant/variables definition
 *******************************************************************************/

/* ------------------------ *
 *   Macro definition
 * ------------------------ */
  // M5stack coreS3 : UART2
  #define PC_RX       18        // G18 : PortC_RX
  #define PC_TX       17        // G17 : PortC_TX

  #define MP3_DEF      1        // MP3 file number (default)

/* ------------------------ *
 *   Constant
 * ------------------------ */
  enum EN_MP3_STATE {  MP3_OFF = 0,   // 0: 起動前
                       MP3_INIT,      // 1: 初期化中
                       MP3_STOP,      // 2: 停止
                       MP3_PLAY,      // 3: 再生中
                       MP3_ERR };     // 4: ファイルクローズ

/* ------------------------ *
 *   Global variables
 * ------------------------ */

  volatile U1 f_sound_req;      // MP3 sound play request
  volatile U1 u1_sound_volume;  // MP3 sound volume
  volatile U2 u2_time_total;    // MP3 sound total time


  volatile EN_MP3_STATE   en_sound_stat;         // DFPlayer state (EN_MP3_STATE)
  volatile ST_BYTE        st_sound_err;
  #define  f_err_vol      st_sound_err.Bit.b0    // 01h : set volume err


/*******************************************************************************
 * User defined functions
 *******************************************************************************/


/* ----------------- *
 *   DFPlayer API    *
 * ----------------- */
 void init_Sound(void) {

    en_sound_stat = MP3_OFF;

    Serial.println("***** DFPlayer ******");
    Serial.println("    DFPlayer initializing start...");

    DF1201SSerial.begin(115200, SERIAL_8N1, PC_RX, PC_TX);  // RX=, TX=でシリアル通信
    en_sound_stat = MP3_INIT;

    // DF1201Sの初期化
    while (!DF1201S.begin(DF1201SSerial)) {
        Serial.println("  DFPlayer Init failed, please check the wire connection!");
        delay(1000);
    }

    // 音量設定
    DF1201S.setVol(U1_VOL_DEF);
    // Serial.print("    VOL: ");
    // Serial.println(DF1201S.getVol());

    // 音楽モードに切り替え
    DF1201S.switchFunction(DF1201S.MUSIC);
    // delay(2000);  // プロンプトトーンの終了待ち

    // 再生モードの設定：1曲だけ再生
    DF1201S.setPlayMode(DF1201S.SINGLE);  // SINGLEモードがある場合はこれを設定
    // Serial.print("    PlayMode: ");
    // Serial.println(DF1201S.getPlayMode());

    DF1201S.playFileNum(MP3_DEF);       // ファイルを再生
    // DF1201S.start();                    // 再生を開始
    DF1201S.pause();                    // 再生を停止

    Serial.print("    TotalFile ");
    Serial.println(DF1201S.getTotalFile());

    Serial.print("    CurFileNum: ");
    Serial.println(DF1201S.getCurFileNumber());

    // Serial.print("    FileName: ");
    // Serial.println(DF1201S.getFileName());
    // delay(3000);

    en_sound_stat = MP3_STOP;
    u1_sound_volume = U1_VOL_DEF;
    u2_time_total = DF1201S.getTotalTime();

    Serial.println("    DFPlayer initializing finished!");
    Serial.println("");

    st_sound_err.Byte = 0;
}


/* ------------------------ *
 *   MP3再生判定、音量設定
 * ------------------------ */
void judge_Sound(void) {

    static U1 tf_pat_mp3_req = OFF;
    static U1 tf_start_last = OFF;
    static EN_MP3_STATE ten_sound_stat_last = MP3_STOP;

    U2 tu2_time_cur = 0;

    if (f_start != tf_start_last) {
        if (f_start == ON) {
            tf_pat_mp3_req = u1_pat_seting[u1_pat_no][EN_PAT_MP3];
            Serial.print("[MP3 START] Pattern:");
            Serial.print(u1_pat_no);
            Serial.print(" MP3 Enable:");
            Serial.println(tf_pat_mp3_req ? "ON" : "OFF");
        }
        else {
            tf_pat_mp3_req = OFF;
            Serial.println("[MP3 STOP] f_start=OFF");
        }
    }
    tf_start_last = f_start;


   // play sound
    if (tf_pat_mp3_req == ON) {
        if (en_sound_stat == MP3_STOP) {
            f_sound_req = ON;
        }
        else if (en_sound_stat == MP3_PLAY) {

            // Serial.print("curtime: ");
            // Serial.println(DF1201S.getCurTime());
            // Serial.print("alltime: ");
            // Serial.println(DF1201S.getTotalTime());

            tu2_time_cur = DF1201S.getCurTime();

            if (tu2_time_cur == u2_time_total) {
                f_sound_req = OFF;
            }
        }
        else {
            f_sound_req = OFF;
        }
    }
    else {
        f_sound_req = OFF;
    }

    // MP3 再生終了判定
    if (en_sound_stat == MP3_PLAY) {
        if (f_sound_req == OFF) {
            en_sound_stat = MP3_STOP;
        }
    }




    // SW BOXからMP3 Volumeを決定
    if (f_sound_req == OFF) {
        u1_sound_volume = 0;
    }
    else {
        u1_sound_volume = map(u1_swbox_vol[VOL_SPK], 0, U1MAX, U1_VOL_MIN, U1_VOL_MAX);
    }
}


/* ------------------------ *
 *   DFPlayer Output
 * ------------------------ */
void output_Sound(void) {

    static U1 tu1_sound_vol_last = U1_VOL_DEF;
    static U1 tf_sound_req_last = OFF;
    U1 tu1_ret;

    // set volume
    if (u1_sound_volume != tu1_sound_vol_last) {
        DF1201S.setVol(u1_sound_volume);
        // tu1_ret = DF1201S.getVol();
        // if (tu1_ret != u1_sound_volume) {
        //     f_err_vol = ON;
        // };
        // Serial.print("    VOL: ");
        // Serial.println(DF1201S.getVol());
    }
    tu1_sound_vol_last = u1_sound_volume;

    // play sound
    if (f_sound_req != tf_sound_req_last) {
        if (f_sound_req == ON) {
            DF1201S.playFileNum(MP3_DEF);       // ファイルを再生
            en_sound_stat = MP3_PLAY;
            f_synchro_sound = ON;
            Serial.print("[MP3 PLAY] File:");
            Serial.print(MP3_DEF);
            Serial.print(" f_synchro_sound=ON Time:");
            Serial.println(millis());
        }
        else {
            DF1201S.pause();                   // 再生を停止
            en_sound_stat = MP3_STOP;
            f_synchro_sound = OFF;
            Serial.print("[MP3 PAUSE] f_synchro_sound=OFF Time:");
            Serial.println(millis());
        }
    }
    tf_sound_req_last = f_sound_req;

}
