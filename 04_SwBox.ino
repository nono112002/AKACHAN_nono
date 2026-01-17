/*******************************************************************************
 *  File   : SwBox.ino
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
  // M5stack coreS3 : UART1
  #define P_RXD0      44        // G44: RXD0
  #define P_TXD0      43        // G43: TXD0

  #define f_sw_start    st_swbox_stat[0].Bit.b0   // 01h : START/STOP
  #define f_sw_pat_1    st_swbox_stat[0].Bit.b1   // 02h : PATTERN 1
  #define f_sw_NA_12    st_swbox_stat[0].Bit.b2   // 04h
  #define f_sw_pat_2    st_swbox_stat[0].Bit.b3   // 08h : PATTERN 2
  #define f_sw_NA_14    st_swbox_stat[0].Bit.b4   // 10h
  #define f_sw_pat_3    st_swbox_stat[0].Bit.b5   // 20h : PATTERN 3
  #define f_sw_NA_16    st_swbox_stat[0].Bit.b6   // 40h
  #define f_sw_NA_17    st_swbox_stat[0].Bit.b7   // 80h

  #define f_sw_NA_20    st_swbox_stat[1].Bit.b0   // 01h
  #define f_sw_pat_4    st_swbox_stat[1].Bit.b1   // 02h : PATTERN 4
  #define f_sw_NA_22    st_swbox_stat[1].Bit.b2   // 04h
  #define f_sw_NA_23    st_swbox_stat[1].Bit.b3   // 08h
  #define f_sw_NA_24    st_swbox_stat[1].Bit.b4   // 10h
  #define f_sw_NA_25    st_swbox_stat[1].Bit.b5   // 20h
  #define f_sw_NA_26    st_swbox_stat[1].Bit.b6   // 40h
  #define f_sw_NA_27    st_swbox_stat[1].Bit.b7   // 80h

/* ------------------------ *
 *   Constant
 * ------------------------ */

/* ------------------------ *
 *   Global variables
 * ------------------------ */
  volatile U1 u1_rcv_data[7] = {0,0,0,0,0,0,0};



/*******************************************************************************
 * User defined functions
 *******************************************************************************/

/* ------------------------ *
 *   UART初期化
 * ------------------------ */
void init_UART() 
{
    Serial.println("SwBox init start...");

    Serial1.begin(38400, SERIAL_8N1, P_RXD0, P_TXD0);

    Serial.println("SwBox init finish !");
    Serial.println("");
}

/* ------------------------ *
 *   SW BOX初期化
 * ------------------------ */
void init_swbox()
{
    init_UART();

    f_start = OFF;
    u1_pat_no = PAT_0; 
}


/* ------------------------ *
 *   SW BOXからのUART受信
 * ------------------------ */
void rcv_UART() 
{
    if (Serial1.available() >= 7)
    {
        for (int i = 0; i <= 6; i++)
        {
            u1_rcv_data[i] = Serial1.read();
        }

        st_swbox_stat[0].Byte = u1_rcv_data[0];
        st_swbox_stat[1].Byte = u1_rcv_data[1];

        u1_swbox_vol[0] = u1_rcv_data[2];
        u1_swbox_vol[1] = u1_rcv_data[3];
        u1_swbox_vol[2] = u1_rcv_data[4];
        u1_swbox_vol[3] = u1_rcv_data[5];
        u1_swbox_vol[4] = u1_rcv_data[6];

        // Serial.print("Rx:  ");
        // for (int i = 0; i < 2; i++)
        // {
        //     Serial.print(st_swbox_stat[i].Byte, HEX);
        //     Serial.print("  ");
        // }
        // for (int i = 0; i < 5; i++)
        // {
        //     Serial.print(u1_swbox_vol[i], HEX);
        //     Serial.print("  ");
        // }
        // Serial.println();
        // Serial.print(f_sw_start, HEX);
        // Serial.print(" ");
        // Serial.print(f_sw_pat_1, HEX);
        // Serial.print(" ");
        // Serial.print(f_sw_pat_2, HEX);
        // Serial.print(" ");
        // Serial.print(f_sw_pat_3, HEX);
        // Serial.print(" ");
        // Serial.print(f_sw_pat_4, HEX);
        // Serial.println();
        while ( Serial1.available() ) {
            Serial1.read();
        }
    }


  return;
}


/* ------------------------ *
 *   パターン判定
 * ------------------------ */
void judge_pattern()
{
    // START / STOP の判定
    static U1 tf_start_swbox_last = OFF;

    if (f_sw_start != tf_start_swbox_last) {
        Serial.print("[SWBOX] f_sw_start=");
        Serial.print(f_sw_start ? "ON" : "OFF");
        Serial.print(" Time:");
        Serial.println(millis());
        tf_start_swbox_last = f_sw_start;
    }

    f_start = f_sw_start;

    // パターン選択 の判定
    if (f_start == OFF) {
        u1_pat_no = PAT_0;
    }
    else if (   (f_sw_NA_12 == ON)
             || (f_sw_NA_14 == ON)
             || (f_sw_NA_16 == ON)
             || (f_sw_NA_17 == ON)
             || (f_sw_NA_20 == ON)
             || (f_sw_NA_22 == ON)
             || (f_sw_NA_23 == ON)
             || (f_sw_NA_24 == ON)
             || (f_sw_NA_25 == ON)
             || (f_sw_NA_26 == ON)
             || (f_sw_NA_27 == ON)) {

        u1_pat_no = PAT_0;
        Serial.println("Sw Box Signal Error... ");
    }
    else if (   (f_sw_pat_1 == ON)
             && (f_sw_pat_2 == OFF)
             && (f_sw_pat_3 == OFF)
             && (f_sw_pat_4 == OFF)) {

        u1_pat_no = PAT_1;
    }
    else if (   (f_sw_pat_1 == OFF)
             && (f_sw_pat_2 == ON)
             && (f_sw_pat_3 == OFF)
             && (f_sw_pat_4 == OFF)) {

        u1_pat_no = PAT_2;
    }
    else if (   (f_sw_pat_1 == OFF)
             && (f_sw_pat_2 == OFF)
             && (f_sw_pat_3 == ON)
             && (f_sw_pat_4 == OFF)) {

        u1_pat_no = PAT_3;
    }
    else if (   (f_sw_pat_1 == OFF)
             && (f_sw_pat_2 == OFF)
             && (f_sw_pat_3 == OFF)
             && (f_sw_pat_4 == ON)) {

        u1_pat_no = PAT_4;
    }
    else {
        u1_pat_no = PAT_0;
    }
    // Serial.print("PAT: ");
    // Serial.println(u1_pat_no);
}
