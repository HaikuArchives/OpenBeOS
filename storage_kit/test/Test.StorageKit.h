#ifndef __sk_testing_is_so_much_fun_h__
#define __sk_testing_is_so_much_fun_h__

#include <CppUnitShell.h>
#include <cppunit/TestCase.h>
#include <StorageDefs.h>

class BPath;

namespace StorageKit {

class TestShell : public CppUnitShell {
public:
	TestShell();
	~TestShell();
	int Run(int argc, char *argv[]);
	virtual void PrintDescription(int argc, char *argv[]);
	const char* TestDir() const;
private:
	BPath *fTestDir;
};

class TestCase : public CppUnit::TestCase {
public:
	TestCase();
	void SaveCWD();
	void RestoreCWD(const char *alternate = NULL);
protected:
	bool fValidCWD;
	char fCurrentWorkingDir[B_PATH_NAME_LENGTH+1];
};

};

extern StorageKit::TestShell shell;


#endif // __sk_testing_is_so_much_fun_h__
