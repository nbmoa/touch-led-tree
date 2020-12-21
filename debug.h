#ifdef DEBUG
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
