#include <Directory.h>

BDirectory::BDirectory() {
}

BDirectory::BDirectory(const entry_ref *ref) {
}


status_t
BDirectory::GetEntry(BEntry *entry) const {
	return B_ERROR;
}


status_t
BDirectory::GetNextEntry(BEntry *entry, bool traverse = false) {
	return B_ERROR;
}

status_t
BDirectory::GetNextRef(entry_ref *ref) {
	return B_ERROR;
}

int32
BDirectory::GetNextDirents(struct dirent *buf, size_t length,
							int32 count = INT_MAX) {
	return B_ERROR;
}

status_t
BDirectory::Rewind() {
	return B_ERROR;
}

int32
BDirectory::CountEntries() {
	return B_ERROR;
}


