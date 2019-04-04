#ifndef DEBUG_H
#define DEBUG_H

void debuginfo(char *buffer,unsigned short length);

#define DEBUG_INFO(BUFFER,LENGTH)		\
{		\
		debugcom.printf("%s %s %d ",__FILE__,__FUNCTION__,__LINE__);		\
		debuginfo(BUFFER,LENGTH);			\
}	

#endif

