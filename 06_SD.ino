/*******************************************************************************
 *  File   : SD.ino
 *  Update : 2025/07/10
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

  /* --- ピン番号 ------------------------------------ */
  #define SPI_CS_PIN   4
  // # define SPI_MOSI_PIN 7 // G37
  // # define SPI_MISO_PIN 9 // G35
  // # define SPI_SCK_PIN 11 // G36

  /* --- 数字判別 ------------------------------------ */
  #define CHARACTER                   0     // 文字

  /* --- 極性判定 ------------------------------------ */
  #define SUCCESS                     1     // 成功
  #define FAILED                      0     // 失敗

  /* --- 極性判定 ------------------------------------ */
  #define NORMAL                      1     // 正常
  #define ABNORMAL                    0     // 異常


/* ------------------------ *
 *   Constant
 * ------------------------ */


/* ------------------------ *
 *   Global variables
 * ------------------------ */
  U1 gf_sd_init;                   // SDカードの初期化状態
  U1 gf_sd_open;                   // SDカードの開閉状態
  U1 gf_load_state[CSV_FILE_NUM];  // CSV読取り状態判定
  U1 gf_load_finish;               // CSV読取り状態

  S4 s4_csv_row_max;               // 行数の最大
  U1 f_csv_row_end;                // csv最終行到達フラグ

  File csv_file;


/*******************************************************************************
 * User defined functions
 *******************************************************************************

/* ------------------------------------------------------  *
 *   csvファイルオープン
 * ------------------------------------------------------- */
void open_csv(U1 u1_load_file_num)
{
  /* --- SDカードの初期化確認 ------------------------ */
  Serial.println("=== CSV File Loading ===");
  Serial.print("Initializing SD card...");

  if (SD.begin(SPI_CS_PIN) == ABNORMAL) {
      gf_sd_init = FAILED;
      Serial.println(" FAILED");
      Serial.println("  Check SD card connection and format (FAT32)");
  } else {
      gf_sd_init = SUCCESS;
      Serial.println(" SUCCESS");
  }

  /* --- SDカードの初期化が成功した場合、ファイルオープン処理 ----------*/
  if (gf_sd_init == SUCCESS) {
      /* --- csvファイルオープン -------------------------*/
      Serial.print("Opening CSV file: ");
      switch (u1_load_file_num) {
          case 0:
              Serial.print(CSV_FILE_00);
              csv_file = SD.open(CSV_FILE_00);
              break;
          // case 1:   csv_file = SD.open(CSV_FILE_01);  break;
          // case 2:   csv_file = SD.open(CSV_FILE_02);  break;
          default:
              gf_sd_init = FAILED;
              Serial.print("Invalid file number");
      }
      Serial.println();
  }

  if (gf_sd_init == SUCCESS) {
      /* --- csvファイルが開けたかの確認 ------------------*/
      if (csv_file == ABNORMAL) {
          gf_sd_open = FAILED;
          Serial.println("  CSV File open FAILED");
          Serial.println("  Check if file exists on SD card");
      } else {
          gf_sd_open = SUCCESS;
          Serial.print("  CSV File opened successfully, size: ");
          Serial.print(csv_file.size());
          Serial.println(" bytes");
      }
  }
}


/* ------------------------------------------------------  *
 *   全てのcsvファイルを読み込めたか判断
 *  ------------------------------------------------------ */
void judge_csv()
{
    gf_load_finish = SUCCESS;  // 初期化

    /* 全ファイルが読込みできたか判定 */ 
    for (int i=0; i<CSV_FILE_NUM; i++) {
        if (gf_load_state[i] == FAILED) {
            gf_load_finish = FAILED;
        }
    }

    /* デバッグ用 */
    if (gf_load_finish == SUCCESS) {   // 読込み成功
        Serial.println("csv load finish");
    } else {                          // 読込み失敗
        Serial.println("csv load error");
    }
}


/* ------------------------------------------------------  *
 *   時間列:実数から整数に変換
 * ------------------------------------------------------- */
S4 conv_2_time(float val_csv)
{
    S4 s4_val_calc;

    s4_val_calc = (S4)(val_csv * 1000);
    
    return s4_val_calc;
}


/* ------------------------------------------------------  *
 *   呼吸音列:実数から10倍整数に変換（小数点第一位まで保持）
 * ------------------------------------------------------- */
S4 conv_2_breathing(float val_csv)
{
    float val_lim;
    S4 s4_val_calc;

    /* リミッター処理 */
    if (val_csv < INPUT_MIN_BREATH) {        // 最小
        val_lim = INPUT_MIN_BREATH;
    } else if (val_csv > INPUT_MAX_BREATH) { // 最大
        val_lim = INPUT_MAX_BREATH;
    } else {
        val_lim = val_csv;
    }

    s4_val_calc = (S4)(val_lim * (OUTPUT_MAX_BREATH - OUTPUT_MIN_BREATH) / (INPUT_MAX_BREATH - INPUT_MIN_BREATH));

    return s4_val_calc;
}


/* -----------------------------------------------------  *
 *   心音列:実数から整数に変換
 * ------------------------------------------------------- */
S4 conv_2_heartbeat(float val_csv)
{
    float val_lim;
    S4 s4_val_calc;

    /* リミッター処理 */
    if (val_csv < INPUT_MIN_HEART) {        // 最小
        val_lim = INPUT_MIN_HEART;
    } else if (val_csv > INPUT_MAX_HEART) { // 最大
        val_lim = INPUT_MAX_HEART;
    } else {
        val_lim = val_csv;
    }

    s4_val_calc = (S4)(val_lim * (OUTPUT_MAX_HEART - OUTPUT_MIN_HEART) / (INPUT_MAX_HEART - INPUT_MIN_HEART));

    return s4_val_calc;
}


/* ------------------------ *
 *   HeartBeat -> Motor ON 時間に変換
 * ------------------------ */
void get_time_MTON(S4 ts4_time, S4 ts4_HB)
{
    static S4 ts4_time_last[8] = {0};
    static S4 ts4_HB_last[8] = {0};
    static U2 tu2_index = 0;

    ts4_time_last[7] = ts4_time_last[6];  // ↑ 古い
    ts4_time_last[6] = ts4_time_last[5];
    ts4_time_last[5] = ts4_time_last[4];
    ts4_time_last[4] = ts4_time_last[3];
    ts4_time_last[3] = ts4_time_last[2];
    ts4_time_last[2] = ts4_time_last[1];
    ts4_time_last[1] = ts4_time_last[0];
    ts4_time_last[0] = ts4_time;          // ↓ 新しい

    ts4_HB_last[7]   = ts4_HB_last[6];    // ↑ 古い
    ts4_HB_last[6]   = ts4_HB_last[5];
    ts4_HB_last[5]   = ts4_HB_last[4];
    ts4_HB_last[4]   = ts4_HB_last[3];
    ts4_HB_last[3]   = ts4_HB_last[2];
    ts4_HB_last[2]   = ts4_HB_last[1];
    ts4_HB_last[1]   = ts4_HB_last[0];
    ts4_HB_last[0]   = ts4_HB;            // ↓ 新しい

    // if (   (ts4_HB_last[7] <  0)
    //     && (ts4_HB_last[6] <  0)
    //     && (ts4_HB_last[5] <  0)
    //     && (ts4_HB_last[4] <  0)
    //     && (ts4_HB_last[3] >= 0) && (ts4_time_last[3] > 0)
    //     && (ts4_HB_last[2] >  0)
    //     && (ts4_HB_last[1] >  0)
    //     && (ts4_HB_last[0] >  0) ) {

    /*
     *  心音の谷を検出
     */
    // if (   (ts4_HB_last[7] < 0)
    //     && (ts4_HB_last[6] < 0)
    //     && (ts4_HB_last[5] < 0)
    //     && (ts4_HB_last[4] < 0)
    //     && (ts4_HB_last[3] < 0)
    //     && (ts4_HB_last[2] < 0)
    //     && (ts4_HB_last[1] < 0)
    //     && (ts4_HB_last[0] < 0)
    //     && (ts4_HB_last[7] >= ts4_HB_last[3])
    //     && (ts4_HB_last[6] >= ts4_HB_last[3])
    //     && (ts4_HB_last[5] >= ts4_HB_last[3])
    //     && (ts4_HB_last[4] >= ts4_HB_last[3]) && (ts4_time_last[3] > 0)
    //     && (ts4_HB_last[3] <  ts4_HB_last[2])
    //     && (ts4_HB_last[3] <  ts4_HB_last[1])
    //     && (ts4_HB_last[3] <  ts4_HB_last[0]) ) {

    /*
     *  心音の山を検出
     */
    if (   (ts4_HB_last[7] > 0)
        && (ts4_HB_last[6] > 0)
        && (ts4_HB_last[5] > 0)
        && (ts4_HB_last[4] > 0)
        && (ts4_HB_last[3] > 0)
        && (ts4_HB_last[2] > 0)
        && (ts4_HB_last[1] > 0)
        && (ts4_HB_last[0] > 0)
        && (ts4_HB_last[7] <= ts4_HB_last[3])
        && (ts4_HB_last[6] <= ts4_HB_last[3])
        && (ts4_HB_last[5] <= ts4_HB_last[3])
        && (ts4_HB_last[4] <= ts4_HB_last[3]) && (ts4_time_last[3] > 0)
        && (ts4_HB_last[3] >  ts4_HB_last[2])
        && (ts4_HB_last[3] >  ts4_HB_last[1])
        && (ts4_HB_last[3] >  ts4_HB_last[0]) ) {

        u4_time_mt_on[tu2_index] = ts4_time_last[3];
        tu2_index++;
        u2_time_index = tu2_index;

        if (DEBUG_TIME_MTON && tu2_index <= 5) {
            Serial.print("[MTON DETECT] idx:");
            Serial.print(tu2_index - 1);
            Serial.print(" time:");
            Serial.print(ts4_time_last[3]);
            Serial.print(" HB:");
            Serial.println(ts4_HB_last[3]);
        }
    }

}



/* ------------------------------------------------------  *
 *   csvファイルの中身を配列に格納
 * ------------------------------------------------------- */
void read_csv(U1 u1_load_file_num)
{
  U1 f_judge_convert;             // 文字列数字変換が可能か判断
  U1 u1_buf_len;                  // 一行の要素数
  float val_csv;                  // 文字列から変換した実数
  U1 u1_num_start;                // 実数変換前の文字列の開始地点
  U2 u2_row           = 0;        // 格納する配列の行番号
  U1 u1_col           = 0;        // 格納する配列の列番号
  U2 u2_line_count    = 0;        // 読み込んだ行数カウント

  Serial.println("Reading CSV data...");

  if (DEBUG_CSV_CONVERT) {
      Serial.println();
      Serial.println("=== CSV Conversion Debug ===");
      Serial.println("Row | Time(orig) -> Time(ms) | Breathing(orig) -> Breath(conv) | HeartBeat(orig) -> Heart(conv)");
      Serial.println("----|------------------------|----------------------------------|----------------------------------");
  }

  /* csvファイルにまだ読み取り可能な文字(byte)がある場合 */
  while (csv_file.available() > 0){
      u2_line_count++;
      f_judge_convert  = ENABLE;

      /* 一行を抽出 */
      String buf = csv_file.readStringUntil('\n');

      /* 1文字目が数字ではない場合、文字列数字変換が可能 */
      if (isDigit(buf[0]) == CHARACTER) {
          f_judge_convert = DISABLE;
      }
      

      /* 文字列数字変換が可能な場合、変換処理 */
      if (f_judge_convert == ENABLE) {
          /* 文字列の要素数の取得 */
          u1_buf_len = buf.length();

          u1_num_start = 0; // カンマの分割場所のリセット
          u1_col = 0;       // 格納する配列の列数リセット

          // デバッグ用: 元データ保存
          float debug_time_orig = 0.0;
          float debug_breath_orig = 0.0;
          float debug_heart_orig = 0.0;

          /* --- 文字列をカンマで分割し、それぞれの文字列を実数へ変換 ----------------------------- */
          for (int i=0; i<u1_buf_len; i++) {
              /* カンマか行末の場合、実数に変換 */
              if ((buf[i] == ',') || (i == (u1_buf_len - 1))) {
                  val_csv = (buf.substring(u1_num_start, i-1)).toFloat();

                  // デバッグ用: 元データ保存
                  if (DEBUG_CSV_CONVERT) {
                      if (u1_col == EN_TIME) debug_time_orig = val_csv;
                      else if (u1_col == EN_BREATHING) debug_breath_orig = val_csv;
                      else if (u1_col == EN_HEARTBEAT) debug_heart_orig = val_csv;
                  }

                  /* 実数から整数に変換 */
                  switch (u1_col) {
                      case EN_TIME:
                          s4_vital[u1_load_file_num][u2_row][u1_col] = conv_2_time(val_csv);
                          break;
                      case EN_BREATHING:
                          s4_vital[u1_load_file_num][u2_row][u1_col] = conv_2_breathing(val_csv);

                          get_time_SOL_ONOFF( s4_vital[u1_load_file_num][u2_row][EN_TIME],
                                              s4_vital[u1_load_file_num][u2_row][EN_BREATHING] );

                          break;
                      case EN_HEARTBEAT:
                          s4_vital[u1_load_file_num][u2_row][u1_col] = conv_2_heartbeat(val_csv);

                          get_time_MTON( s4_vital[u1_load_file_num][u2_row][EN_TIME],
                                         s4_vital[u1_load_file_num][u2_row][EN_HEARTBEAT] );

                          break;
                      default:
                          // 上記以外の場合は配列vitalに格納しない
                          break;
                  }

                  u1_col++;
                  u1_num_start = i + 1;
              }
          }

          // デバッグ出力: 変換結果表示
          if (DEBUG_CSV_CONVERT) {
              Serial.print(u2_row);
              Serial.print(" | ");
              Serial.print(debug_time_orig, 6);
              Serial.print(" -> ");
              Serial.print(s4_vital[u1_load_file_num][u2_row][EN_TIME]);
              Serial.print(" | ");
              Serial.print(debug_breath_orig, 6);
              Serial.print(" -> ");
              Serial.print(s4_vital[u1_load_file_num][u2_row][EN_BREATHING]);
              Serial.print(" | ");
              Serial.print(debug_heart_orig, 6);
              Serial.print(" -> ");
              Serial.println(s4_vital[u1_load_file_num][u2_row][EN_HEARTBEAT]);
          }

          u2_row++; // U2_CSV_ROWが上限

          // 進捗表示（100行ごと）
          if (u2_row % 100 == 0) {
              Serial.print("  Loaded ");
              Serial.print(u2_row);
              Serial.println(" data rows...");
          }
      }
  }

  Serial.print("CSV reading complete. Total lines: ");
  Serial.print(u2_line_count);
  Serial.print(", Data rows: ");
  Serial.println(u2_row);
}


/* ------------------------ *
 *   Breathing -> SOL ON/OFF 時間検出 (HeartBeat方式)
 *   呼吸音: 10倍整数値で処理
 * ------------------------ */
void get_time_SOL_ONOFF(S4 ts4_time, S4 ts4_breathing)
{
    static S4 ts4_time_last[8] = {0};
    static S4 ts4_breathing_last[8] = {0};
    static U2 tu2_on_index = 0;
    static U2 tu2_off_index = 0;

    ts4_time_last[7] = ts4_time_last[6];  // ↑ 古い
    ts4_time_last[6] = ts4_time_last[5];
    ts4_time_last[5] = ts4_time_last[4];
    ts4_time_last[4] = ts4_time_last[3];
    ts4_time_last[3] = ts4_time_last[2];
    ts4_time_last[2] = ts4_time_last[1];
    ts4_time_last[1] = ts4_time_last[0];
    ts4_time_last[0] = ts4_time;          // ↓ 新しい

    ts4_breathing_last[7] = ts4_breathing_last[6];    // ↑ 古い
    ts4_breathing_last[6] = ts4_breathing_last[5];
    ts4_breathing_last[5] = ts4_breathing_last[4];
    ts4_breathing_last[4] = ts4_breathing_last[3];
    ts4_breathing_last[3] = ts4_breathing_last[2];
    ts4_breathing_last[2] = ts4_breathing_last[1];
    ts4_breathing_last[1] = ts4_breathing_last[0];
    ts4_breathing_last[0] = ts4_breathing;            // ↓ 新しい

    /*
     *  呼吸音の谷を検出 → ソレノイドON (HeartBeat方式：閾値判定なし)
     *  中心点([3])が前後より小さい場合
     */
    if (   (ts4_time_last[3] > 0)
        && (ts4_breathing_last[7] >= ts4_breathing_last[3])
        && (ts4_breathing_last[6] >= ts4_breathing_last[3])
        && (ts4_breathing_last[5] >= ts4_breathing_last[3])
        && (ts4_breathing_last[4] >= ts4_breathing_last[3])
        && (ts4_breathing_last[3] <  ts4_breathing_last[2])
        && (ts4_breathing_last[3] <  ts4_breathing_last[1])
        && (ts4_breathing_last[3] <  ts4_breathing_last[0]) ) {

        u4_time_sol_on[tu2_on_index] = ts4_time_last[3];
        tu2_on_index++;
        u2_sol_on_index = tu2_on_index;
    }

    /*
     *  呼吸音の山を検出 → ソレノイドOFF (HeartBeat方式：閾値判定なし)
     *  中心点([3])が前後より大きい場合
     */
    if (   (ts4_time_last[3] > 0)
        && (ts4_breathing_last[7] <= ts4_breathing_last[3])
        && (ts4_breathing_last[6] <= ts4_breathing_last[3])
        && (ts4_breathing_last[5] <= ts4_breathing_last[3])
        && (ts4_breathing_last[4] <= ts4_breathing_last[3])
        && (ts4_breathing_last[3] >  ts4_breathing_last[2])
        && (ts4_breathing_last[3] >  ts4_breathing_last[1])
        && (ts4_breathing_last[3] >  ts4_breathing_last[0]) ) {

        u4_time_sol_off[tu2_off_index] = ts4_time_last[3];
        tu2_off_index++;
        u2_sol_off_index = tu2_off_index;
    }
}



/* ------------------------ *
 *   csv->配列に変換
 * ------------------------ */
void make_csv_table()
{
    /* csvファイルの読み込み状況を初期化 */
    for (int i=0; i<CSV_FILE_NUM; i++){
        gf_load_state[i] = FAILED;
    }

    /*　定義されたファイル数の読込みが終わっていない場合 */
    for (int i=0; i<CSV_FILE_NUM; i++) {
        gf_sd_init = FAILED;
        gf_sd_open = FAILED;

        /* csvファイルを開く */
        open_csv(i);

        /* SDカードの初期化かオープンが出来ていない場合、処理をスキップ */
        if (   (gf_sd_init == SUCCESS)
            && (gf_sd_open == SUCCESS)) {
            /* csvファイルを読込み、配列に格納 */
            read_csv(i);

            /* csvファイルが読込めたか判定 */
            if (csv_file.available() == 0) {
                gf_load_state[i] = SUCCESS;
            }

            /* csvファイルを閉じる */
            csv_file.close();
        }
    }

    /* 全ファイルが読込みできたか判定*/
    judge_csv();

    /* デバッグ用シリアルモニタ表示 */
    if (DEBUG_CSV_TABLE) {
        for (int i=0; i<CSV_FILE_NUM; i++) {
            Serial.println("=== CSV Data Table ===");
            Serial.println("Row | Time(ms) | Breathing | HeartBeat");
            Serial.println("-----|----------|-----------|----------");

            for (int j=0; j<U2_CSV_ROW; j++) {

                if (   (s4_vital[i][j][EN_TIME] == 0) && (j > 0)) {
                    s4_csv_row_max = (S4)j;
                    f_csv_row_end = ON;
                    break;
                }

                Serial.print(j);
                Serial.print(" | ");

                for (int k=0; k<U1_CSV_COL; k++) {
                    Serial.print(s4_vital[i][j][k]);
                    if (k < U1_CSV_COL - 1) {
                        Serial.print(" | ");
                    }
                }
                Serial.println("");
            }

            Serial.println("-----|----------|-----------|----------");
            Serial.print("Total rows loaded: ");
            Serial.println(s4_csv_row_max);
            Serial.println("");
        }
    }

    /* デバッグ用シリアルモニタ表示 */
    if (DEBUG_TIME_MTON) {
        Serial.println("=== Motor Timing Debug ===");
        for (int i=0; i<u2_time_index; i++) {
              Serial.print(i);
              Serial.print(", ");
              Serial.print(u4_time_mt_on[i]);
              Serial.println();
        }
        Serial.print(" Motor time index: ");
        Serial.print(u2_time_index);
        Serial.println();
    }

    /* ブラダタイミングデバッグ用シリアルモニタ表示 */
    if (DEBUG_TIME_SOL_ON) {
        Serial.println("=== Bladder ON Timing Debug ===");
        for (int i=0; i<u2_sol_on_index; i++) {
              Serial.print("ON[");
              Serial.print(i);
              Serial.print("]: ");
              Serial.println(u4_time_sol_on[i]);
        }
        Serial.print(" Bladder ON index: ");
        Serial.println(u2_sol_on_index);

        Serial.println("=== Bladder OFF Timing Debug ===");
        for (int i=0; i<u2_sol_off_index; i++) {
              Serial.print("OFF[");
              Serial.print(i);
              Serial.print("]: ");
              Serial.println(u4_time_sol_off[i]);
        }
        Serial.print(" Bladder OFF index: ");
        Serial.println(u2_sol_off_index);
    }

    /* CSV読み込みサマリー */
    if (DEBUG_CSV_SUMMARY) {
        Serial.println("");
        Serial.println("========================================");
        Serial.println("       CSV Loading Summary");
        Serial.println("========================================");
        Serial.print("CSV File:          ");
        Serial.println(CSV_FILE_00);
        Serial.print("Total Data Rows:   ");
        Serial.println(s4_csv_row_max);
        Serial.print("Motor Timings:     ");
        Serial.println(u2_time_index);
        Serial.print("Bladder ON Times:  ");
        Serial.println(u2_sol_on_index);
        Serial.print("Bladder OFF Times: ");
        Serial.println(u2_sol_off_index);
        Serial.print("Load Status:       ");
        Serial.println(gf_load_finish == SUCCESS ? "SUCCESS" : "FAILED");
        Serial.println("========================================");
        Serial.println("");
    }
}
