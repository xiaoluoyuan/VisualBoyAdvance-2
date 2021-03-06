#ifndef RTC_H
#define RTC_H

u16 rtcRead(u32 address);
bool rtcWrite(u32 address, u16 value);
void rtcEnable(bool);
bool rtcIsEnabled();
void rtcReset();

#endif // RTC_H
