#include <TimedEventQueue.h>
#include <stdio.h>

#define DEBUG 1
#include <Debug.h>

void InsertRemoveTest()
{
	BTimedEventQueue *q =new BTimedEventQueue;
	q->AddEvent(media_timed_event(0x1007,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1005,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x9999,BTimedEventQueue::B_STOP));//
	q->AddEvent(media_timed_event(0x1006,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1002,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1011,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1000,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x0777,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1001,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1000,BTimedEventQueue::B_STOP));//
	q->AddEvent(media_timed_event(0x1003,BTimedEventQueue::B_START));//
	q->AddEvent(media_timed_event(0x1000,BTimedEventQueue::B_SEEK));//
	ASSERT(q->EventCount() == 12);
	ASSERT(q->HasEvents() == true);
	
	q->RemoveEvent(&media_timed_event(0x1003,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 11);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1007,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 10);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1000,BTimedEventQueue::B_STOP));
	ASSERT(q->EventCount() == 9);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1000,BTimedEventQueue::B_SEEK));
	ASSERT(q->EventCount() == 8);
	ASSERT(q->HasEvents() == true);

	//remove non existing element (time)
	q->RemoveEvent(&media_timed_event(0x1111,BTimedEventQueue::B_STOP));
	ASSERT(q->EventCount() == 8);
	ASSERT(q->HasEvents() == true);

	//remove non existing element (type)
	q->RemoveEvent(&media_timed_event(0x1011,BTimedEventQueue::B_STOP));
	ASSERT(q->EventCount() == 8);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1000,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 7);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1011,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 6);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1002,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 5);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x0777,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 4);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x9999,BTimedEventQueue::B_STOP));
	ASSERT(q->EventCount() == 3);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1006,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 2);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1001,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 1);
	ASSERT(q->HasEvents() == true);

	q->RemoveEvent(&media_timed_event(0x1005,BTimedEventQueue::B_START));
	ASSERT(q->EventCount() == 0);
	ASSERT(q->HasEvents() == false);
	
	delete q;
}

media_timed_event DoForEachEvent;
int DoForEachCount;

BTimedEventQueue::queue_action 
DoForEachHook(media_timed_event *event, void *context)
{
	DoForEachEvent = *event;
	DoForEachCount++;
	printf("Callback, event_time = %x\n",int(event->event_time));
	return BTimedEventQueue::B_NO_ACTION;
}

void DoForEachTest()
{
	BTimedEventQueue *q =new BTimedEventQueue;
	ASSERT(q->EventCount() == 0);
	ASSERT(q->HasEvents() == false);

	q->AddEvent(media_timed_event(0x1000,BTimedEventQueue::B_SEEK));
	q->AddEvent(media_timed_event(0x1001,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1002,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1003,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1010,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1011,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1012,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1004,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1005,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1006,BTimedEventQueue::B_STOP));
	q->AddEvent(media_timed_event(0x1007,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1008,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1009,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_SEEK));
	ASSERT(q->EventCount() == 16);
	ASSERT(q->HasEvents() == true);
	

	printf("\n expected: 0x1000\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1000,BTimedEventQueue::B_AT_TIME);
	ASSERT(DoForEachEvent == media_timed_event(0x1000,BTimedEventQueue::B_SEEK));
	ASSERT(DoForEachCount == 1);
	

	printf("\n expected: 0x1000\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1006,BTimedEventQueue::B_AT_TIME);
	ASSERT(DoForEachEvent == media_timed_event(0x1006,BTimedEventQueue::B_STOP));
	ASSERT(DoForEachCount == 1);

	printf("\n expected: 0x1013, 0x1013, 0x1013\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1013,BTimedEventQueue::B_AT_TIME);
	ASSERT(DoForEachCount == 3);

	printf("\n expected: 0x1000, 0x1001, 0x1002\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1003,BTimedEventQueue::B_BEFORE_TIME,false);
	ASSERT(DoForEachCount == 3);

	printf("\n expected: 0x1000, 0x1001, 0x1002, 0x1003\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1003,BTimedEventQueue::B_BEFORE_TIME,true);
	ASSERT(DoForEachCount == 4);

	printf("\n expected: 0x1013, 0x1013, 0x1013\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1012,BTimedEventQueue::B_AFTER_TIME,false);
	ASSERT(DoForEachCount == 3);

	printf("\n expected: 0x1012, 0x1013, 0x1013, 0x1013\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1012,BTimedEventQueue::B_AFTER_TIME,true);
	ASSERT(DoForEachCount == 4);

	printf("\n expected: none\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1013,BTimedEventQueue::B_AFTER_TIME,false);
	ASSERT(DoForEachCount == 0);

	printf("\n expected: 0x1013, 0x1013, 0x1013\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x1013,BTimedEventQueue::B_AFTER_TIME,true);
	ASSERT(DoForEachCount == 3);

	printf("\n expected: all 16\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x0,BTimedEventQueue::B_ALWAYS);
	ASSERT(DoForEachCount == 16);

	printf("\n expected: none\n");
	DoForEachCount = 0;	
	q->DoForEach(DoForEachHook,(void*)1234,0x0,BTimedEventQueue::B_ALWAYS,false,BTimedEventQueue::B_WARP);
	ASSERT(DoForEachCount == 0);

	delete q;
}	

void FlushTest()
{
	BTimedEventQueue *q =new BTimedEventQueue;
	ASSERT(q->EventCount() == 0);
	ASSERT(q->HasEvents() == false);

	q->AddEvent(media_timed_event(0x1000,BTimedEventQueue::B_SEEK));
	q->AddEvent(media_timed_event(0x1001,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1002,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1003,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1010,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1011,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1012,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1004,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1005,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1006,BTimedEventQueue::B_STOP));
	q->AddEvent(media_timed_event(0x1007,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1008,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1009,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_START));
	q->AddEvent(media_timed_event(0x1013,BTimedEventQueue::B_SEEK));
	ASSERT(q->EventCount() == 16);
	ASSERT(q->HasEvents() == true);
	
	delete q;
}	


int main()
{
	InsertRemoveTest();
	DoForEachTest();
	FlushTest();
	return 0;
}
