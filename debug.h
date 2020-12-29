#ifdef DEBUG
 #define PRINT(x)  Serial.print (x)
 #define PRINTDEC(x)  Serial.print (x, DEC)
 #define PRINTLN(x)  Serial.println (x)
 #define PRINTDECLN(m,x) Serial.print(m); Serial.print(x,DEC); Serial.println("")
 #ifdef ENABLE_DEBUG_PRINTS
  #define DEBUG_PRINT(x)  Serial.print (x)
  #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_PRINTDECLN(m,x) Serial.print(m); Serial.print(x,DEC); Serial.println("")
 #else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTDECLN(m,x)
 #endif
#else
 #define PRINT(x)
 #define PRINTDEC(x)
 #define PRINTLN(x)
 #define PRINTDECLN(m,x)
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTDECLN(m,x)
#endif
