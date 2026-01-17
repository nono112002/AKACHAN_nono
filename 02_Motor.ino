/*******************************************************************************
 *  File   : Motor.ino
 *  Update : 2025/07/17
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
  // M5stack coreS3 : PWM
  #define P_PWM1      8         // G8 : PB_IN
  #define P_PWM2      9         // G9 : PB_OUT
  #define P_PWM3      13        // G13: I2C_DOUT
  #define P_PWM4      14        // G14: I2C_DIN

  #define FREQ_DEF    12000     // [Hz] Vibration Motor Frequency (default)

/* ------------------------ *
 *   Constant
 * ------------------------ */
  enum EN_PWM_CH  { PWM_CH1 = 0,        // 0: PWM ch0
                    PWM_CH2,            // 1: PWM ch1
                    PWM_CH3,            // 2: PWM ch2
                    PWM_CH4 };          // 3: PWM ch3

  enum EN_MT_SET  { EN_MT_ON = 0,       // 0: Motor ON/OFF
                    EN_MT_PIN,          // 1: PWM Port No(Gxx)
                    EN_MT_CH };         // 2: Motor chx

  const U1 U1_MT_NUM = PWM_CH4 + 1;     // モーター数

  const U2 U2_CNT_MT_ON  = (U2)(TIME_MT_ON  / MAIN_CYCLE);
  const U2 U2_CNT_MT_OFF = (U2)(TIME_MT_OFF / MAIN_CYCLE);

/* ------------------------ *
 *   Global variables
 * ------------------------ */
                                                      // Use ,  Port No,  PWM Ch
  volatile U1 u1_mt_seting[U1_MT_NUM][EN_MT_CH + 1] = { { ON ,  P_PWM1 ,  PWM_CH1 },      // MT1 設定
                                                        { ON ,  P_PWM2 ,  PWM_CH2 },      // MT2 設定
                                                        { ON ,  P_PWM3 ,  PWM_CH3 },      // MT3 設定
                                                        { ON ,  P_PWM4 ,  PWM_CH4 } };    // MT4 設定

  volatile U1 f_motor_req;      // Motor running request
  volatile U1 u1_motor_duty;    // PWM duty
  volatile U2 u2_cnt_motor;     // Motor running time cnt


/*******************************************************************************
 * User defined functions
 *******************************************************************************/

/* ------------------------ *
 *   PWM出力ポート初期化
 * ------------------------ */
void init_Motor(void)
{
    Serial.println("Motor init start...");

    for (int i = 0; i < U1_MT_NUM; i++) {
        // Serial.print("i:");
        // Serial.print(i);
        ledcSetup(u1_mt_seting[i][EN_MT_CH], FREQ_DEF, 8);
        ledcAttachPin(u1_mt_seting[i][EN_MT_PIN], u1_mt_seting[i][EN_MT_CH]);
        // Serial.println("");
    }

    Serial.println("Motor init finish !");
    Serial.println("");

    u1_motor_duty = 0;
    f_motor_req = OFF;
    u2_cnt_motor = 0;
}


/* ------------------------ *
 *   振動モータ駆動要求判定
 * ------------------------ */
void judge_Motor(void)
{
    static U1 tf_start_last = OFF;
    static U1 tf_pat_mt_req = OFF;

    static U1 f_synchro_sound_last;
    static U4 tu4_time_sound_start;
    static U4 tu4_time_diff;
    static U2 tu2_cnt_index = 0;

    if (f_start != tf_start_last) {
        if (f_start == ON) {
            // f_motor_req = u1_pat_seting[u1_pat_no][EN_PAT_MT];
            tf_pat_mt_req = u1_pat_seting[u1_pat_no][EN_PAT_MT];
            Serial.print("[MOTOR START] Pattern:");
            Serial.print(u1_pat_no);
            Serial.print(" Motor Enable:");
            Serial.println(tf_pat_mt_req ? "ON" : "OFF");
        }
        else {
            // f_motor_req = OFF;
            tf_pat_mt_req = OFF;
            Serial.println("[MOTOR STOP] f_start=OFF");
        }
    }
    tf_start_last = f_start;

    if (tf_pat_mt_req == ON) {

        /* MP3再生開始時間記憶 */
        if (   (f_synchro_sound == ON)
            && (f_synchro_sound_last == OFF)) {
 
            tu4_time_sound_start = millis();
            tu2_cnt_index = 0;

            Serial.print("[MOTOR CSV SYNC] Sound start time: ");
            Serial.println(tu4_time_sound_start);
        }

        if (f_synchro_sound == ON) {
            tu4_time_diff = millis() - tu4_time_sound_start;

            if (tu4_time_diff >= u4_time_mt_on[tu2_cnt_index]) {

                if (DEBUG_SYNC_MOTOR) {
                    Serial.print(" ind:");
                    Serial.print(tu2_cnt_index);
                    Serial.print(" tbl:");
                    Serial.print(u4_time_mt_on[tu2_cnt_index]);
                    Serial.print(" df:");
                    Serial.print(tu4_time_diff);
                    Serial.println();
                }

                f_motor_req = ON;
                Serial.print("[MOTOR CSV] ON at ");
                Serial.print(tu4_time_diff);
                Serial.print("ms, idx:");
                Serial.println(tu2_cnt_index);

                if (tu2_cnt_index < u2_time_index) {
                    tu2_cnt_index++;
                }
            }
        }
        else {
            f_motor_req = OFF;
            tu2_cnt_index = 0;
            Serial.println("[MOTOR CSV] f_synchro_sound=OFF");
        }
    }
    else {
        f_motor_req = OFF;
        tu2_cnt_index = 0;
    }
    f_synchro_sound_last = f_synchro_sound;

    // SW BOXからPWM Dutyを決定
    if (f_motor_req == OFF) {
        u1_motor_duty = 0;
    }
    else {
        u1_motor_duty = map(u1_swbox_vol[VOL_PWM], 0, U1MAX, U1_PWM_MIN, U1_PWM_MAX);
    }
}


/* ------------------------ *
 *   振動モータ出力判定
 * ------------------------ */
void output_Motor(void)
{
    U1 tu1_mt_duty;

    if (f_motor_req == ON) {
        u2_cnt_motor++;
    }
    else {
        u2_cnt_motor = 0;
    }

    // if (u2_cnt_motor > U2_CNT_MT_OFF) {
    //     u2_cnt_motor = 0;
    // }
    // else if (u2_cnt_motor > U2_CNT_MT_ON) {
    if (u2_cnt_motor > U2_CNT_MT_ON) {
        tu1_mt_duty = 0;
        f_motor_req = OFF;
        f_synchro_motor = OFF;
        // Serial.print(millis());
        // Serial.println();
    }
    else if (u2_cnt_motor > 0) {
        tu1_mt_duty = u1_motor_duty;
        f_synchro_motor = ON;

        // if (DEBUG_MOTOR_OUTPUT) {
        //     Serial.print("d:");
        //     Serial.print(u1_motor_duty);
        //     Serial.println();
        // }
    }
    else {
        tu1_mt_duty = 0;
        f_synchro_motor = OFF;
    }

    for (int i = 0; i < U1_MT_NUM; i++) {
        if (u1_mt_seting[i][EN_MT_ON] == ON) {
            ledcWrite(u1_mt_seting[i][EN_MT_PIN], tu1_mt_duty);
            // if (DEBUG_MOTOR_OUTPUT) {
            //     Serial.print(" i:");
            //     Serial.print(i);
            //     Serial.print(" m:");
            //     Serial.print(u1_motor_duty);
            //     Serial.println();
            // }
        }
        else {
            ledcWrite(u1_mt_seting[i][EN_MT_PIN], 0);
        }
        delay(2);
    }

    // debug monitor
    // if (DEBUG_MOTOR_OUTPUT) {
    //     Serial.print("Cnt:");
    //     Serial.print(u2_cnt_motor);
    //     Serial.print("Con:");
    //     Serial.print(U2_CNT_MT_ON);
    //     Serial.print("Coff:");
    //     Serial.print(U2_CNT_MT_OFF);
    //     Serial.print('\t');
    //     Serial.print("MT:");
    //     Serial.print(u1_motor_duty);
    //     Serial.print('\t');
    //     Serial.print("Run:");
    //     Serial.print(f_motor_req);
    //     Serial.print('\t');
    //     Serial.print("bt:");
    //     Serial.print(f_synchro_motor);
    //     Serial.println();
    // }
}