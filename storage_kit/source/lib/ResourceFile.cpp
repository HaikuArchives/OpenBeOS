//----------------------------------------------------------------------
//  This software is part of the OpenBeOS distribution and is covered 
//  by the OpenBeOS license.
//---------------------------------------------------------------------
/*!
	\file ResourceFile.cpp
	ResourceFile implementation.
*/

#include "ResourceFile.h"

#include <algobase.h>
#include <stdio.h>

#include "Elf.h"
#include "Exception.h"
#include "Pef.h"
#include "ResourceItem.h"
#include "ResourcesContainer.h"
#include "ResourcesDefs.h"
//#include "Warnings.h"

namespace StorageKit {

// ELF defs
static const uint32	kMaxELFHeaderSize			= sizeof(Elf32_Ehdr) + 32;
static const char	kELFFileMagic[4]			= { 0x7f, 'E', 'L', 'F' };

// sanity bounds
static const uint32	kMaxResourceCount			= 10000;
static const uint32	kELFMaxResourceAlignment	= 1024 * 1024 * 10;	// 10 MB

// recognized file types (indices into kFileTypeNames)
enum {
	FILE_TYPE_UNKNOWN		= 0,
	FILE_TYPE_X86_RESOURCE	= 1,
	FILE_TYPE_PPC_RESOURCE	= 2,
	FILE_TYPE_ELF			= 3,
	FILE_TYPE_PEF			= 4,
	FILE_TYPE_EMPTY			= 5,
};

const char *kFileTypeNames[] = {
	"unknown",
	"x86 resource file",
	"PPC resource file",
	"ELF object file",
	"PEF object file",
	"empty file",
};


// helper functions/classes

// read_exactly
static
void
read_exactly(BPositionIO &file, off_t position, void *buffer, size_t size,
			 const char *errorMessage = NULL)
{
	ssize_t read = file.ReadAt(position, buffer, size);
	if (read < 0)
		throw Exception(read, errorMessage);
	else if ((size_t)read != size) {
		if (errorMessage) {
			throw Exception("%s Read too few bytes (%ld/%lu).", errorMessage,
							read, size);
		} else
			throw Exception("Read too few bytes (%ld/%lu).", read, size);
	}
}

// write_exactly
static
void
write_exactly(BPositionIO &file, off_t position, const void *buffer,
			  size_t size, const char *errorMessage = NULL)
{
	ssize_t written = file.WriteAt(position, buffer, size);
	if (written < 0)
		throw Exception(written, errorMessage);
	else if ((size_t)written != size) {
		if (errorMessage) {
			throw Exception("%s Wrote too few bytes (%ld/%lu).", errorMessage,
							written, size);
		} else
			throw Exception("Wrote too few bytes (%ld/%lu).", written, size);
	}
}

// align_value
template<typename TV, typename TA>
static inline
TV
align_value(const TV &value, const TA &alignment)
{
	return ((value + alignment - 1) / alignment) * alignment;
}

// calculate_checksum
static
uint32
calculate_checksum(const void *data, uint32 size)
{
	uint32 checkSum = 0;
	const uint8 *csData = (const uint8*)data;
	const uint8 *dataEnd = csData + size;
	const uint8 *current = csData;
	for (; current < dataEnd; current += 4) {
		uint32 word = 0;
		int32 bytes = min(4L, dataEnd - current);
		for (int32 i = 0; i < bytes; i++)
			word = (word << 8) + current[i];
		checkSum += word;
	}
	return checkSum;
}

// skip_bytes
static inline
const void*
skip_bytes(const void *buffer, int32 offset)
{
	return (const char*)buffer + offset;
}

// skip_bytes
static inline
void*
skip_bytes(void *buffer, int32 offset)
{
	return (char*)buffer + offset;
}

// fill_pattern
static
void
fill_pattern(uint32 byteOffset, void *_buffer, uint32 count)
{
	uint32 *buffer = (uint32*)_buffer;
	for (uint32 i = 0; i < count; i++)
		buffer[i] = kUnusedResourceDataPattern[(byteOffset / 4 + i) % 3];
}

// fill_pattern
static
void
fill_pattern(const void *dataBegin, void *buffer, uint32 count)
{
	fill_pattern((char*)buffer - (const char*)dataBegin, buffer, count);
}

// fill_pattern
static
void
fill_pattern(const void *dataBegin, void *buffer, const void *bufferEnd)
{
	fill_pattern(dataBegin, buffer,
				 ((const char*)bufferEnd - (char*)buffer) / 4);
}

// check_pattern
static
bool
check_pattern(uint32 byteOffset, void *_buffer, uint32 count,
			  bool hostEndianess)
{
	bool result = true;
	uint32 *buffer = (uint32*)_buffer;
	for (uint32 i = 0; result && i < count; i++) {
		uint32 value = buffer[i];
		if (!hostEndianess)
			value = B_SWAP_INT32(value);
		result
			= (value == kUnusedResourceDataPattern[(byteOffset / 4 + i) % 3]);
	}
	return result;
}

// MemArea
struct MemArea {
	MemArea(const void *data, uint32 size) : data(data), size(size) {}

	inline bool check(const void *_current, uint32 skip = 0) const
	{
		const char *start = (const char*)data;
		const char *current = (const char*)_current;
		return (start <= current && start + size >= current + skip);
	}

	const void*	data;
	uint32		size;
};

// AutoDeleter
template<typename C>
struct AutoDeleter {
	AutoDeleter(C *object, bool array = false) : object(object), array(array)
	{
	}

	~AutoDeleter()
	{
		if (array)
			delete[] object;
		else
			delete object;
	}

	C*		object;
	bool	array;
};

// resource_parse_info
struct resource_parse_info {
	off_t				file_size;
	int32				resource_count;
	ResourcesContainer	*container;
	char				*info_table;
	uint32				info_table_offset;
	uint32				info_table_size;
};


// constructor
ResourceFile::ResourceFile()
			: fFile(),
			  fFileType(FILE_TYPE_UNKNOWN),
			  fHostEndianess(true),
			  fEmptyResources(true)
{
}

// destructor
ResourceFile::~ResourceFile()
{
	Unset();
}

// SetTo
status_t
ResourceFile::SetTo(BFile *file, bool clobber)
{
	status_t error = (file ? B_OK : B_BAD_VALUE);
	Unset();
	if (error == B_OK) {
		try {
			_InitFile(*file, clobber);
		} catch (Exception exception) {
			Unset();
			if (exception.Error() != B_OK)
				error = exception.Error();
			else
				error = B_ERROR;
		}
	}
	return error;
}

// Unset
void
ResourceFile::Unset()
{
	// file
	fFile.Unset();
	fFileType = FILE_TYPE_UNKNOWN;
	fHostEndianess = true;
	fEmptyResources = true;
}

// InitCheck
status_t
ResourceFile::InitCheck() const
{
	return fFile.InitCheck();
}

// InitContainer
status_t
ResourceFile::InitContainer(ResourcesContainer &container)
{
	container.MakeEmpty();
	status_t error = InitCheck();
	if (error == B_OK && !fEmptyResources) {
		resource_parse_info parseInfo;
		parseInfo.file_size = 0;
		parseInfo.resource_count = 0;
		parseInfo.container = &container;
		parseInfo.info_table = NULL;
		parseInfo.info_table_offset = 0;
		parseInfo.info_table_size = 0;
		try {
			// get the file size
			error = fFile.GetSize(&parseInfo.file_size);
			if (error != B_OK)
				throw Exception(error, "Failed to get the file size.");
			_ReadHeader(parseInfo);
			_ReadIndex(parseInfo);
			_ReadInfoTable(parseInfo);
		} catch (Exception exception) {
			if (exception.Error() != B_OK)
				error = exception.Error();
			else
				error = B_ERROR;
		}
		delete[] parseInfo.info_table;
	}
	return error;
}

// ReadResource
status_t
ResourceFile::ReadResource(ResourceItem &resource, bool force)
{
	status_t error = InitCheck();
	size_t size = resource.DataSize();
	if (error == B_OK && (force || !resource.IsLoaded())) {
		if (error == B_OK)
			error = resource.SetSize(size);
		void *data = NULL;
		if (error == B_OK) {
			data = resource.Data();
			ssize_t bytesRead = fFile.ReadAt(resource.Offset(), data, size);
			if (bytesRead < 0)
				error = bytesRead;
			else if ((size_t)bytesRead != size)
				error = B_IO_ERROR;
		}
		if (error == B_OK) {
			// convert the data, if necessary
			if (!fHostEndianess)
				swap_data(resource.Type(), data, size, B_SWAP_ALWAYS);
			resource.SetLoaded(true);
		}
	}
	return error;
}

// ReadResources
status_t
ResourceFile::ReadResources(ResourcesContainer &container, bool force)
{
	status_t error = InitCheck();
	int32 count = container.CountResources();
	for (int32 i = 0; error == B_OK && i < count; i++) {
		if (ResourceItem *resource = container.ResourceAt(i))
			error = ReadResource(*resource, force);
		else
			error = B_ERROR;
	}
	return error;
}

// WriteResources
status_t
ResourceFile::WriteResources(ResourcesContainer &container)
{
	status_t error = InitCheck();
	if (error == B_OK && !fFile.File()->IsWritable())
		error = B_NOT_ALLOWED;
	if (error == B_OK && fFileType == FILE_TYPE_EMPTY)
		error = _MakeEmptyResourceFile();
	if (error == B_OK)
		error = _WriteResources(container);
	if (error == B_OK)
		fEmptyResources = false;
	return error;
}


// _InitFile
void
ResourceFile::_InitFile(BFile &file, bool clobber)
{
	status_t error = B_OK;
	fFile.Unset();
	// get the file size first
	off_t fileSize = 0;
	error = file.GetSize(&fileSize);
	if (error != B_OK)
		throw Exception(error, "Failed to get the file size.");
	// read the first four bytes, and check, if they identify a resource file
	char magic[4];
	if (fileSize >= 4)
		read_exactly(file, 0, magic, 4, "Failed to read magic number.");
	else if (fileSize > 0 && !clobber)
		throw Exception(B_IO_ERROR, "File is not a resource file.");
	if (fileSize == 0) {
		// empty file
		fHostEndianess = true;
		fFileType = FILE_TYPE_EMPTY;
		fFile.SetTo(&file, 0);
		fEmptyResources = true;
	} else if (!memcmp(magic, kX86ResourceFileMagic, 4)) {
		// x86 resource file
		fHostEndianess = B_HOST_IS_LENDIAN;
		fFileType = FILE_TYPE_X86_RESOURCE;
		fFile.SetTo(&file, kX86ResourcesOffset);
		fEmptyResources = false;
	} else if (!memcmp(magic, kPEFFileMagic1, 4)) {
		PEFContainerHeader pefHeader;
		read_exactly(file, 0, &pefHeader, kPEFContainerHeaderSize,
					 "Failed to read PEF container header.");
		if (!memcmp(pefHeader.tag2, kPPCResourceFileMagic, 4)) {
			// PPC resource file
			fHostEndianess = B_HOST_IS_BENDIAN;
			fFileType = FILE_TYPE_PPC_RESOURCE;
			fFile.SetTo(&file, kPPCResourcesOffset);
			fEmptyResources = false;
		} else if (!memcmp(pefHeader.tag2, kPEFFileMagic2, 4)) {
			// PEF file
			fFileType = FILE_TYPE_PEF;
			_InitPEFFile(file, pefHeader);
		} else
			throw Exception(B_IO_ERROR, "File is not a resource file.");
	} else if (!memcmp(magic, kELFFileMagic, 4)) {
		// ELF file
		fFileType = FILE_TYPE_ELF;
		_InitELFFile(file);
	} else if (!memcmp(magic, kX86ResourceFileMagic, 2)) {
		// x86 resource file with screwed magic?
//		Warnings::AddCurrentWarning("File magic is 0x%08lx. Should be 0x%08lx "
//									"for x86 resource file. Try anyway.",
//									ntohl(*(uint32*)magic),
//									ntohl(*(uint32*)kX86ResourceFileMagic));
		fHostEndianess = B_HOST_IS_LENDIAN;
		fFileType = FILE_TYPE_X86_RESOURCE;
		fFile.SetTo(&file, kX86ResourcesOffset);
		fEmptyResources = true;
	} else {
		if (clobber) {
			// make it an x86 resource file
			fHostEndianess = true;
			fFileType = FILE_TYPE_EMPTY;
			fFile.SetTo(&file, 0);
		} else
			throw Exception(B_IO_ERROR, "File is not a resource file.");
	}
	error = fFile.InitCheck();
	if (error != B_OK)
		throw Exception(error, "Failed to initialize resource file.");
	// clobber, if desired
	if (clobber) {
		// just write an empty resources container
		ResourcesContainer container;
		WriteResources(container);
	}
}

// _InitELFFile
void
ResourceFile::_InitELFFile(BFile &file)
{
	status_t error = B_OK;
	// get the file size
	off_t fileSize = 0;
	error = file.GetSize(&fileSize);
	if (error != B_OK)
		throw Exception(error, "Failed to get the file size.");
	// read ELF header
	Elf32_Ehdr fileHeader;
	read_exactly(file, 0, &fileHeader, sizeof(Elf32_Ehdr),
				 "Failed to read ELF header.");
	// check data encoding (endianess)
	switch (fileHeader.e_ident[EI_DATA]) {
		case ELFDATA2LSB:
			fHostEndianess = B_HOST_IS_LENDIAN;
			break;
		case ELFDATA2MSB:
			fHostEndianess = B_HOST_IS_BENDIAN;
			break;
		default:
		case ELFDATANONE:
			throw Exception(B_IO_ERROR, "Unsupported ELF data encoding.");
			break;
	}
	// get the header values
	uint32 headerSize				= _GetUInt16(fileHeader.e_ehsize);
	uint32 programHeaderTableOffset	= _GetUInt32(fileHeader.e_phoff);
	uint32 programHeaderSize		= _GetUInt16(fileHeader.e_phentsize);
	uint32 programHeaderCount		= _GetUInt16(fileHeader.e_phnum);
	uint32 sectionHeaderTableOffset	= _GetUInt32(fileHeader.e_shoff);
	uint32 sectionHeaderSize		= _GetUInt16(fileHeader.e_shentsize);
	uint32 sectionHeaderCount		= _GetUInt16(fileHeader.e_shnum);
	bool hasProgramHeaderTable = (programHeaderTableOffset != 0);
	bool hasSectionHeaderTable = (sectionHeaderTableOffset != 0);
	// check the sanity of the header values
	// ELF header size
	if (headerSize < sizeof(Elf32_Ehdr) || headerSize > kMaxELFHeaderSize) {
		throw Exception(B_IO_ERROR,
						"Invalid ELF header: invalid ELF header size: %lu.",
						headerSize);
	}
	uint32 resourceOffset = headerSize;
	uint32 resourceAlignment = 0;
	// program header table offset and entry count/size
	uint32 programHeaderTableSize = 0;
	if (hasProgramHeaderTable) {
		if (programHeaderTableOffset < headerSize
			|| programHeaderTableOffset > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid ELF header: invalid program "
							"header table offset: %lu.",
							programHeaderTableOffset);
		}
		programHeaderTableSize = programHeaderSize * programHeaderCount;
		if (programHeaderSize < sizeof(Elf32_Phdr)
			|| programHeaderTableOffset + programHeaderTableSize > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid ELF header: program header "
							"table exceeds file: %lu.",
							programHeaderTableOffset + programHeaderTableSize);
		}
		resourceOffset = max(resourceOffset, programHeaderTableOffset
											 + programHeaderTableSize);
		// iterate through the program headers
		for (int32 i = 0; i < (int32)programHeaderCount; i++) {
			uint32 shOffset = programHeaderTableOffset + i * programHeaderSize;
			Elf32_Phdr programHeader;
			read_exactly(file, shOffset, &programHeader, sizeof(Elf32_Shdr),
						 "Failed to read ELF program header.");
			// get the header values
			uint32 type			= _GetUInt32(programHeader.p_type);
			uint32 offset		= _GetUInt32(programHeader.p_offset);
			uint32 size			= _GetUInt32(programHeader.p_filesz);
			uint32 alignment	= _GetUInt32(programHeader.p_align);
			// check the values
			// PT_NULL marks the header unused,
			if (type != PT_NULL) {
				if (/*offset < headerSize ||*/ offset > fileSize) {
					throw Exception(B_IO_ERROR, "Invalid ELF program header: "
									"invalid program offset: %lu.", offset);
				}
				uint32 segmentEnd = offset + size;
				if (segmentEnd > fileSize) {
					throw Exception(B_IO_ERROR, "Invalid ELF section header: "
									"segment exceeds file: %lu.", segmentEnd);
				}
				resourceOffset = max(resourceOffset, segmentEnd);
				resourceAlignment = max(resourceAlignment, alignment);
			}
		}
	}
	// section header table offset and entry count/size
	uint32 sectionHeaderTableSize = 0;
	if (hasSectionHeaderTable) {
		if (sectionHeaderTableOffset < headerSize
			|| sectionHeaderTableOffset > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid ELF header: invalid section "
							"header table offset: %lu.",
							sectionHeaderTableOffset);
		}
		sectionHeaderTableSize = sectionHeaderSize * sectionHeaderCount;
		if (sectionHeaderSize < sizeof(Elf32_Shdr)
			|| sectionHeaderTableOffset + sectionHeaderTableSize > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid ELF header: section header "
							"table exceeds file: %lu.",
							sectionHeaderTableOffset + sectionHeaderTableSize);
		}
		resourceOffset = max(resourceOffset, sectionHeaderTableOffset
											 + sectionHeaderTableSize);
		// iterate through the section headers
		for (int32 i = 0; i < (int32)sectionHeaderCount; i++) {
			uint32 shOffset = sectionHeaderTableOffset + i * sectionHeaderSize;
			Elf32_Shdr sectionHeader;
			read_exactly(file, shOffset, &sectionHeader, sizeof(Elf32_Shdr),
						 "Failed to read ELF section header.");
			// get the header values
			uint32 type		= _GetUInt32(sectionHeader.sh_type);
			uint32 offset	= _GetUInt32(sectionHeader.sh_offset);
			uint32 size		= _GetUInt32(sectionHeader.sh_size);
			// check the values
			// SHT_NULL marks the header unused,
			// SHT_NOBITS sections take no space in the file
			if (type != SHT_NULL && type != SHT_NOBITS) {
				if (offset < headerSize || offset > fileSize) {
					throw Exception(B_IO_ERROR, "Invalid ELF section header: "
									"invalid section offset: %lu.", offset);
				}
				uint32 sectionEnd = offset + size;
				if (sectionEnd > fileSize) {
					throw Exception(B_IO_ERROR, "Invalid ELF section header: "
									"section exceeds file: %lu.", sectionEnd);
				}
				resourceOffset = max(resourceOffset, sectionEnd);
			}
		}
	}
	// align the offset
	if (resourceAlignment < kELFMinResourceAlignment)
		resourceAlignment = kELFMinResourceAlignment;
	if (resourceAlignment > kELFMaxResourceAlignment) {
		throw Exception(B_IO_ERROR, "The ELF object file requires an invalid "
						"alignment: %lu.", resourceAlignment);
	}
	resourceOffset = align_value(resourceOffset, resourceAlignment);
	if (resourceOffset >= fileSize) {
//		throw Exception("The ELF object file does not contain resources.");
		fEmptyResources = true;
	} else
		fEmptyResources = false;
	// fine, init the offset file
	fFile.SetTo(&file, resourceOffset);
}

// _InitPEFFile
void
ResourceFile::_InitPEFFile(BFile &file, const PEFContainerHeader &pefHeader)
{
	status_t error = B_OK;
	// get the file size
	off_t fileSize = 0;
	error = file.GetSize(&fileSize);
	if (error != B_OK)
		throw Exception(error, "Failed to get the file size.");
	// check architecture -- we support PPC only
	if (memcmp(pefHeader.architecture, kPEFArchitecturePPC, 4))
		throw Exception(B_IO_ERROR, "PEF file architecture is not PPC.");
	fHostEndianess = B_HOST_IS_BENDIAN;
	// get the section count
	uint16 sectionCount = _GetUInt16(pefHeader.sectionCount);
	// iterate through the PEF sections headers
	uint32 sectionHeaderTableOffset = kPEFContainerHeaderSize;
	uint32 sectionHeaderTableEnd
		= sectionHeaderTableOffset + sectionCount * kPEFSectionHeaderSize;
	uint32 resourceOffset = sectionHeaderTableEnd;
	for (int32 i = 0; i < (int32)sectionCount; i++) {
		uint32 shOffset = sectionHeaderTableOffset + i * kPEFSectionHeaderSize;
		PEFSectionHeader sectionHeader;
		read_exactly(file, shOffset, &sectionHeader, kPEFSectionHeaderSize,
					 "Failed to read PEF section header.");
		// get the header values
		uint32 offset	= _GetUInt32(sectionHeader.containerOffset);
		uint32 size		= _GetUInt32(sectionHeader.packedSize);
		// check the values
		if (offset < sectionHeaderTableEnd || offset > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid PEF section header: invalid "
							"section offset: %lu.", offset);
		}
		uint32 sectionEnd = offset + size;
		if (sectionEnd > fileSize) {
			throw Exception(B_IO_ERROR, "Invalid PEF section header: section "
							"exceeds file: %lu.", sectionEnd);
		}
		resourceOffset = max(resourceOffset, sectionEnd);
	}
	if (resourceOffset >= fileSize) {
//		throw Exception("The PEF object file does not contain resources.");
		fEmptyResources = true;
	} else
		fEmptyResources = false;
	// init the offset file
	fFile.SetTo(&file, resourceOffset);
}

// _ReadHeader
void
ResourceFile::_ReadHeader(resource_parse_info &parseInfo)
{
	// read the header
	resources_header header;
	read_exactly(fFile, 0, &header, kResourcesHeaderSize,
				 "Failed to read the header.");
	// check the header
	// magic
	uint32 magic = _GetUInt32(header.rh_resources_magic);
	if (magic == kResourcesHeaderMagic) {
		// everything is fine
	} else if (B_SWAP_INT32(magic) == kResourcesHeaderMagic) {
		const char *endianessStr[2] = { "little", "big" };
		int32 endianess
			= (fHostEndianess == ((bool)B_HOST_IS_LENDIAN ? 0 : 1));
//		Warnings::AddCurrentWarning("Endianess seems to be %s, although %s "
//									"was expected.",
//									endianessStr[1 - endianess],
//									endianessStr[endianess]);
		fHostEndianess = !fHostEndianess;
	} else
		throw Exception(B_IO_ERROR, "Invalid resources header magic.");
	// resource count
	uint32 resourceCount = _GetUInt32(header.rh_resource_count);
	if (resourceCount > kMaxResourceCount)
		throw Exception(B_IO_ERROR, "Bad number of resources.");
	// index section offset
	uint32 indexSectionOffset = _GetUInt32(header.rh_index_section_offset);
	if (indexSectionOffset != kResourceIndexSectionOffset) {
		throw Exception(B_IO_ERROR, "Unexpected resource index section "
						"offset. Is: %lu, should be: %lu.", indexSectionOffset,
						kResourceIndexSectionOffset);
	}
	// admin section size
	uint32 indexSectionSize = kResourceIndexSectionHeaderSize
							  + kResourceIndexEntrySize * resourceCount;
	indexSectionSize = align_value(indexSectionSize,
								   kResourceIndexSectionAlignment);
	uint32 adminSectionSize = _GetUInt32(header.rh_admin_section_size);
	if (adminSectionSize != indexSectionOffset + indexSectionSize) {
		throw Exception(B_IO_ERROR, "Unexpected resource admin section size. "
						"Is: %lu, should be: %lu.", adminSectionSize,
						indexSectionOffset + indexSectionSize);
	}
	// set the resource count
	parseInfo.resource_count = resourceCount;
}

// _ReadIndex
void
ResourceFile::_ReadIndex(resource_parse_info &parseInfo)
{
	int32 &resourceCount = parseInfo.resource_count;
	off_t &fileSize = parseInfo.file_size;
	// read the header
	resource_index_section_header header;
	read_exactly(fFile, kResourceIndexSectionOffset, &header,
				 kResourceIndexSectionHeaderSize,
				 "Failed to read the resource index section header.");
	// check the header
	// index section offset
	uint32 indexSectionOffset = _GetUInt32(header.rish_index_section_offset);
	if (indexSectionOffset != kResourceIndexSectionOffset) {
		throw Exception(B_IO_ERROR, "Unexpected resource index section "
						"offset. Is: %lu, should be: %lu.", indexSectionOffset,
						kResourceIndexSectionOffset);
	}
	// index section size
	uint32 expectedIndexSectionSize = kResourceIndexSectionHeaderSize
		+ kResourceIndexEntrySize * resourceCount;
	expectedIndexSectionSize = align_value(expectedIndexSectionSize,
										   kResourceIndexSectionAlignment);
	uint32 indexSectionSize = _GetUInt32(header.rish_index_section_size);
	if (indexSectionSize != expectedIndexSectionSize) {
		throw Exception(B_IO_ERROR, "Unexpected resource index section size. "
						"Is: %lu, should be: %lu.", indexSectionSize,
						expectedIndexSectionSize);
	}
	// unknown section offset
	uint32 unknownSectionOffset
		= _GetUInt32(header.rish_unknown_section_offset);
	if (unknownSectionOffset != indexSectionOffset + indexSectionSize) {
		throw Exception(B_IO_ERROR, "Unexpected resource index section size. "
						"Is: %lu, should be: %lu.", unknownSectionOffset,
						indexSectionOffset + indexSectionSize);
	}
	// unknown section size
	uint32 unknownSectionSize = _GetUInt32(header.rish_unknown_section_size);
	if (unknownSectionSize != kUnknownResourceSectionSize) {
		throw Exception(B_IO_ERROR, "Unexpected resource index section "
						"offset. Is: %lu, should be: %lu.",
						unknownSectionOffset, kUnknownResourceSectionSize);
	}
	// info table offset and size
	uint32 infoTableOffset = _GetUInt32(header.rish_info_table_offset);
	uint32 infoTableSize = _GetUInt32(header.rish_info_table_size);
	if (infoTableOffset + infoTableSize > fileSize)
		throw Exception(B_IO_ERROR, "Invalid info table location.");
	parseInfo.info_table_offset = infoTableOffset;
	parseInfo.info_table_size = infoTableSize;
	// read the index entries
	uint32 indexTableOffset = indexSectionOffset
							  + kResourceIndexSectionHeaderSize;
	int32 maxResourceCount = (unknownSectionOffset - indexTableOffset)
							 / kResourceIndexEntrySize;
	int32 actualResourceCount = 0;
	bool tableEndReached = false;
	for (int32 i = 0; !tableEndReached && i < maxResourceCount; i++) {
		// read one entry
		tableEndReached = !_ReadIndexEntry(parseInfo, i, indexTableOffset,
										   (i >= resourceCount));
		if (!tableEndReached)
			actualResourceCount++;
	}
	// check resource count
	if (actualResourceCount != resourceCount) {
		if (actualResourceCount > resourceCount) {
//			Warnings::AddCurrentWarning("Resource index table contains "
//										"%ld entries, although it should be "
//										"%ld only.", actualResourceCount,
//										resourceCount);
		}
		resourceCount = actualResourceCount;
	}
}

// _ReadIndexEntry
bool
ResourceFile::_ReadIndexEntry(resource_parse_info &parseInfo, int32 index,
							  uint32 tableOffset, bool peekAhead)
{
	off_t &fileSize = parseInfo.file_size;
	//
	bool result = true;
	resource_index_entry entry;
	// read one entry
	off_t entryOffset = tableOffset + index * kResourceIndexEntrySize;
	read_exactly(fFile, entryOffset, &entry, kResourceIndexEntrySize,
				 "Failed to read a resource index entry.");
	// check, if the end is reached early
	if (result && check_pattern(entryOffset, &entry,
								kResourceIndexEntrySize / 4, fHostEndianess)) {
		if (!peekAhead) {
//			Warnings::AddCurrentWarning("Unexpected end of resource index "
//										"table at index: %ld (/%ld).",
//										index + 1, resourceCount);
		}
		result = false;
	}
	uint32 offset = _GetUInt32(entry.rie_offset);
	uint32 size = _GetUInt32(entry.rie_size);
	// check the location
	if (result && offset + size > fileSize) {
		if (peekAhead) {
//			Warnings::AddCurrentWarning("Invalid data after resource index "
//										"table.");
		} else {
			throw Exception(B_IO_ERROR, "Invalid resource index entry: index: "
							"%ld, offset: %lu (%lx), size: %lu (%lx).",
							index + 1, offset, offset, size, size);
		}
		result = false;
	}
	// add the entry
	if (result) {
		ResourceItem *item = new ResourceItem;
		item->SetLocation(offset, size);
		parseInfo.container->AddResource(item, index, false);
	}
	return result;
}

// _ReadInfoTable
void
ResourceFile::_ReadInfoTable(resource_parse_info &parseInfo)
{
	int32 &resourceCount = parseInfo.resource_count;
	// read the info table
	status_t error = B_OK;
	char *tableData = new char[parseInfo.info_table_size];
	int32 dataSize = parseInfo.info_table_size;
	parseInfo.info_table = tableData;
	read_exactly(fFile, parseInfo.info_table_offset, tableData, dataSize,
				 "Failed to read resource info table.");
	//
	bool *readIndices = new bool[resourceCount + 1];	// + 1 => always > 0
	for (int32 i = 0; i < resourceCount; i++)
		readIndices[i] = false;
	AutoDeleter<bool> deleter(readIndices, true);
	MemArea area(tableData, dataSize);
	const void *data = tableData;
	// check the table end/check sum
	if (_ReadInfoTableEnd(data, dataSize))
		dataSize -= kResourceInfoTableEndSize;
	// read the infos
	int32 resourceIndex = 1;
	uint32 minRemainderSize
		= kMinResourceInfoBlockSize + kResourceInfoSeparatorSize;
	while (area.check(data, minRemainderSize)) {
		// read a resource block
		if (!area.check(data, kMinResourceInfoBlockSize)) {
			throw Exception(B_IO_ERROR, "Unexpected end of resource info "
							"table at index %ld.", resourceIndex);
		}
		const resource_info_block *infoBlock
			= (const resource_info_block*)data;
		type_code type = _GetUInt32(infoBlock->rib_type);
		// read the infos of this block
		const resource_info *info = infoBlock->rib_info;
		while (info) {
			data = _ReadResourceInfo(parseInfo, area, info, type, readIndices);
			// prepare for next iteration, if there is another info
			if (!area.check(data, kResourceInfoSeparatorSize)) {
				throw Exception(B_IO_ERROR, "Unexpected end of resource info "
								"table after index %ld.", resourceIndex);
			}
			const resource_info_separator *separator
				= (const resource_info_separator*)data;
			if (_GetUInt32(separator->ris_value1) == 0xffffffff
				&& _GetUInt32(separator->ris_value2) == 0xffffffff) {
				// info block ends
				info = NULL;
				data = skip_bytes(data, kResourceInfoSeparatorSize);
			} else {
				// another info follows
				info = (const resource_info*)data;
			}
			resourceIndex++;
		}
		// end of the info block
	}
	// handle special case: empty resource info table
	if (resourceIndex == 1) {
		if (!area.check(data, kResourceInfoSeparatorSize)) {
			throw Exception(B_IO_ERROR, "Unexpected end of resource info "
							"table.");
		}
		const resource_info_separator *tableTerminator
			= (const resource_info_separator*)data;
		if (_GetUInt32(tableTerminator->ris_value1) != 0xffffffff
			|| _GetUInt32(tableTerminator->ris_value2) != 0xffffffff) {
			throw Exception(B_IO_ERROR, "The resource info table ought to be "
							"empty, but is not properly terminated.");
		}
		data = skip_bytes(data, kResourceInfoSeparatorSize);
	}
	// Check, if the correct number of bytes are remaining.
	uint32 bytesLeft = (const char*)tableData + dataSize - (const char*)data;
	if (bytesLeft != 0) {
		throw Exception(B_IO_ERROR, "Error at the end of the resource info "
						"table: %lu bytes are remaining.", bytesLeft);
	}
	// check, if all items have been initialized
	for (int32 i = resourceCount - 1; i >= 0; i--) {
		if (!readIndices[i]) {
//			Warnings::AddCurrentWarning("Resource item at index %ld "
//										"has no info. Item removed.", i + 1);
			if (ResourceItem *item = parseInfo.container->RemoveResource(i))
				delete item;
			resourceCount--;
		}
	}
}

// _ReadInfoTableEnd
bool
ResourceFile::_ReadInfoTableEnd(const void *data, int32 dataSize)
{
	bool hasTableEnd = true;
	if ((uint32)dataSize < kResourceInfoSeparatorSize)
		throw Exception(B_IO_ERROR, "Info table is too short.");
	if ((uint32)dataSize < kResourceInfoTableEndSize)
		hasTableEnd = false;
	if (hasTableEnd) {
		const resource_info_table_end *tableEnd
			= (const resource_info_table_end*)
			  skip_bytes(data, dataSize - kResourceInfoTableEndSize);
		if (_GetInt32(tableEnd->rite_terminator) != 0)
			hasTableEnd = false;
		if (hasTableEnd) {
			dataSize -= kResourceInfoTableEndSize;
			// checksum
			uint32 checkSum = calculate_checksum(data, dataSize);
			uint32 fileCheckSum = _GetUInt32(tableEnd->rite_check_sum);
			if (checkSum != fileCheckSum) {
				throw Exception(B_IO_ERROR, "Invalid resource info table check"
								" sum: In file: %lx, calculated: %lx.",
								fileCheckSum, checkSum);
			}
		}
	}
//	if (!hasTableEnd)
//		Warnings::AddCurrentWarning("resource info table has no check sum.");
	return hasTableEnd;
}

// _ReadResourceInfo
const void*
ResourceFile::_ReadResourceInfo(resource_parse_info &parseInfo, 
								const MemArea &area, const resource_info *info,
								type_code type, bool *readIndices)
{
	int32 &resourceCount = parseInfo.resource_count;
	//
	int32 id = _GetInt32(info->ri_id);
	int32 index = _GetInt32(info->ri_index);
	uint16 nameSize = _GetUInt16(info->ri_name_size);
	const char *name = info->ri_name;
	// check the values
	bool ignore = false;
	// index
	if (index < 1 || index > resourceCount) {
//		Warnings::AddCurrentWarning("Invalid index field in resource "
//									"info table: %lu.", index);
		ignore = true;
	}
	if (!ignore) {
		if (readIndices[index - 1]) {
			throw Exception(B_IO_ERROR, "Multiple resource infos with the "
							"same index field: %ld.", index);
		}
		readIndices[index - 1] = true;
	}
	// name size
	if (!area.check(name, nameSize)) {
		throw Exception(B_IO_ERROR, "Invalid name size (%d) for index %ld in "
						"resource info table.", (int)nameSize, index);
	}
	// check, if name is null terminated
	if (name[nameSize - 1] != 0) {
//		Warnings::AddCurrentWarning("Name for index %ld in "
//									"resource info table is not null "
//									"terminated.", index);
	}
	// set the values
	if (!ignore) {
		BString resourceName(name, nameSize);
		if (ResourceItem *item = parseInfo.container->ResourceAt(index - 1))
			item->SetIdentity(type, id, resourceName.String());
		else {
			throw Exception(B_IO_ERROR, "Unexpected error: No resource item "
							"at index %ld.", index);
		}
	}
	return skip_bytes(name, nameSize);
}

// _WriteResources
status_t
ResourceFile::_WriteResources(ResourcesContainer &container)
{
	status_t error = B_OK;
	int32 resourceCount = container.CountResources();
	char *buffer = NULL;
	try {
		// calculate sizes and offsets
		// header
		uint32 size = kResourcesHeaderSize;
		size_t bufferSize = size;
		// index section
		uint32 indexSectionOffset = size;
		uint32 indexSectionSize = kResourceIndexSectionHeaderSize
			+ resourceCount * kResourceIndexEntrySize;
		indexSectionSize = align_value(indexSectionSize,
									   kResourceIndexSectionAlignment);
		size += indexSectionSize;
		bufferSize = max(bufferSize, indexSectionSize);
		// unknown section
		uint32 unknownSectionOffset = size;
		uint32 unknownSectionSize = kUnknownResourceSectionSize;
		size += unknownSectionSize;
		bufferSize = max(bufferSize, unknownSectionSize);
		// data
		uint32 dataOffset = size;
		uint32 dataSize = 0;
		for (int32 i = 0; i < resourceCount; i++) {
			ResourceItem *item = container.ResourceAt(i);
			if (!item->IsLoaded())
				throw Exception(B_IO_ERROR, "Resource is not loaded.");
			dataSize += item->DataSize();
			bufferSize = max(bufferSize, item->DataSize());
		}
		size += dataSize;
		// info table
		uint32 infoTableOffset = size;
		uint32 infoTableSize = 0;
		type_code type = 0;
		for (int32 i = 0; i < resourceCount; i++) {
			ResourceItem *item = container.ResourceAt(i);
			if (i == 0 || type != item->Type()) {
				if (i != 0)
					infoTableSize += kResourceInfoSeparatorSize;
				type = item->Type();
				infoTableSize += kMinResourceInfoBlockSize;
			} else
				infoTableSize += kMinResourceInfoSize;
			if (const char *name = item->Name())
				infoTableSize += strlen(name) + 1;
		}
		infoTableSize += kResourceInfoSeparatorSize
						 + kResourceInfoTableEndSize;
		size += infoTableSize;
		bufferSize = max(bufferSize, infoTableSize);

		// write...
		// set the file size
		fFile.SetSize(size);
		buffer = new char[bufferSize];
		void *data = buffer;
		// header
		resources_header *resourcesHeader = (resources_header*)data;
		resourcesHeader->rh_resources_magic = kResourcesHeaderMagic;
		resourcesHeader->rh_resource_count = resourceCount;
		resourcesHeader->rh_index_section_offset = indexSectionOffset;
		resourcesHeader->rh_admin_section_size = indexSectionOffset
												 + indexSectionSize;
		for (int32 i = 0; i < 13; i++)
			resourcesHeader->rh_pad[i] = 0;
		write_exactly(fFile, 0, buffer, kResourcesHeaderSize,
					  "Failed to write resources header.");
		// index section
		data = buffer;
		// header
		resource_index_section_header *indexHeader
			= (resource_index_section_header*)data;
		indexHeader->rish_index_section_offset = indexSectionOffset;
		indexHeader->rish_index_section_size = indexSectionSize;
		indexHeader->rish_unknown_section_offset = unknownSectionOffset;
		indexHeader->rish_unknown_section_size = unknownSectionSize;
		indexHeader->rish_info_table_offset = infoTableOffset;
		indexHeader->rish_info_table_size = infoTableSize;
		fill_pattern(buffer - indexSectionOffset,
					 &indexHeader->rish_unused_data1, 1);
		fill_pattern(buffer - indexSectionOffset,
					 indexHeader->rish_unused_data2, 25);
		fill_pattern(buffer - indexSectionOffset,
					 &indexHeader->rish_unused_data3, 1);
		// index table
		data = skip_bytes(data, kResourceIndexSectionHeaderSize);
		resource_index_entry *entry = (resource_index_entry*)data;
		uint32 entryOffset = dataOffset;
		for (int32 i = 0; i < resourceCount; i++, entry++) {
			ResourceItem *item = container.ResourceAt(i);
			uint32 entrySize = item->DataSize();
			entry->rie_offset = entryOffset;
			entry->rie_size = entrySize;
			entry->rie_pad = 0;
			entryOffset += entrySize;
		}
		uint32 indexSectionPadding = (char*)entry - buffer;
		fill_pattern(buffer - indexSectionOffset, entry,
					 buffer + indexSectionSize);
		write_exactly(fFile, indexSectionOffset, buffer, indexSectionSize,
					  "Failed to write index section.");
		// unknown section
		fill_pattern(unknownSectionOffset, buffer, unknownSectionSize / 4);
		write_exactly(fFile, unknownSectionOffset, buffer, unknownSectionSize,
					  "Failed to write unknown section.");
		// data
		uint32 itemOffset = dataOffset;
		for (int32 i = 0; i < resourceCount; i++) {
			data = buffer;
			ResourceItem *item = container.ResourceAt(i);
			const void *itemData = item->Data();
			uint32 itemSize = item->DataSize();
			if (!itemData && itemSize > 0)
				throw Exception(error, "Invalid resource item data.");
			if (itemData) {
				// swap data, if necessary
				if (!fHostEndianess) {
					memcpy(data, itemData, itemSize);
					swap_data(item->Type(), data, itemSize, B_SWAP_ALWAYS);
					itemData = data;
				}
				write_exactly(fFile, itemOffset, itemData, itemSize,
							  "Failed to write resource item data.");
			}
			item->SetOffset(itemOffset);
			itemOffset += itemSize;
		}
		// info table
		data = buffer;
		type = 0;
		for (int32 i = 0; i < resourceCount; i++) {
			ResourceItem *item = container.ResourceAt(i);
			resource_info *info = NULL;
			if (i == 0 || type != item->Type()) {
				if (i != 0) {
					resource_info_separator *separator
						= (resource_info_separator*)data;
					separator->ris_value1 = 0xffffffff;
					separator->ris_value2 = 0xffffffff;
					data = skip_bytes(data, kResourceInfoSeparatorSize);
				}
				type = item->Type();
				resource_info_block *infoBlock = (resource_info_block*)data;
				infoBlock->rib_type = type;
				info = infoBlock->rib_info;
			} else
				info = (resource_info*)data;
			// info
			info->ri_id = item->ID();
			info->ri_index = i + 1;
			info->ri_name_size = 0;
			data = info->ri_name;
			if (const char *name = item->Name()) {
				uint32 nameLen = strlen(name);
				memcpy(info->ri_name, name, nameLen + 1);
				data = skip_bytes(data, nameLen + 1);
				info->ri_name_size = nameLen + 1;
			}
		}
		// separator
		resource_info_separator *separator = (resource_info_separator*)data;
		separator->ris_value1 = 0xffffffff;
		separator->ris_value2 = 0xffffffff;
		// table end
		data = skip_bytes(data, kResourceInfoSeparatorSize);
		resource_info_table_end *tableEnd = (resource_info_table_end*)data;
		tableEnd->rite_check_sum = calculate_checksum(buffer,
			infoTableSize - kResourceInfoTableEndSize);
		tableEnd->rite_terminator = 0;
		write_exactly(fFile, infoTableOffset, buffer, infoTableSize,
					  "Failed to write info table.");
	} catch (Exception exception) {
		if (exception.Error() != B_OK)
			error = exception.Error();
		else
			error = B_ERROR;
	}
	delete[] buffer;
	return error;
}

// _MakeEmptyResourceFile
status_t
ResourceFile::_MakeEmptyResourceFile()
{
	status_t error = fFile.InitCheck();
	if (error == B_OK && !fFile.File()->IsWritable())
		error = B_NOT_ALLOWED;
	if (error == B_OK) {
		try {
			BFile *file = fFile.File();
			// make it an x86 resource file
			error = file->SetSize(4);
			if (error != B_OK)
				throw Exception(error, "Failed to set file size.");
			write_exactly(*file, 0, kX86ResourceFileMagic, 4,
						  "Failed to write magic number.");
			fHostEndianess = B_HOST_IS_LENDIAN;
			fFileType = FILE_TYPE_X86_RESOURCE;
			fFile.SetTo(file, kX86ResourcesOffset);
			fEmptyResources = true;
		} catch (Exception exception) {
			if (exception.Error() != B_OK)
				error = exception.Error();
			else
				error = B_ERROR;
		}
	}
	return error;
}


};	// namespace StorageKit
