#include "a0_data.h"
#include <Arduino.h>
#include "automatics.h"



void timersTick() {   
  
  realTime[0] = rtc.getHours();
  realTime[1] = rtc.getMinutes();
  realTime[2] = rtc.getSeconds();
  getDay();   
  uptime += (float)0.0000115741;   

  for (byte curChannel = 0; curChannel < 10; curChannel++) {
    activeChannel = loadChannel(curChannel);
    if (activeChannel.state                                                             
        && (curChannel >= 7 || ((curChannel < 7 || !activeChannel.type) && activeChannel.relayType != 2) ) ) {   
      switch (activeChannel.mode) {
        case 0:   
          if (millis() - timerMillis[curChannel] >= activeChannel.period * 1000
              && channelStates[curChannel] != activeChannel.direction) {
            timerMillis[curChannel] = millis();
            channelStates[curChannel] = activeChannel.direction;
          }
          if (millis() - timerMillis[curChannel] >= activeChannel.work * 1000
              && channelStates[curChannel] == activeChannel.direction) {
            channelStates[curChannel] = !activeChannel.direction;
          }
          break;
        case 1:   
          if (realTime[2] == 0 || realTime[2] == 1) { 
            if (activeChannel.impulsePrd < 6) {                              
              if (channelStates[curChannel] != activeChannel.direction) {    
                byte waterTime = 0;                                                 
                for (byte j = 0; j < 60 / impulsePrds[activeChannel.impulsePrd]; j++) {
                  if (realTime[1] == waterTime) {
                    channelStates[curChannel] = activeChannel.direction;     
                    timerMillis[curChannel] = millis();                             
                  }
                  waterTime += impulsePrds[activeChannel.impulsePrd];
                }
              }
            } else if (activeChannel.impulsePrd < 13) {                        
              if (realTime[1] == 0 && (realTime[2] == 0 || realTime[2] == 1)) { 
                if (channelStates[curChannel] != activeChannel.direction) {    
                  byte waterTime = activeChannel.startHour;                    
                  for (byte j = 0; j < 24 / impulsePrds[activeChannel.impulsePrd]; j++) {
                    if (waterTime < 24) {
                      if (realTime[0] == waterTime) {
                        channelStates[curChannel] = activeChannel.direction;   
                        timerMillis[curChannel] = millis();                           
                      }
                    } else {
                      if (realTime[0] == waterTime - 24) {
                        channelStates[curChannel] = activeChannel.direction;   
                        timerMillis[curChannel] = millis();                           
                      }
                    }
                    waterTime += impulsePrds[activeChannel.impulsePrd];
                  }
                }
              }
            } else {        
              if (realTime[1] == 0 && (realTime[2] == 0 || realTime[2] == 1)) {
                if (channelStates[curChannel] != activeChannel.direction) {    
                  if (thisDay % impulsePrds[activeChannel.impulsePrd] == 0  
                      && realTime[0] == activeChannel.startHour) { 
                    channelStates[curChannel] = activeChannel.direction;   
                    timerMillis[curChannel] = millis();                           
                  }
                }
              }
            }
          }

          if (channelStates[curChannel] == activeChannel.direction                     
              && millis() - timerMillis[curChannel] >= activeChannel.work * 1000) {    
            channelStates[curChannel] = !activeChannel.direction;                      
          }
          break;
        case 2:   
          if (checkDay(curChannel)) channelStates[curChannel] = activeChannel.direction;     
          else channelStates[curChannel] = !activeChannel.direction;    
          break;
        case 3:   
          if ((int16_t)(millis() - timerMillis[curChannel]) >= activeChannel.sensPeriod * 1000L) {
            timerMillis[curChannel] = millis();

            /*
              флаг и laststate на каждый канал, +20 байт
              если (laststate != checkHysteresis) {
                если (!флаг) {
                  сброс таймера
                  флаг = 1
                } иначе {
                  laststate = checkHysteresis
                  стейт меняем
                  флаг = 0
                }
              }
            */
            byte result = checkHysteresis(curChannel);
            if (result == 1) channelStates[curChannel] = activeChannel.direction;
            else if (result == 2) channelStates[curChannel] = !activeChannel.direction;
          }
          break;
        case 4:   
          break;
        case 5:   
#if (USE_DAWN == 1)
          if ((realTime[2] % 5) == 0 || startFlagDawn) {
            checkDawn(curChannel);
          }
#endif
          break;
      }
      
      if (activeChannel.global && !checkDay(curChannel)) channelStates[curChannel] = !activeChannel.direction;
    }
  }
  startFlagDawn = false;
  

  
  boolean atLeastOneValve = false;                              
  
  for (byte relay = 0; relay < 9; relay++) {
    if ((relay == 7 && !SERVO1_RELAY) || (relay == 8 && !SERVO2_RELAY)) continue;
    channelsStruct thisCh = loadChannel(relay);
    if (thisCh.state && thisCh.mode < 4) {
      if (thisCh.relayType == 1                                   
          && channelStates[relay] == thisCh.direction)            
        atLeastOneValve = true;                                   
      digitalWrite(relayPins[relay], channelStates[relay]);       
    }
  }

  for (byte relay = 0; relay < 9; relay++) {
    if ((relay == 7 && !SERVO1_RELAY) || (relay == 8 && !SERVO2_RELAY)) continue;
    channelsStruct thisCh = loadChannel(relay);
    if (thisCh.state && thisCh.relayType == 2 && thisCh.mode < 4) {  
      if (atLeastOneValve) channelStates[relay] = thisCh.direction;  
      else channelStates[relay] = !thisCh.direction;                 
      digitalWrite(relayPins[relay], channelStates[relay]);          
    }
  }

#if (PID_AUTOTUNE == 1)
  tunePID();
  if (tunerSettings.channel > 0) {
    if (tunerSettings.manual) {
      pwmVal[tunerSettings.channel - 1] = tunerSettings.steady;
    } else {
      if (tunerSettings.tuner) pwmVal[tunerSettings.channel - 1] = tuner.value;
    }
  }
#endif

#if (USE_PID_RELAY == 1)
  
  if (loadChannel(0).mode == 4) {
    PWMactive[0] = (long)pwmVal[0] * PWMperiod / 255;
  }

  if (loadChannel(1).mode == 4) {
    PWMactive[1] = (long)pwmVal[1] * PWMperiod / 255;
  }
#endif

  
  if (loadChannel(2).mode >= 4) {
    analogWrite(RELAY_2, pwmVal[2]);
  }
  if (loadChannel(3).mode >= 4) {
    analogWrite(RELAY_3, pwmVal[3]);
  }

  
  activeChannel = loadChannel(7);
#if (SERVO1_RELAY == 1)   
  digitalWrite(SERVO_0, channelStates[7]);
#else   
  if (activeChannel.mode != 4 && activeChannel.mode != 5) {   
    pwmVal[4] = channelStates[7] ? settings.minAngle[0] : settings.maxAngle[0];
  }

#if (SMOOTH_SERVO == 1)
  if (activeChannel.state) servo1.setTargetDeg(activeChannel.direction ? pwmVal[4] : (180 - pwmVal[4]));
#else
  if (activeChannel.state) servo1.write(activeChannel.direction ? pwmVal[4] : (180 - pwmVal[4]));
#endif

#endif

  activeChannel = loadChannel(8);
#if (SERVO2_RELAY == 1)   
  digitalWrite(SERVO_1, channelStates[8]);
#else
  if (activeChannel.mode != 4 && activeChannel.mode != 5) {   
    pwmVal[5] = channelStates[8] ? settings.minAngle[1] : settings.maxAngle[1];
  }

#if (SMOOTH_SERVO == 1)
  if (activeChannel.state) servo2.setTargetDeg(activeChannel.direction ? pwmVal[5] : (180 - pwmVal[5]));
#else
  if (activeChannel.state) servo2.write(activeChannel.direction ? pwmVal[5] : (180 - pwmVal[5]));
#endif

#endif


#if (USE_DRIVE == 1)
  
  if (lastDriveState != channelStates[9] && millis() - driveTimer >= DRIVE_TOUT * 1000L) {
    lastDriveState = channelStates[9];
    driveState = 1;
  }
#endif

#if (SCHEDULE_NUM > 0)
  checkShedule();   
#endif
}

boolean checkDay(byte channel) {
  byte today = rtc.getDay();
  if (today == 0) today = 7;
  long thisTime = ((long)realTime[0] * 3600 + realTime[1] * 60 + realTime[2]);
  channelsStruct temp = loadChannel(channel);
  if (bitRead(temp.week, today - 1) &&
      ((temp.weekOff > temp.weekOn) ?
       (thisTime > (long)temp.weekOn && thisTime < (long)temp.weekOff) :
       (thisTime > (long)temp.weekOn || thisTime < (long)temp.weekOff))) return true;
  else return false;
}

byte checkHysteresis(byte channel) {
  byte state = 0;
  channelsStruct temp = loadChannel(channel);
  if (temp.thresholdMax > temp.threshold) {
    
    if (sensorVals[temp.sensor] >= temp.thresholdMax)
      state = 1;

    
    if (sensorVals[temp.sensor] < temp.threshold)
      state = 2;
  } else {
    
    if (sensorVals[temp.sensor] >= temp.threshold)
      state = 2;

    
    if (sensorVals[temp.sensor] < temp.thresholdMax)
      state = 1;
  }
  return state;
}

void checkDawn(byte curChannel) {
#if (USE_DAWN == 1)
  if (curChannel == 2 || curChannel == 3 || curChannel == 7 || curChannel == 8) {
    byte curPWMchannel = channelToPWM[curChannel];
    activeDawn = loadDawn(curPWMchannel);
    long nowTime = realTime[0] * 60 * 60L + realTime[1] * 60L + realTime[2];
    long timeStart = activeDawn.start * 60 * 60L;
    long timeStop = activeDawn.stop * 60 * 60L;
    byte thisSignal;
    byte thisMin = activeChannel.direction ? activeDawn.minV : activeDawn.maxV;
    byte thisMax = activeChannel.direction ? activeDawn.maxV : activeDawn.minV;

    if (nowTime < (timeStart)) {   
      thisSignal = thisMin;
    } else if (nowTime < (timeStart + activeDawn.dur1 * 60)) { 
      thisSignal = map((nowTime - timeStart), 0, activeDawn.dur1 * 60, thisMin, thisMax);
    } else if (nowTime < (timeStop)) {  
      thisSignal = thisMax;
    } else if (nowTime < (timeStop + activeDawn.dur2 * 60)) { 
      thisSignal = map((nowTime - timeStop), 0, activeDawn.dur2 * 60, thisMax, thisMin);
    } else {  
      thisSignal = thisMin;
    }
    pwmVal[curPWMchannel] = thisSignal;
  }
#endif
}




void getDay() {
  uint16_t days = rtc.getDate();
  for (byte i = 0; i < rtc.getMonth() - 1; i++)
    days += daysMonth[i];

  if (rtc.getMonth() > 2 && rtc.getYear() % 4 == 0) 
    ++days;
  thisDay = days;
}

#if (SCHEDULE_NUM > 0)
void checkShedule() {
  int thisDayFromZero = thisDay - 1;
  for (byte i = 0; i < SCHEDULE_NUM; i++) {
    activeSchedule = loadSchedule(i);
    if (activeSchedule.pidChannel > 0) {  
      byte idx;
      if (activeSchedule.startDay > 0) {   
        if (activeSchedule.startDay < activeSchedule.endDay) {  
          if (thisDayFromZero < activeSchedule.startDay) idx = 0;
          else if (thisDayFromZero >= activeSchedule.endDay) idx = activeSchedule.pointAmount - 1;
          else idx = activeSchedule.pointAmount * (thisDayFromZero - activeSchedule.startDay) / (activeSchedule.endDay - activeSchedule.startDay);
        } else {          
          byte dayNum = (thisDayFromZero < 365 && thisDayFromZero > activeSchedule.endDay) ? (thisDayFromZero - activeSchedule.startDay) : (365 - activeSchedule.startDay + thisDayFromZero);
          if (thisDayFromZero >= activeSchedule.endDay && thisDayFromZero < activeSchedule.startDay) idx = 0;
          else idx = activeSchedule.pointAmount * dayNum / (activeSchedule.endDay + 365 - activeSchedule.startDay);
        }
      } else {  
        idx = activeSchedule.pointAmount * realTime[0] / 24;
      }
      activePID = loadPID(activeSchedule.pidChannel - 1);
      activePID.setpoint = activeSchedule.setpoints[idx] / 10.0f;
      savePID(activeSchedule.pidChannel - 1);
    }
  }
}
#endif
