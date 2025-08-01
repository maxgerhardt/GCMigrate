#include "a0_data.h"
#include "arrowcontrol.h"
#include "redrawScreen.h"

void printDash() {
  lcd.print('-');
}

void printSpace() {
  lcd.print("  ");
}

void printOneSpace() {
  lcd.print(' ');
}

void printColon() {
  lcd.print(':');
}

void spaceColon() {
  lcd.print(": ");
}

void clearLine() {
  for (byte i = 0; i < 19; i++) {
    lcd.print(' ');
  }
}

void clearLine2(byte row) {
  lcd.setCursor(0, row);
  for (byte i = 0; i < 20; i++) {
    lcd.print((' '));
  }
}
void printOff() {
  lcd.print("Off");
}
void printOn() {
  lcd.print("On");
}

void drawPlot(int *plot_array) {
#if (USE_PLOTS == 1)
  float max_value = -32000.0;
  float min_value = 32000.0;

  for (byte i = 0; i < 15; i++) {
    float thisVal = plot_array[i] / 7.0;
    if (thisVal > max_value) max_value = thisVal;
    if (thisVal < min_value) min_value = thisVal;
  }

  lcd.setCursor(0, 1); lcd.print(max_value, 1);
  lcd.setCursor(0, 2); lcd.print('>'); lcd.print(plot_array[14] / 7.0, 1);
  lcd.setCursor(0, 3); lcd.print(min_value, 1);

  for (byte i = 0; i < 15; i++) {                  
    float thisVal = plot_array[i] / 7.0;
    int fill_val = thisVal;
    fill_val = constrain(fill_val, min_value, max_value);
    byte infill, fract;
    
    if (thisVal > min_value)
      infill = floor((float)(thisVal - min_value) / (max_value - min_value) * 4 * 10);
    else infill = 0;
    fract = (float)(infill % 10) * 8 / 10;                   
    infill = infill / 10;

    for (byte n = 0; n < 4; n++) {            
      if (n < infill && infill > 0) {         
        lcd.setCursor(5 + i, (3 - n));        
        lcd.write(0);
      }
      if (n >= infill) {                      
        lcd.setCursor(5 + i, (3 - n));
        if (fract > 0) lcd.write(fract);      
        else lcd.write(16);                   
        for (byte k = n + 1; k < 4; k++) {    
          lcd.setCursor(5 + i, (3 - k));
          lcd.write(16);
        }
        break;
      }
    }
  }
#endif
}

void redrawPlot() {
#if (USE_PLOTS == 1)
  int tempPlot[6][15];
  switch (settings.plotMode) {
    case 0: memcpy(tempPlot, sensMinute, 180); break;
    case 1: EEPROM.get(EEPR_PLOT_H, tempPlot); break;
    case 2: EEPROM.get(EEPR_PLOT_D, tempPlot); break;
  }
  drawPlot(tempPlot[debugPage - 1 - PID_AUTOTUNE]);
#endif
  lcd.setCursor(1, 0);
  lcd.print(sensorNames[debugPage - 1 - PID_AUTOTUNE]);
}

#if (PID_AUTOTUNE == 1)
void redrawTuner() {
  //lcd.clear();
  lcd.setCursor(0, 0);
  clearLine();
  lcd.setCursor(0, 0);
  lcd.write(126);
  lcd.print("TUNER: ");
  lcd.print(tuneNames[tuner.status]);
  lcd.setCursor(0, 1);
  lcd.print("t:");
  lcd.print(tuner.cycle / 1000L);
  printSpace();
  lcd.setCursor(7, 1);
  lcd.print("v:");
  lcd.print(tuner.value);
  printSpace();
  lcd.setCursor(13, 1);
  lcd.print("i:");
  lcd.print(tuner.input);
  lcd.setCursor(0, 2);
  lcd.print("min:");
  lcd.print(tuner.min, 1);
  printSpace();
  lcd.setCursor(10, 2);
  lcd.print("max:");
  lcd.print(tuner.max, 1);
  lcd.setCursor(0, 3);
  lcd.print("P:");
  lcd.print(tuner.P, 1);
  printSpace();
  lcd.setCursor(7, 3);
  lcd.print("I:");
  lcd.print(tuner.I);
  printSpace();
  lcd.setCursor(14, 3);
  lcd.print("D:");
  if (tunerSettings.result) {
    printDash();
    printSpace();
  } else {
    if (tuner.D < 100)
      lcd.print(tuner.D, 1);
    else
      lcd.print(tuner.D, 0);
  }
}
#endif

void redrawChannels() {
  

  if (currentLine == 0 || currentLine == 4) {
    lcd.setCursor(1, 0);
    lcd.print(channelNames[currentChannel]);
#if (USE_PID_RELAY == 1)
    if (currentChannel <= 3) lcd.write(42); 
#else
    if (currentChannel == 2 || currentChannel == 3) lcd.write(42); 
#endif
    
    lcd.setCursor(15, 0);
    if (setChannel.state) printOn();
    else printOff();
  }

  
  if (setChannel.state) {
    byte curMode;
    if (currentChannel < 7) curMode = 0;
    else if (currentChannel < 9) curMode = 1;
    else curMode = 2;
#if (SERVO1_RELAY == 1)
    if (currentChannel == 7) curMode = 0;
#endif
#if (SERVO2_RELAY == 1)
    if (currentChannel == 8) curMode = 0;
#endif

    if (currentLine == 1 || currentLine == 4) {
      lcd.setCursor(1, 1);
      clearLine();
      lcd.setCursor(1, 1);
      if (setChannel.relayType != 2) {
        lcd.print(settingsNames[curMode * 3 + 0]); spaceColon();
        lcd.print(modeNames[setChannel.mode]);
        if (setChannel.global && setChannel.mode != 2) lcd.print(F("+W"));
      } else {
        lcd.print("---");
      }
    }

    if (currentLine == 2 || currentLine == 4) {
      lcd.setCursor(1, 2);
      clearLine();
      lcd.setCursor(1, 2);
      lcd.print(settingsNames[curMode * 3 + 1]); spaceColon();
      lcd.print(directionNames[setChannel.direction + curMode * 2]);
    }
    if (currentLine == 3 || currentLine == 4) {
      lcd.setCursor(1, 3);
      clearLine();
      lcd.setCursor(1, 3);
      lcd.print(settingsNames[curMode * 3 + 2]); spaceColon();

      if (curMode == 0) {
        if (setChannel.mode < 4)
          lcd.print(relayNames[setChannel.relayType]);
        else
          lcd.print("---");
      }
#if (SERVO1_RELAY == 0 || SERVO2_RELAY == 0)
      if (curMode == 1) {
        lcd.setCursor(11, 3);
        if (currentChannel == 7) {
#if (SERVO1_RELAY == 0)
          lcd.print(settings.minAngle[0]);
          lcd.setCursor(16, 3);
          lcd.print(settings.maxAngle[0]);
#endif
        } else if (currentChannel == 8) {
#if (SERVO2_RELAY == 0)
          lcd.print(settings.minAngle[1]);
          lcd.setCursor(16, 3);
          lcd.print(settings.maxAngle[1]);
#endif
        }
      }
#endif
      if (curMode == 2) {
        //lcd.setCursor(11, 3);
        lcd.print(settings.driveTimeout / 10.0, 1);
      }
    }
  }
}

void redrawSettings() {
  byte curSetMode = setChannel.mode;
  curPWMchannel = channelToPWM[currentChannel];
  if (curSetMode < 4) {
    if (currentLine == 4) printName();
    if (currentLine == 1 || currentLine == 4) {
      lcd.setCursor(1, 1);
      clearLine();
      lcd.setCursor(1, 1);

      switch (curSetMode) {
        case 0: lcd.print(modeSettingsNames[0]); spaceColon();
          lcd.setCursor(8, 1);
          if (thisH[0] < 10) lcd.print(F("00"));
          if (thisH[0] >= 10 && thisH[0] < 100) lcd.print(0);
          lcd.print((byte)(thisH[0]));
          printColon();
          if (thisM[0] < 10) lcd.print(0);
          lcd.print((byte)(thisM[0]));
          printColon();
          if (thisS[0] < 10) lcd.print(0);
          lcd.print((byte)(thisS[0]));
          break;
        case 1: lcd.print(modeSettingsNames[3]); spaceColon();
          lcd.print((int)(impulsePrds[setChannel.impulsePrd]));
          if (setChannel.impulsePrd < 6)
            lcd.print(F("m "));
          else if (setChannel.impulsePrd < 13)
            lcd.print(F("h "));
          else
            lcd.print(F("d "));
          break;
        case 2:
          lcd.setCursor(0, 1);
          lcd.print(F("Days "));
          for (byte i = 0; i < 7; i++) {
            lcd.print(bitRead(setChannel.week, i));
            lcd.print(" ");
          }
          break;
        case 3: lcd.print(modeSettingsNames[8]); spaceColon();
          if (setChannel.sensPeriod < 60) {
            lcd.print(setChannel.sensPeriod);
            lcd.print(F("s "));
          } else {
            lcd.print((float)setChannel.sensPeriod / 60, 1);
            lcd.print(F("m "));
          }
          break;
      }
    }

    if (currentLine == 2 || currentLine == 4) {
      lcd.setCursor(1, 2);
      clearLine();
      lcd.setCursor(1, 2);

      switch (curSetMode) {
        case 0: lcd.print(modeSettingsNames[1]); spaceColon();
          lcd.setCursor(9, 2);
          if (thisH[1] < 10) lcd.print(0);
          lcd.print((byte)(thisH[1]));
          printColon();
          if (thisM[1] < 10) lcd.print(0);
          lcd.print((byte)(thisM[1]));
          printColon();
          if (thisS[1] < 10) lcd.print(0);
          lcd.print((byte)(thisS[1]));
          break;
        case 1: lcd.print(modeSettingsNames[4]); spaceColon();
          if (setChannel.work < 60) {
            lcd.print(setChannel.work);
            lcd.print(F("s "));
          } else {
            lcd.print(setChannel.work / 60);
            lcd.print(F("m "));
          }
          break;
        case 2:
          lcd.setCursor(0, 2);
          printOn();
          lcd.setCursor(4, 2);
          if (thisH[0] < 10) lcd.print(" ");
          lcd.print((byte)(thisH[0]));
          printColon();
          if (thisM[0] < 10) lcd.print(0);
          lcd.print((byte)(thisM[0]));
          printColon();
          if (thisS[0] < 10) lcd.print(0);
          lcd.print((byte)(thisS[0]));
          break;
        case 3: lcd.print(modeSettingsNames[9]); spaceColon();
          lcd.print(sensorNames[setChannel.sensor]);
          lcd.print(F(" "));
          if (setChannel.sensor==6)
          lcd.print(sensorVals[setChannel.sensor]/100);
        else
          lcd.print((int)(sensorVals[setChannel.sensor]));;
          
#if (DEBUG_ENABLE==1)
          Serial.print("setChannel.sensor ="); Serial.println(setChannel.sensor); 
#endif            


          break;
      }
    }

    if (currentLine == 3 || currentLine == 4) {
      lcd.setCursor(1, 3);
      clearLine();
      lcd.setCursor(1, 3);
      switch (curSetMode) {
        case 0:
          lcd.print(modeSettingsNames[2]); spaceColon();
          if (setChannel.period > 0) {
            long period = setChannel.period - (millis() - timerMillis[currentChannel]) / 1000L;
            byte leftH = floor((long)period / 3600);    
            byte leftM = floor((period - (long)leftH * 3600) / 60);
            byte leftS = period - (long)leftH * 3600 - leftM * 60;

            lcd.print(leftH);
            printColon();
            lcd.print(leftM);
            printColon();
            lcd.print(leftS);
          }
          break;
        case 1:
          lcd.print(modeSettingsNames[5]); spaceColon();
          lcd.print(setChannel.startHour);
          lcd.print(F(" h "));
          break;
        case 2:
          lcd.setCursor(0, 3);
          printOff();
          lcd.setCursor(4, 3);
          if (thisH[1] < 10) lcd.print(' ');
          lcd.print((byte)(thisH[1]));
          printColon();
          if (thisM[1] < 10) lcd.print(0);
          lcd.print((byte)(thisM[1]));
          printColon();
          if (thisS[1] < 10) lcd.print(0);
          lcd.print((byte)(thisS[1]));

          lcd.setCursor(13, 3);
          lcd.print(F("Glob"));
          lcd.setCursor(18, 3);
          lcd.print(setChannel.global);
          break;
        case 3:
          lcd.print("min:");
          lcd.print(setChannel.threshold);
          lcd.setCursor(11, 3);
          lcd.print("max:");
          lcd.print(setChannel.thresholdMax);
#if (DEBUG_ENABLE == 1)          
          Serial.print("min:");Serial.println(setChannel.threshold);
          Serial.print("max:");Serial.println(setChannel.thresholdMax);
#endif
      }
    }
  } else {    
    currentLine = 0;
    byte thisAmount;  
    if (curSetMode == 4) thisAmount = PID_SET_AMOUNT;
    else if (curSetMode == 5) thisAmount = DAWN_SET_AMOUNT;

    int8_t screenPos = (arrowPos - 1) / 4;
    if (lastScreen != screenPos) lcd.clear();
    lastScreen = screenPos;

    if (screenPos < 1) printName();
    if (arrowPos == 0) arrow(0, 0);
    else space(0, 0);
    if (arrowPos == 1) arrow(14, 0);
    else space(14, 0);

    for (byte i = 0; i < 4; i++) {  
      if (arrowPos <= 1 && i == 0) continue;

      lcd.setCursor(0, i);          
      int8_t index = 4 * screenPos + i - 1; 
      smartArrow(arrowPos == 4 * screenPos + i + 1); 
      if (index >= thisAmount) break; 

      
      if (index >= 0) {
        if (curSetMode == 4) {
#if (USE_PID == 1)
          lcd.print(pidNames[index]); spaceColon();
          byte driveOffset = 1;
          if (currentChannel == 9) driveOffset = 100;
          switch (index) {
            case 0: lcd.print(setPID.kP, 2); break;
            case 1: lcd.print(setPID.kI, 2); break;
            case 2: lcd.print(setPID.kD, 2); break;
            case 3: lcd.print(sensorNames[setPID.sensor]); break;
            case 4: lcd.print(setPID.setpoint, 1); break;
            case 5: lcd.print(setPID.dT); break;
            case 6: lcd.print(((float)setPID.minSignal / driveOffset), 1); break;
            case 7: lcd.print(((float)setPID.maxSignal / driveOffset), 1); break;
          }
#endif
        }
        if (curSetMode == 5) {
#if (USE_DAWN == 1)
          lcd.print(dawnNames[index]); spaceColon();
          switch (index) {
            case 0: lcd.print(setDawn.start); lcd.print('h'); break;
            case 1: lcd.print(setDawn.dur1); lcd.print('m'); break;
            case 2: lcd.print(setDawn.stop); lcd.print('h'); break;
            case 3: lcd.print(setDawn.dur2); lcd.print('m'); break;
            case 4: lcd.print(setDawn.minV); break;
            case 5: lcd.print(setDawn.maxV); break;
          }
#endif
        }
        printSpace();
      }
    }
  }
}

void printName() {
  lcd.setCursor(1, 0);
  lcd.print(modeNames[setChannel.mode]);
  if (setChannel.global && setChannel.mode != 2) lcd.print(F("+W"));
  lcd.setCursor(15, 0); lcd.print(F("Back"));
}


#define POS_1 4
#define POS_2 9

#if (USE_CO2 == 1)
#if (CO2_PIN == 2)
#if (DALLAS_SENS1 == 1)
#define POS_1 3
#define POS_2 8
#else
#define POS_1 4
#define POS_2 8
#endif
#endif
#endif

void redrawDebug() {
  lcd.setCursor(1, 0); lcd.print(F("DEBUG"));
  lcd.setCursor(7, 0); lcd.print(F("S1:"));
  if (loadChannel(7).state) {

#if (SERVO1_RELAY == 0)
    if (loadChannel(7).mode >= 4) {
      lcd.print(pwmVal[4]); printSpace();
    } else {
      if (channelStates[7]) lcd.print(F("Max"));
      else lcd.print(F("Min"));
    }
#else
    lcd.print(channelStates[7] ^ !loadChannel(7).direction);
#endif

  }
  else printDash();

  lcd.setCursor(14, 0); lcd.print(F("S2:"));
  if (loadChannel(8).state) {

#if (SERVO2_RELAY == 0)
    if (loadChannel(8).mode >= 4) {
      lcd.print(pwmVal[5]); printSpace();
    } else {
      if (channelStates[8]) lcd.print(F("Max"));
      else lcd.print(F("Min"));
    }
#else
    lcd.print(channelStates[8] ^ !loadChannel(8).direction);
#endif
  }
  else printDash();

  
  
  //clearLine2(1);
  lcd.setCursor(0, 1); lcd.print((int)(sensorVals[1])); lcd.write(37); printSpace();

  lcd.setCursor(POS_1, 1);
#if (DALLAS_SENS1 == 1)
  
#if (DALLAS_AMOUNT > 1)

#if (DALLAS_DISP == 0)
  lcd.print(sensorVals[2], 1);
#elif (DALLAS_DISP == 1)
  static byte counter = 0;
  if (counter % 2 == 0) {
    lcd.print(counter / 2 + 1);
    printSpace(); 
    printSpace();
  }
  else lcd.print(dallasBuf[counter / 2], 1);
  counter++;
  if (counter >= DALLAS_AMOUNT * 2) counter = 0;
#endif

#else
  lcd.print(sensorVals[2], 1);
#endif

#else
  
  lcd.print((int)sensorVals[2]);
#endif

#if (THERM1 == 1 || DALLAS_SENS1 == 1)
  lcd.write(223);
#endif
  printSpace();

  // 3 dht
  lcd.setCursor(POS_2, 1);

// #if (DHT_SENS2 == 1)
//   printDash();
// #else
  
//   lcd.print((int)sensorVals[3]);
// #endif

#if (THERM2 == 1)
  lcd.write(223);
#endif
  printSpace();
  
  lcd.setCursor(13, 1); lcd.print((int)sensorVals[4]);
#if (THERM3 == 1)
  lcd.write(223);
#endif
  printSpace();
  
  lcd.setCursor(17, 1); lcd.print((int)sensorVals[5]);
#if (THERM4 == 1)
  lcd.write(223);
  if ((int)sensorVals[5] < 10) lcd.print(' ');
#else
  if ((int)sensorVals[5] < 10) printSpace();
  else if ((int)sensorVals[5] < 100) lcd.print(' ');
#endif

  lcd.setCursor(0, 2); lcd.print(sensorVals[0], 1); lcd.write(223); printSpace();
  lcd.setCursor(6, 2); lcd.print(F("R:"));
  for (byte i = 0; i < 8; i++) {
    channelsStruct temp = loadChannel(i);
    if (temp.state) {
#if (USE_PID == 1 || USE_DAWN == 1)
      if (temp.mode == 4 || temp.mode == 5) {
        byte thisHeight;
        thisHeight = round(pwmVal[i] / 32); 

        if (thisHeight == 0) lcd.write(32);   
        else if (thisHeight < 8) lcd.write(thisHeight + 1); 
        else lcd.write(0);          

      } else
#endif
      {
        lcd.print((bool)channelStates[i] ^ !temp.direction);
      }
    }
    else printDash();
  }

  lcd.setCursor(15, 2); lcd.print(F("D:"));
  if (loadChannel(9).state) {
    if (loadChannel(9).mode != 4) lcd.print(lastDriveState);
    else lcd.print(pwmVal[6]);    
  }
  else printDash();

  lcd.setCursor(0, 3); lcd.print(sensorVals[6]/100, 2); lcd.print("hP"); printSpace();
  // lcd.setCursor(0, 3);
  // if (realTime[0] < 10) lcd.print(F(" "));
  // lcd.print((byte)(realTime[0])); printColon();
  // if (realTime[1] < 10) lcd.print(0);
  // lcd.print((byte)(realTime[1])); printColon();
  // if (realTime[2] < 10) lcd.print(0);
  // lcd.print((byte)(realTime[2]));
  // lcd.setCursor(9, 3);
  // byte today = rtc.getDay();
  // if (today == 0) today = 7;
  // lcd.print(today);
  lcd.setCursor(12, 3); lcd.print(F("U:")); lcd.print(uptime);
}

void redrawMainSettings() {
  currentLine = 0;
  byte screenPos = arrowPos / 4;
  if (lastScreen != screenPos) lcd.clear();
  lastScreen = screenPos;

  for (byte i = 0; i < 4; i++) {  
    int8_t index = 4 * screenPos + i - 1; 
    lcd.setCursor(0, i);          
    smartArrow(arrowPos == 4 * screenPos + i); 

    if (screenPos == 0 && i == 0) lcd.print(F("SETTINGS"));

    
    if (index >= SETTINGS_AMOUNT) break;

    
    if (index >= 0) {
      lcd.print(settingsPageNames[index]);
      spaceColon();
      switch (index) {
        case 0: 
          if (settings.backlight) {
            printOn();
          } else {
            printOff();
          }
          break;
        case 1: lcd.print(settings.backlTime); break;
        case 2: lcd.print(settings.drvSpeed); break;
        case 3: lcd.print(rtc.getDate()); break;
        case 4: lcd.print(rtc.getMonth()); break;
        case 5: lcd.print(rtc.getYear()); break;
        case 6: lcd.print(settings.comSensPeriod); break;
#if (USE_PLOTS==1)
        case 7: lcd.print(plotNames[settings.plotMode]); break;
#endif
#if (SMOOTH_SERVO == 1)
        case 8: lcd.print(settings.srv1_Speed); break;
        case 9: lcd.print(settings.srv1_Acc, 1); break;
        case 10: lcd.print(settings.srv2_Speed); break;
        case 11: lcd.print(settings.srv2_Acc, 1); break;
#endif
#if (PID_AUTOTUNE == 1)
        case 8: tunerSettings.tuner ? printOn() : printOff();  break;
        case 9: tunerSettings.result ? lcd.print("PI") : lcd.print("PID"); break;
        case 10:
          switch (tunerSettings.channel) {
            case 0: printOff(); break;
            case 1:
            case 2:
            case 3:
            case 4: lcd.print(tunerSettings.channel); break;
            case 5: lcd.print("S1"); break;
            case 6: lcd.print("S2"); break;
            case 7: lcd.print("Dr"); break;
          }
          break;
        case 11: lcd.print(sensorNames[tunerSettings.sensor]);
          break;
        case 12: tunerSettings.manual ? lcd.print("yes") : lcd.print("no"); break;
        case 13: lcd.print(tunerSettings.steady); break;
        case 14: lcd.print(tunerSettings.step); break;
        case 15: lcd.print(tunerSettings.window); break;
        case 16: lcd.print(tunerSettings.kickTime); break;
        case 17: lcd.print(tunerSettings.delay); break;
        case 18: lcd.print(tunerSettings.period); break;
#endif
      }
      printSpace();
    }
  }
}

#if (SCHEDULE_NUM > 0)
void redrawSchedule() {
  currentLine = 0;
  int8_t schChannel = currentChannel - 10;
  byte screenPos = arrowPos / 4;
  if (lastScreen != screenPos) lcd.clear();
  lastScreen = screenPos;

  for (byte i = 0; i < 4; i++) {  
    int8_t index = 4 * screenPos + i - 1; 
    

    if (index >= 6 + setSchedule.pointAmount) break;
    lcd.setCursor(0, i);          
    smartArrow(arrowPos == 4 * screenPos + i); 
    if (screenPos == 0 && i == 0) {
      lcd.print(F("SCHEDULE "));
      lcd.print(schChannel + 1);
    }

    
    if (index >= 0 && index < 4) {
      lcd.print(schedulePageNames[index]);
      spaceColon();
      switch (index) {
        case 0:
          switch (setSchedule.pidChannel) {
            case 0: printOff(); break;
            case 1:
            case 2:
            case 3:
            case 4: lcd.print(setSchedule.pidChannel); break;
            case 5: lcd.print("S1"); break;
            case 6: lcd.print("S2"); break;
            case 7: lcd.print("Dr"); break;
          }
          break;
        case 1: if (setSchedule.startDay > 0) printDate(setSchedule.startDay - 1);
          else lcd.print("every");
          printSpace();
          break;
        case 2: if (setSchedule.startDay > 0) printDate(setSchedule.endDay - 1);
          else printDash();
          break;
        case 3: lcd.print(setSchedule.pointAmount); break;
      }
    } else if (index >= 4) {
      index -= 4;
      if (index < setSchedule.pointAmount) {
        lcd.print("Point ");
        lcd.print(index + 1);
        spaceColon();
        lcd.print(setSchedule.setpoints[index] / 10.0f, 1);
      }
    }
    printSpace();
  }
}

void printDate(int day365) {
  int curDay = 0;
  byte month, day;
  for (byte i = 0; i < 12; i++) {
    curDay += daysMonth[i];
    if (curDay > day365) {
      curDay -= daysMonth[i];
      month = i + 1;
      break;
    }
  }
  day = day365 - curDay + 1;
  lcd.print(day);
  lcd.print('.');
  lcd.print(month);
}
#endif

void redrawService() {
  if (currentLine == 4) lcd.clear();

  if (currentLine == 0 || currentLine == 4) {
    clearLine2(0);
    lcd.setCursor(1, 0);
#if (START_MENU == 0)
    lcd.print(F("SERVICE"));
#else
    lcd.print(F("Back"));
#endif

    lcd.setCursor(12, 0);
    if (realTime[0] < 10) lcd.print(' ');
    lcd.print((byte)(realTime[0])); printColon();
    if (realTime[1] < 10) lcd.print(0);
    lcd.print((byte)(realTime[1])); printColon();
    if (realTime[2] < 10) lcd.print(0);
    lcd.print((byte)(realTime[2]));
  }

  if (currentLine == 1 || currentLine == 4) {
    clearLine2(1);
    lcd.setCursor(0, 1); lcd.print(F("R1 2 3 4 5 6 7"));
  }
  if (currentLine == 2 || currentLine == 4) {
    clearLine2(2);
    lcd.setCursor(1, 2); for (byte i = 0; i < 7; i++) {
      lcd.print(channelStatesServ[i] ^ !loadChannel(i).direction);
      lcd.print(' ');
    }
  }
  if (currentLine == 3 || currentLine == 4) {
    clearLine2(3);
    lcd.setCursor(0, 3); lcd.print(F("D:"));
    lcd.setCursor(2, 3);
    if (channelStatesServ[9]) lcd.print(F("OPEN"));
    else lcd.print(F("CLOS"));

    lcd.setCursor(7, 3); lcd.print(F("S1:"));
    if (SERVO1_RELAY) {   
      lcd.print(channelStatesServ[7]);
    } else {
      lcd.print((byte)(servoPosServ[0]));
    }

    lcd.setCursor(14, 3); lcd.print(("S2:"));
    if (SERVO2_RELAY) {   
      lcd.print(channelStatesServ[8]);
    } else {
      lcd.print((byte)(servoPosServ[1]));
    }
  }
}

void redrawScreen() {
  if (currentLine == 4) {
    lcd.clear();
  }

  if (navDepth == 0) {            
    if (currentChannel == -3) redrawService();
    else if (currentChannel == -2) redrawMainSettings();
    else if (currentChannel == -1) redrawDebug();
    else if (currentChannel < 10) redrawChannels();
#if (SCHEDULE_NUM > 0)
    else redrawSchedule();
#endif
  } else {
    redrawSettings();
  }
  drawArrow();
}
