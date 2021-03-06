#include "utils.h"

unsigned char myValToBaseXChar(unsigned short v) {
	if (v<10) return ('0'+v);		//00..09  -> '0'..'9'
	if (v<36) return ('A'-10+v);	//10..35  -> 'A'..'Z'
	return ('a'-36+v);				//36..61+ -> 'a'..'z'+
}

void myCopyPStr(const Str255 s,Str255 t) {
	BlockMove((Ptr) s,(Ptr) t,s[0]+1);
}

void myAppendPStr(Str255 s,const Str255 suffixStr) {
    register unsigned short	i=s[0];
    register unsigned short	j=suffixStr[0];
	
	if (j) {
		if ((i+j) <= 255) {
			BlockMove((Ptr) &suffixStr[1],(Ptr) &s[i+1],j);
			s[0]+=j;
		}
		else DebugStr("\pOverflow");
	}
}

void myAppendCharToPStr(Str255 s,unsigned char c) {
	if (s[0] < 255) s[++s[0]] = c;
	else DebugStr("\pOverflow");
}

void myUNumToBaseBPStr(unsigned long n,Str255 s,unsigned short b,unsigned short minDigits) { 
	s[0] = myUNumToBaseBDigits(n,&s[1],b,minDigits);
}

unsigned short myUNumToBaseBDigits(unsigned long n,StringPtr s,unsigned short b,unsigned short minDigits) {
register char	numStr[32];		//32 binary digits are possible from unsigned 32 bits
register short	start,i,j=0;

	if ((b<2) || (b>62)) return (0);	//error - illegal base was passed
	do {
		numStr[j++]=myValToBaseXChar(n%b);
		n/=b;
	} while (n>0);
	start=(j<minDigits) ? (minDigits-j) : 0;
	for (i=0;i<start;i++) s[i]='0';
	for (i=start;j>0;i++) s[i]=numStr[--j];
	return (i);
}