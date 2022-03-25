/*
  Date and time functions
*/
String getCurrentTime() {
  String extraZeroH, extraZeroM, extraZeroS;
  if (hour() < 10) extraZeroH = '0';
  if (minute() < 10) extraZeroM = '0';
  if (second() < 10) extraZeroS = '0';
  return String(extraZeroH + hour()) + ':' + extraZeroM + minute() + ':' + extraZeroS + second();
}

String getCurrentDate() {
  return(String(day())) + "-" + monthShortStr(month()) + "-" + year();
}

void printTimeDate() {
  terminal.println("-----------------------");
  terminal.println( getCurrentDate() + String(" | ") + getCurrentTime() );
}

void sendUptime ()
{
  uptime::calculateUptime();
  String uptime_format = (String)(uptime::getDays()) + "d:" + (String)(uptime::getHours()  ) + "h:" + (String)(uptime::getMinutes()) + "m" ;
  Blynk.virtualWrite(vPIN_UPTIME,  uptime_format);
}
BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
}

/*
  printOutput() - easy print to terminal with date function
*/
void printOutput(String a) {
  printTimeDate();
  terminal.println(a);
  terminal.flush();
}




/*
  formatTime() - Format millis in to MM:SS
*/
String formatTime(long milliSeconds) {
  long hours = 0, mins = 0, secs = 0;
  String returned, secs_o, mins_o, hours_o;
  secs = milliSeconds / 1000;
  mins = secs / 60;
  hours = mins / 60;
  secs = secs - (mins * 60);
  mins = mins - (hours * 60);
  if (secs < 10 && mins) secs_o = "0";
  if (mins) returned += mins + String("m ");

  returned += secs_o + secs + String("s");
  return returned;
}
