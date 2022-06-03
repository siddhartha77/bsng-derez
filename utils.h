unsigned char myValToBaseXChar(unsigned short v);
void myCopyPStr(const Str255 s,Str255 t);
void myAppendPStr(Str255 s,const Str255 suffixStr);
void myAppendCharToPStr(Str255 s,unsigned char c);
void myUNumToBaseBPStr(unsigned long n,Str255 s,unsigned short b,unsigned short minDigits);
unsigned short myUNumToBaseBDigits(unsigned long n,StringPtr s,unsigned short b,unsigned short minDigits);