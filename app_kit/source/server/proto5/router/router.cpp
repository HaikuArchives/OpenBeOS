#include "router.h"
#include <InterfaceDefs.h>
#include <String.h>
#include <AppDefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <Message.h>
#include <Roster.h>

#include "PortLink.h"
#include "ServerProtocol.h"
//#define SERVER_PORT_NAME "OBappserver"
//#define SERVER_INPUT_PORT "OBinputport"


/*******************************************************
*   
*******************************************************/
BInputServerFilter* instantiate_input_filter(){
   return (new RouterInputFilter());
}

/*******************************************************
*   
*******************************************************/
RouterInputFilter::RouterInputFilter():BInputServerFilter(){
   port_id pid = find_port(SERVER_INPUT_PORT);
   serverlink = new PortLink(pid);
   
}

/*******************************************************
*   
*******************************************************/
RouterInputFilter::~RouterInputFilter(){
}

/*******************************************************
*   Everthing went just find .. right?
*******************************************************/
status_t RouterInputFilter::InitCheck(){
   return B_OK;
}



/*******************************************************
*   
*******************************************************/
filter_result RouterInputFilter::Filter(BMessage *message, BList *outList){
   if(serverlink == NULL){
      return B_DISPATCH_MESSAGE;
   }
   port_id pid = find_port(SERVER_INPUT_PORT);
   if(pid < 0){
      return B_DISPATCH_MESSAGE;
   }   
   serverlink->SetPort(pid);
   
   switch(message->what){
   case B_MOUSE_MOVED:{
      BPoint p;
      uint32 buttons = 0;
      // get piont and button from msg
      if(message->FindPoint("where",&p) != B_OK){
         
      }
      int64 time=(int64)real_time_clock();
      
      serverlink->SetOpCode(B_MOUSE_MOVED);
      serverlink->Attach(&time,sizeof(int64));
      serverlink->Attach(&p.x,sizeof(float));
      serverlink->Attach(&p.y,sizeof(float));
      serverlink->Attach(&buttons,sizeof(int32));
      serverlink->Flush();
      }break;
   // Should be some Mouse Down and Up code here ..
   // Along with some Key Down and up codes ..
   default:
      break;
   }
   
   //delete serverlink;
   
   // Let all msg flow normally to the 
   // Be app_server
   return B_DISPATCH_MESSAGE;
}
