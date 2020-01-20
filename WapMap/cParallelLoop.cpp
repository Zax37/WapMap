#include "cParallelLoop.h"
#include <hge.h>
#include <ctime>
#include <cstdio>8
extern HGE * hge;

//time_t prevtime = 0;
//int highest = 0;

cParallelLoop::cParallelLoop()
{
	fDeltaTreshold = 0.001f;
	hCallback = 0;
}

cParallelLoop::~cParallelLoop()
{

}

void cParallelLoop::SetFPS(int i)
{
	if (i <= 0)
		fDeltaTreshold = 0.001f;
	else
		fDeltaTreshold = (1.0f / float(i));
}

void cParallelLoop::Tick()
{
	/*if( prevtime == 0 ) prevtime = timeGetTime();
	int t = int(timeGetTime()-prevtime);
	prevtime = timeGetTime();
	if( int(t) > highest ) highest = int(t);
	if( t > 3 ){
	 printf("TICK @ %d", t);
	 printf(" [MAX %d]\n", highest);
	}*/
	if (hge->Timer_GetDelta() > fDeltaTreshold) { //TODO: Check!!!
	 //printf("   MAINLOOP\n");
	 //hge->System_DoManualMainLoop();
		if (hCallback != 0)
			hCallback->ParallelTrigger();
	}
}
