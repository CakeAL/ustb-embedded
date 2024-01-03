/*******************************************************************************

Filename:     wait.c
Target:       cc2530
Author:       UNique
Revised:      14/7-2011
Revision:     1.1
Email:        cyliu@stu.xidian.edu.cn

******************************************************************************/
#include "hal.h"

//-----------------------------------------------------------------------------
// See hal.h for a description of this function.
//-----------------------------------------------------------------------------
void halWait(BYTE wait){
   UINT32 largeWait;

   if(wait == 0)
   {return;}
   largeWait = ((UINT16) (wait << 7));
   largeWait += 114*wait;

   largeWait = (largeWait >> CLKSPD);
   while(largeWait--);

   return;
}
