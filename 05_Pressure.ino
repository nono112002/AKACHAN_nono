/*******************************************************************************
 *  File   : Pressure.ino
 *  Update : 2025/04/15
 *  Bord   : M5Stack CoreS3 SE
 *  Lib    : Wire.h, SparkFun_MicroPressure.h, SparkFun_I2C_Mux_Arduino_Library.h
 *  Overview : SparkFun MicroPressure Sensor + M5Stack PortA Unit Hub V2.1
 *******************************************************************************/

/*******************************************************************************
 * User defined constant/variables definition
 *******************************************************************************/

/* ------------------------ *
 *   Macro definition
 * ------------------------ */
  // M5stack coreS3 : I2C Port A
  #define PA_SDA      2         // G2 : PA_SDA
  #define PA_SCL      1         // G1 : PA_SCL

  #define UNIT_ADRS   0x70      // Unit Hub I2C Adress
  #define SAMPLE_NUM  5         // MRP サンプル数


/* ------------------------ *
 *   Constant
 * ------------------------ */
  enum EN_HUB_NO {  HUB_CH0 = 0,        // 0: MPR No.1
                    HUB_CH1,            // 1: MPR No.2
                    HUB_CH2,            // 2: Not Use
                    HUB_CH3,            // 3: Not Use
                    HUB_CH4,            // 4: MPR No.3
                    HUB_CH5, };         // 5: Not Use

  enum EN_MPR_NO {  MPR_NO1 = 0,        // 0: MPR No.1
                    MPR_NO2,            // 1: MPR No.2
                    MPR_NO3, };         // 2: MPR No.3

  enum EN_MPR_SET { EN_MPR_ON = 0,      // 0: MPR ON/OFF
                    EN_MPR_HUB };       // 1: HUB No

  const U1 U1_MPR_NUM = MPR_NO3 + 1;    // 圧力センサ数

  const U2 U2_CNT_MPR_CYC =   (U2)(TIME_MPR_CYC  / MAIN_CYCLE);

  const U4 U4_MPR_PRESS_MAX = (U4)(MPR_MAX_PRESS * 100.0);
  const U4 U4_MPR_PRESS_MIN = (U4)(MPR_MIN_PRESS * 100.0);
  const U4 U4_MPR_PRESS_OFF = (U4)(MPR_OFF_PRESS * 100.0);
    
/* ------------------------ *
 *   Global variables
 * ------------------------ */
// M5Stack PortA Unit Hub V2.1 (PAC9548AP)
  QWIICMUX i2cMux;

// SparkFun MicroPressure Sensor (MPRLS0025PA00001A)
  SparkFun_MicroPressure mpr[U1_MPR_NUM];
                                                             // Use ,  Hub Ch
  volatile    U1 u1_mpr_seting[U1_MPR_NUM][EN_MPR_HUB + 1] = { { ON ,  HUB_CH0 },     // MPR1 設定
                                                               { ON ,  HUB_CH1 },     // MPR2 設定
                                                               { ON ,  HUB_CH4 } };   // MPR3 設定

  volatile float fl_mpr_law[U1_MPR_NUM][SAMPLE_NUM] = {0};   // [KPA] 圧力センサ値(取得値)
  volatile float fl_mpr_def[U1_MPR_NUM];                     // [KPA] 大気圧
  volatile float fl_mpr_ave[U1_MPR_NUM];                     // [KPA] 圧力センサ値(移動平均)
  volatile float fl_mpr_diff[U1_MPR_NUM] = {0};              // [KPA] 大気圧との差圧
  volatile    U4 u4_mpr_diff[U1_MPR_NUM];                    // [kPa] 大気圧との差圧 (fl_mpr_diff * 100)
  volatile    U4 u4_mpr_tgt;                                 // [kPa] 差圧閾値 (fl_mpr_diff * 100)



/*******************************************************************************
 * User defined functions
 *******************************************************************************

/* ------------------------ *
 *   圧力センサ値取得
 * ------------------------ */
float read_Pressure(U1 tu1_ch)
{
    float tflt_mpr;

    i2cMux.setPort(u1_mpr_seting[tu1_ch][EN_MPR_HUB]);
    mpr[tu1_ch].begin(DEFAULT_ADDRESS, Wire);
    tflt_mpr = mpr[tu1_ch].readPressure(KPA);

        // Serial.print("ch");
        // Serial.print(tu1_ch);
        // Serial.print(": ");
        // Serial.print(tflt_mpr);
        // Serial.print(", ");

    return(tflt_mpr);
}


/* ------------------------ *
 *   大気圧取得
 * ------------------------ */
void get_AAP(void)     // AAP: Atmospheric Air Pressure
{
    for (int i = 0; i < U1_MPR_NUM; i++) {
        fl_mpr_def[i] = 0;

        for (int j = 0; j < SAMPLE_NUM; j++) {
            fl_mpr_law[i][j] = read_Pressure(i);
            fl_mpr_def[i] += fl_mpr_law[i][j];
            delay(50);
        }
        fl_mpr_def[i] = fl_mpr_def[i] / SAMPLE_NUM;

        // Serial.print("ch");
        // Serial.print(i);
        // Serial.print(": ");
        // Serial.println(fl_mpr_def[i]);
    }
}


/* ------------------------ *
 *   圧力センサ初期化
 * ------------------------ */
void init_MPR(void)
{
    Wire.begin(PA_SDA, PA_SCL);
    Wire.beginTransmission(UNIT_ADRS);
    Wire.endTransmission();

    if (i2cMux.begin(UNIT_ADRS, Wire) == false) {
        Serial.println("I2C cannot conect...");
    }
    Serial.println("Pressure Sensor init finish !");
    Serial.println("");

    get_AAP();
}


/* ------------------------ *
 *   目標圧力計算
 * ------------------------ */
void calc_target_Pressure()
{
    float tfl_tgt;
    u4_mpr_tgt = map(u1_swbox_vol[VOL_PRESS], 0, U1MAX, U4_MPR_PRESS_MIN, U4_MPR_PRESS_MAX);
    tfl_tgt = u4_mpr_tgt / 100.0;

    if (DEBUG_PRESS_OUTPUT) {
        Serial.print("             tgt: ");
        Serial.println(tfl_tgt);
    }
}


/* ------------------------ *
 *   圧力センサ処理
 * ------------------------ */
void proc_Pressure(void)
{
    static U2 tu2_mpr_cycle = 0;
    static U1 tu1_index = 0;

    tu2_mpr_cycle++;

    if (tu2_mpr_cycle > U2_CNT_MPR_CYC) {
        tu2_mpr_cycle = 0;

        for (int i = 0; i < U1_MPR_NUM; i++) {  
            fl_mpr_law[i][tu1_index] = read_Pressure(i);

            fl_mpr_ave[i] = 0;
            for (int j = 0; j < SAMPLE_NUM; j++) {
                fl_mpr_ave[i] += fl_mpr_law[i][j];
            }
            fl_mpr_ave[i] = fl_mpr_ave[i] / SAMPLE_NUM;
            fl_mpr_diff[i] = fl_mpr_ave[i] - fl_mpr_def[i];
            if (fl_mpr_diff[i] < 0.0) {
                u4_mpr_diff[i] = 0;
            }
            else {
                u4_mpr_diff[i] = (U4)(fl_mpr_diff[i] * 100.0);
            }

            if (DEBUG_PRESS_OUTPUT) {
                Serial.print("diff");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(fl_mpr_diff[i]);
            }
        }
        calc_target_Pressure();
    }

    tu1_index++;

    if (tu1_index >= SAMPLE_NUM) {
        tu1_index = 0;
    }
}


/* ------------------------ *
 *   目標圧力到達判定
 * ------------------------ */
U1 judge_Valve_open(U1 tu1_ch)
{
    U1 tu1_ret;

     if (u4_mpr_diff[tu1_ch] < u4_mpr_tgt) {
        tu1_ret = ENABLE;
    }
    else {
        tu1_ret = DISABLE;
    }

    return(tu1_ret);
}


/* ------------------------ *
 *   圧力低下判定
 * ------------------------ */
U1 judge_Valve_close(U1 tu1_ch)
{
    U1 tu1_ret;

    if (u4_mpr_diff[tu1_ch] < U4_MPR_PRESS_OFF) {
        tu1_ret = LOW;
    }
    else {
        tu1_ret = HIGH;
    }

    return(tu1_ret);
}
