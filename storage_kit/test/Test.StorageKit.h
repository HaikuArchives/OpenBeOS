#ifndef __sk_testing_is_so_much_fun_h__
#define __sk_testing_is_so_much_fun_h__

#include "CppUnitShell.h"

class StorageKitShell : public CppUnitShell {
public:
	StorageKitShell();	
	virtual void PrintDescription(int argc, char *argv[]);
};

extern StorageKitShell shell;


#endif // __sk_testing_is_so_much_fun_h__
