#include <MidiStore.h>
#include <MidiText.h>
#include <Entry.h>
#include <iostream.h>

int main(int argc, char * argv[]) {
	if(argc < 2) {
		cerr << "Must supply a filename (*.mid)!" << endl;
		return 1;
	}
	BMidiText text;
	BMidiStore store;
	BEntry entry(argv[1],true);
	if(!entry.Exists()) {
		cerr << "File does not exist." << endl;
		return 2;
	}
	entry_ref e_ref;
	entry.GetRef(&e_ref);
	store.Import(&e_ref);
	text.Connect(&store);
	store.Start();
	snooze(10000000);
	store.Stop();
	
	return 0;
}