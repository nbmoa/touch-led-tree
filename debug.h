#ifdef DEBUG
 #define PRINT(x)  Serial.print (x)
 #define PRINTDEC(x)  Serial.print (x, DEC)
 #define PRINTLN(x)  Serial.println (x)
 #define PRINTDECLN(m,x) Serial.print(m); Serial.print(x,DEC); Serial.println("")
#else
 #define PRINT(x)
 #define PRINTDEC(x)
 #define PRINTLN(x)
 #define PRINTDECLN(m,x)
#endif
