/*

DUN Header by Sikosis

(C) 2002

*/

#ifndef __DUN_H__
#define __DUN_H__

extern const char *APP_SIGNATURE;

class DUN : public BApplication {
public:
   DUN();
   virtual void MessageReceived(BMessage *message);
private:
};

#endif
