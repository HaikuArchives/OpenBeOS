/* Contains the ELF loader */

/*
** Copyright 2001, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <kernel.h>
#include <errors.h>
#include <elf.h>
#include <vfs.h>
#include <vm.h>
#include <thread.h>
#include <debug.h>
// #include <heap.h>
#include <arch/cpu.h>
#include <memheap.h>
/* ROUNDDOWN/UP macro's are defined in arch/x86/stage2_priv.h ??? */

#include <elf32.h>

#include <string.h>
#include <stdio.h>

struct elf_region {
	region_id id;
	addr start;
	addr size;
	long delta;
};

struct elf_image_info {
	struct elf_image_info *next;
	image_id id;
	int ref_count;
	void *vnode;
	struct elf_region regions[2]; // describes the text and data regions
	addr dynamic_ptr; // pointer to the dynamic section
	struct elf_linked_image *linked_images;

	struct Elf32_Ehdr *eheader;
	
	// pointer to symbol participation data structures
	char *needed;
	unsigned int *symhash;
	struct Elf32_Sym *syms;
	char *strtab;
	struct Elf32_Rel *rel;
	int rel_len;
	struct Elf32_Rela *rela;
	int rela_len;
	struct Elf32_Rel *pltrel;
	int pltrel_len;
};

// XXX TK this shall contain a list of linked images
//        (don't know enough about ELF how to get this list)
typedef struct elf_linked_image {
	struct elf_linked_image *next;
	struct elf_image_info *image;
} elf_linked_image;

static struct elf_image_info *kernel_images = NULL;
static struct elf_image_info *kernel_image = NULL;
static mutex image_lock;
static mutex image_load_lock;
static image_id next_image_id = 0;

#define STRING(image, offset) ((char *)(&(image)->strtab[(offset)]))
#define SYMNAME(image, sym) STRING(image, (sym)->st_name)
#define SYMBOL(image, num) ((struct Elf32_Sym *)&(image)->syms[num])
#define HASHTABSIZE(image) ((image)->symhash[0])
#define HASHBUCKETS(image) ((unsigned int *)&(image)->symhash[2])
#define HASHCHAINS(image) ((unsigned int *)&(image)->symhash[2+HASHTABSIZE(image)])

static void insert_image_in_list(struct elf_image_info *image)
{
	mutex_lock(&image_lock);

	image->next = kernel_images;
	kernel_images = image;

	mutex_unlock(&image_lock);
}

static void remove_image_from_list(struct elf_image_info *image)
{
	struct elf_image_info **ptr;

	mutex_lock(&image_lock);

	for(ptr = &kernel_images; *ptr; ptr = &(*ptr)->next) {
		if(*ptr == image) {
			*ptr = image->next;
			image->next = 0;
			break;
		}
	}

	mutex_unlock(&image_lock);
}

static struct elf_image_info *find_image(image_id id)
{
	struct elf_image_info *image;

	mutex_lock(&image_lock);

	for(image = kernel_images; image; image = image->next) {
		if(image->id == id)
			break;
	}
	mutex_unlock(&image_lock);

	return image;
}

static struct elf_image_info *find_image_by_vnode(void *vnode)
{
	struct elf_image_info *image;

	mutex_lock(&image_lock);

	for(image = kernel_images; image; image = image->next) {
		if(image->vnode == vnode)
			break;
	}
	mutex_unlock(&image_lock);

	return image;
}

static struct elf_image_info *create_image_struct()
{
	struct elf_image_info *image;

	image = (struct elf_image_info *)kmalloc(sizeof(struct elf_image_info));
	if(!image)
		return NULL;
	memset(image, 0, sizeof(struct elf_image_info));
	image->regions[0].id = -1;
	image->regions[1].id = -1;
	image->id = atomic_add(&next_image_id, 1);
	image->ref_count = 1;
	image->linked_images = NULL;
	return image;
}

static unsigned long elf_hash(const unsigned char *name)
{
	unsigned long hash = 0;
	unsigned long temp;

	while(*name) {
		hash = (hash << 4) + *name++;
		if((temp = hash & 0xf0000000))
			hash ^= temp >> 24;
		hash &= ~temp;
	}
	return hash;
}

static void dump_image_info(struct elf_image_info *image)
{
	int i;

	dprintf("elf_image_info at %p:\n", image);
	dprintf(" next %p\n", image->next);
	dprintf(" id 0x%x\n", image->id);
	for(i=0; i<2; i++) {
		dprintf(" regions[%d].id 0x%x\n", i, image->regions[i].id);
		dprintf(" regions[%d].start 0x%lx\n", i, image->regions[i].start);
		dprintf(" regions[%d].size 0x%lx\n", i, image->regions[i].size);
		dprintf(" regions[%d].delta %ld\n", i, image->regions[i].delta);
	}
	dprintf(" dynamic_ptr 0x%lx\n", image->dynamic_ptr);
	dprintf(" needed %p\n", image->needed);
	dprintf(" symhash %p\n", image->symhash);
	dprintf(" syms %p\n", image->syms);
	dprintf(" strtab %p\n", image->strtab);
	dprintf(" rel %p\n", image->rel);
	dprintf(" rel_len 0x%x\n", image->rel_len);
	dprintf(" rela %p\n", image->rela);
	dprintf(" rela_len 0x%x\n", image->rela_len);
}

/* XXX - Currently unused
static void dump_symbol(struct elf_image_info *image, struct Elf32_Sym *sym)
{

	dprintf("symbol at %p, in image %p\n", sym, image);

	dprintf(" name index %d, '%s'\n", sym->st_name, SYMNAME(image, sym));
	dprintf(" st_value 0x%x\n", sym->st_value);
	dprintf(" st_size %d\n", sym->st_size);
	dprintf(" st_info 0x%x\n", sym->st_info);
	dprintf(" st_other 0x%x\n", sym->st_other);
	dprintf(" st_shndx %d\n", sym->st_shndx);
}
*/

static struct Elf32_Sym *elf_find_symbol(struct elf_image_info *image, const char *name)
{
	unsigned int hash;
	unsigned int i;

        if(!image->dynamic_ptr)
                return NULL;

	hash = elf_hash(name) % HASHTABSIZE(image);
	for(i = HASHBUCKETS(image)[hash]; i != STN_UNDEF; i = HASHCHAINS(image)[i]) {
		if(!strcmp(SYMNAME(image, &image->syms[i]), name)) {
			return &image->syms[i];
		}
	}

	return NULL;
}

addr elf_lookup_symbol(image_id id, const char *symbol)
{
	struct elf_image_info *image;
	struct Elf32_Sym *sym;
	
	//dprintf( "elf_lookup_symbol: %s\n", symbol );

	image = find_image(id);
	if(!image)
		return 0;

	sym = elf_find_symbol(image, symbol);
	if(!sym)
		return 0;

	if(sym->st_shndx == SHN_UNDEF) {
		return 0;
	}
	
	/*dprintf( "found: %x (%x + %x)\n", sym->st_value + image->regions[0].delta, 
		sym->st_value, image->regions[0].delta );*/
	return sym->st_value + image->regions[0].delta;
}

static int elf_parse_dynamic_section(struct elf_image_info *image)
{
	struct Elf32_Dyn *d;
	int i;
	int needed_offset = -1;

//	dprintf("top of elf_parse_dynamic_section\n");

	image->symhash = 0;
	image->syms = 0;
	image->strtab = 0;

	d = (struct Elf32_Dyn *)image->dynamic_ptr;
	if(!d)
		return ERR_GENERAL;

	for(i=0; d[i].d_tag != DT_NULL; i++) {
		switch(d[i].d_tag) {
			case DT_NEEDED:
				needed_offset = d[i].d_un.d_ptr + image->regions[0].delta;
				break;
			case DT_HASH:
				image->symhash = (unsigned int *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_STRTAB:
				image->strtab = (char *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_SYMTAB:
				image->syms = (struct Elf32_Sym *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_REL:
				image->rel = (struct Elf32_Rel *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_RELSZ:
				image->rel_len = d[i].d_un.d_val;
				break;
			case DT_RELA:
				image->rela = (struct Elf32_Rela *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_RELASZ:
				image->rela_len = d[i].d_un.d_val;
				break;
			// TK: procedure linkage table
			case DT_JMPREL:
				image->pltrel = (struct Elf32_Rel *)(d[i].d_un.d_ptr + image->regions[0].delta);
				break;
			case DT_PLTRELSZ:
				image->pltrel_len = d[i].d_un.d_val;
				break;
			default:
				continue;
		}
	}

	// lets make sure we found all the required sections
	if(!image->symhash || !image->syms || !image->strtab)
		return ERR_GENERAL;

//	dprintf("needed_offset = %d\n", needed_offset);

	if(needed_offset >= 0)
		image->needed = STRING(image, needed_offset);

	return NO_ERROR;
}

// this function first tries to see if the first image and it's already resolved symbol is okay, otherwise
// it tries to link against the shared_image
// XXX gross hack and needs to be done better
static int  elf_resolve_symbol(struct elf_image_info *image, struct Elf32_Sym *sym, struct elf_image_info *shared_image, const char *sym_prepend,addr *sym_addr)
{
	struct Elf32_Sym *sym2;
	char new_symname[512];

	switch(sym->st_shndx) {
		case SHN_UNDEF:
			// patch the symbol name
			strlcpy(new_symname, sym_prepend, sizeof(new_symname));
			strlcat(new_symname, SYMNAME(image, sym), sizeof(new_symname));

			// it's undefined, must be outside this image, try the other image
			sym2 = elf_find_symbol(shared_image, new_symname);
			if(!sym2) {
				dprintf("elf_resolve_symbol: could not resolve symbol '%s'\n", new_symname);
				return ERR_ELF_RESOLVING_SYMBOL;
			}

			// make sure they're the same type
			if(ELF32_ST_TYPE(sym->st_info) != ELF32_ST_TYPE(sym2->st_info)) {
				dprintf("elf_resolve_symbol: found symbol '%s' in shared image but wrong type\n", new_symname);
				return ERR_ELF_RESOLVING_SYMBOL;
			}

			if(ELF32_ST_BIND(sym2->st_info) != STB_GLOBAL && ELF32_ST_BIND(sym2->st_info) != STB_WEAK) {
				dprintf("elf_resolve_symbol: found symbol '%s' but not exported\n", new_symname);
				return ERR_ELF_RESOLVING_SYMBOL;
			}

			*sym_addr = sym2->st_value + shared_image->regions[0].delta;
                        return NO_ERROR;
		case SHN_ABS:
			*sym_addr = sym->st_value;
                        return NO_ERROR;
		case SHN_COMMON:
			// XXX finish this
			dprintf("elf_resolve_symbol: COMMON symbol, finish me!\n");
			return ERR_NOT_IMPLEMENTED_YET;
		default:
			// standard symbol
			*sym_addr = sym->st_value + image->regions[0].delta;
                        return NO_ERROR;
	}
}

static int elf_relocate_rel(struct elf_image_info *image, const char *sym_prepend,
	struct Elf32_Rel *rel, int rel_len )
{
	int i;
	struct Elf32_Sym *sym;
	int vlErr;
	addr S;
	addr A;
	addr P;
	addr final_val;

	S = A = P = 0;
	
	for(i = 0; i * (int)sizeof(struct Elf32_Rel) < rel_len; i++) {
		//dprintf("looking at rel type %d, offset 0x%x\n", ELF32_R_TYPE(rel[i].r_info), rel[i].r_offset);

		// calc S
		switch(ELF32_R_TYPE(rel[i].r_info)) {
			case R_386_32:
			case R_386_PC32:
			case R_386_GLOB_DAT:
			case R_386_JMP_SLOT:
			case R_386_GOTOFF:
				sym = SYMBOL(image, ELF32_R_SYM(rel[i].r_info));

				vlErr = elf_resolve_symbol(image, sym, kernel_image, sym_prepend,&S);
                                if(vlErr<0) return vlErr;
				//dprintf("S 0x%x\n", S);
		}
		// calc A
		switch(ELF32_R_TYPE(rel[i].r_info)) {
			case R_386_32:
			case R_386_PC32:
			case R_386_GOT32:
			case R_386_PLT32:
			case R_386_RELATIVE:
			case R_386_GOTOFF:
			case R_386_GOTPC:
				A = *(addr *)(image->regions[0].delta + rel[i].r_offset);
//					dprintf("A 0x%x\n", A);
				break;
		}
		// calc P
		switch(ELF32_R_TYPE(rel[i].r_info)) {
			case R_386_PC32:
			case R_386_GOT32:
			case R_386_PLT32:
			case R_386_GOTPC:
				P = image->regions[0].delta + rel[i].r_offset;
//					dprintf("P 0x%x\n", P);
				break;
		}

		switch(ELF32_R_TYPE(rel[i].r_info)) {
			case R_386_NONE:
				continue;
			case R_386_32:
				final_val = S + A;
				break;
			case R_386_PC32:
				final_val = S + A - P;
				break;
			case R_386_RELATIVE:
				// B + A;
				final_val = image->regions[0].delta + A;
				break;
			case R_386_JMP_SLOT:
				final_val = S;
				dprintf( "final=%lx\n", final_val );
				break;
			default:
				dprintf("unhandled relocation type %d\n", ELF32_R_TYPE(rel[i].r_info));
				return ERR_NOT_ALLOWED;
		}
		*(addr *)(image->regions[0].delta + rel[i].r_offset) = final_val;
	}
	
	return NO_ERROR;
}

// XXX for now just link against the kernel
static int elf_relocate(struct elf_image_info *image, const char *sym_prepend)
{
	int res = NO_ERROR;
	int i;
//	dprintf("top of elf_relocate\n");

	// deal with the rels first
	if( image->rel ) {
		//dprintf( "total %i relocs\n", image->rel_len / (int)sizeof(struct Elf32_Rel) );
		res = elf_relocate_rel( image, sym_prepend, image->rel, image->rel_len );
		
		if( res )
			return res;
	}
	
	if( image->pltrel ) {
		//dprintf( "total %i plt-relocs\n", image->pltrel_len / (int)sizeof(struct Elf32_Rel) );
		res = elf_relocate_rel( image, sym_prepend, image->pltrel, image->pltrel_len );

		if( res )
			return res;
	}

	if(image->rela) {
		dprintf("RELA relocations not supported\n");
		return ERR_NOT_ALLOWED;
		for(i = 1; i * (int)sizeof(struct Elf32_Rela) < image->rela_len; i++) {
			dprintf("rela: type %d\n", ELF32_R_TYPE(image->rela[i].r_info));
		}
	}
	return res;
}

static int verify_eheader(struct Elf32_Ehdr *eheader)
{
	if(memcmp(eheader->e_ident, ELF_MAGIC, 4) != 0)
		return ERR_INVALID_BINARY;

	if(eheader->e_ident[4] != ELFCLASS32)
		return ERR_INVALID_BINARY;

	if(eheader->e_phoff == 0)
		return ERR_INVALID_BINARY;

	if(eheader->e_phentsize < sizeof(struct Elf32_Phdr))
		return ERR_INVALID_BINARY;

	return 0;
}

int elf_load_uspace(const char *path, struct proc *p, int flags, addr *entry)
{
	struct Elf32_Ehdr eheader;
	struct Elf32_Phdr *pheaders = NULL;
	int fd;
	int err;
	int i;
	ssize_t len;

	dprintf("elf_load: entry path '%s', proc %p\n", path, p);

	fd = sys_open(path, STREAM_TYPE_FILE, 0);
	if(fd < 0)
		return fd;
		
	len = sys_read(fd, &eheader, 0, sizeof(eheader));
	if(len < 0) {
		err = len;
		goto error;
	}
	if(len != sizeof(eheader)) {
		// short read
		err = ERR_INVALID_BINARY;
		goto error;
	}
	err = verify_eheader(&eheader);
	if(err < 0)
		goto error;

	pheaders = (struct Elf32_Phdr *)kmalloc(eheader.e_phnum * eheader.e_phentsize);
	if(pheaders == NULL) {
		dprintf("error allocating space for program headers\n");
		err = ERR_NO_MEMORY;
		goto error;
	}

	dprintf("reading in program headers at 0x%x, len 0x%x\n", eheader.e_phoff, eheader.e_phnum * eheader.e_phentsize);
	len = sys_read(fd, pheaders, eheader.e_phoff, eheader.e_phnum * eheader.e_phentsize);
	if(len < 0) {
		err = len;
		dprintf("error reading in program headers\n");
		goto error;
	}
	if(len != eheader.e_phnum * eheader.e_phentsize) {
		dprintf("short read while reading in program headers\n");
		err = -1;
		goto error;
	}

	for(i=0; i < eheader.e_phnum; i++) {
		char region_name[64];
		region_id id;
		char *region_addr;

		sprintf(region_name, "%s_seg%d", path, i);

		region_addr = (char *)ROUNDOWN(pheaders[i].p_vaddr, PAGE_SIZE);
		if(pheaders[i].p_flags & PF_W) {
			/*
			 * rw segment
			 */
			unsigned start_clearing;
			unsigned to_clear;
			unsigned A= pheaders[i].p_vaddr+pheaders[i].p_memsz;
			unsigned B= pheaders[i].p_vaddr+pheaders[i].p_filesz;

			A= ROUNDOWN(A, PAGE_SIZE);
			B= ROUNDOWN(B, PAGE_SIZE);

			id= vm_map_file(
				p->aspace_id,
				region_name,
				(void **)&region_addr,
				REGION_ADDR_EXACT_ADDRESS,
				ROUNDUP(pheaders[i].p_filesz+ (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE),
				LOCK_RW,
				REGION_PRIVATE_MAP,
				path,
				ROUNDOWN(pheaders[i].p_offset, PAGE_SIZE)
			);
			if(id < 0) {
				dprintf("error allocating region!\n");
				err = ERR_INVALID_BINARY;
				goto error;
			}


			/*
			 * clean garbage brought by mmap
			 */
			start_clearing=
				(unsigned)region_addr
				+ (pheaders[i].p_vaddr % PAGE_SIZE)
				+ pheaders[i].p_filesz;
			to_clear=
				ROUNDUP(pheaders[i].p_filesz+ (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE)
				- (pheaders[i].p_vaddr % PAGE_SIZE)
				- (pheaders[i].p_filesz);
			memset((void*)start_clearing, 0, to_clear);

			/*
			 * check if we need extra storage for the bss
			 */
			if(A != B) {
				/* XXX - this is broken! The final comma on the 1st line means we don't do the
				 *       subtraction? What's actually desired here?
				 */
				size_t bss_size=
					ROUNDUP(pheaders[i].p_memsz+ (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE)/*, */
					- ROUNDUP(pheaders[i].p_filesz+ (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE);

				sprintf(region_name, "%s_bss%d", path, 'X');

				region_addr+= ROUNDUP(pheaders[i].p_filesz+ (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE),
				id= vm_create_anonymous_region(
					p->aspace_id,
					region_name,
					(void **)&region_addr,
					REGION_ADDR_EXACT_ADDRESS,
					bss_size,
					REGION_WIRING_LAZY,
					LOCK_RW
				);
				if(id < 0) {
					dprintf("error allocating region!\n");
					err = ERR_INVALID_BINARY;
					goto error;
				}
			}
		} else {
			/*
			 * assume rx segment
			 */
			id= vm_map_file(
				p->aspace_id,
				region_name,
				(void **)&region_addr,
				REGION_ADDR_EXACT_ADDRESS,
				ROUNDUP(pheaders[i].p_memsz + (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE),
				LOCK_RO,
				REGION_PRIVATE_MAP,
				path,
				ROUNDOWN(pheaders[i].p_offset, PAGE_SIZE)
			);
			if(id < 0) {
				dprintf("error mapping text!\n");
				err = ERR_INVALID_BINARY;
				goto error;
			}
		}
	}

	dprintf("elf_load: done!\n");

	*entry = eheader.e_entry;

	err = 0;

error:
	if(pheaders)
		kfree(pheaders);
	sys_close(fd);

	return err;
}

image_id elf_load_kspace(const char *path, const char *sym_prepend)
{
	struct Elf32_Ehdr *eheader;
	struct Elf32_Phdr *pheaders;
	struct elf_image_info *image;
	void *vnode = NULL;
	int fd;
	int err;
	int i;
	ssize_t len;

	dprintf("elf_load_kspace: entry path '%s'\n", path);

	fd = sys_open(path, STREAM_TYPE_FILE, 0);
	if(fd < 0)
		return fd;

	err = vfs_get_vnode_from_fd(fd, true, &vnode);
	if(err < 0)
		goto error0;

	// XXX awful hack to keep someone else from trying to load this image
	// probably not a bad thing, shouldn't be too many races
	mutex_lock(&image_load_lock);

	// make sure it's not loaded already. Search by vnode
	image = find_image_by_vnode(vnode);
	if( image ) {
		atomic_add( &image->ref_count, 1 );
		//err = ERR_NOT_ALLOWED;
		goto done;
	}
	
	eheader = (struct Elf32_Ehdr *)kmalloc( sizeof( *eheader ));
	if( !eheader ) {
		err = ERR_NO_MEMORY;
		goto error;
	}

	len = sys_read(fd, eheader, 0, sizeof(*eheader));
	if(len < 0) {
		err = len;
		goto error1;
	}
	if(len != sizeof(*eheader)) {
		// short read
		err = ERR_INVALID_BINARY;
		goto error1;
	}
	err = verify_eheader(eheader);
	if(err < 0)
		goto error1;

	image = create_image_struct();
	if(!image) {
		err = ERR_NO_MEMORY;
		goto error1;
	}
	image->vnode = vnode;
	image->eheader = eheader;

	pheaders = (struct Elf32_Phdr *)kmalloc(eheader->e_phnum * eheader->e_phentsize);
	if(pheaders == NULL) {
		dprintf("error allocating space for program headers\n");
		err = ERR_NO_MEMORY;
		goto error2;
	}

//	dprintf("reading in program headers at 0x%x, len 0x%x\n", eheader.e_phoff, eheader.e_phnum * eheader.e_phentsize);
	len = sys_read(fd, pheaders, eheader->e_phoff, eheader->e_phnum * eheader->e_phentsize);
	if(len < 0) {
		err = len;
		dprintf("error reading in program headers\n");
		goto error3;
	}
	if(len != eheader->e_phnum * eheader->e_phentsize) {
		dprintf("short read while reading in program headers\n");
		err = -1;
		goto error3;
	}

	for(i=0; i < eheader->e_phnum; i++) {
		char region_name[64];
		bool ro_segment_handled = false;
		bool rw_segment_handled = false;
		int image_region;
		int lock;

//		dprintf("looking at program header %d\n", i);

		switch(pheaders[i].p_type) {
			case PT_LOAD:
				break;
			case PT_DYNAMIC:
				image->dynamic_ptr = pheaders[i].p_vaddr;
				continue;
			default:
				dprintf("unhandled pheader type 0x%x\n", pheaders[i].p_type);
				continue;
		}

		// we're here, so it must be a PT_LOAD segment
		if((pheaders[i].p_flags & (PF_R | PF_W | PF_X)) == (PF_R | PF_W)) {
			// this is the writable segment
			if(rw_segment_handled) {
				// we've already created this segment
				continue;
			}
			rw_segment_handled = true;
			image_region = 1;
			lock = LOCK_RW|LOCK_KERNEL;
			sprintf(region_name, "%s_rw", path);
		} else if((pheaders[i].p_flags & (PF_R | PF_W | PF_X)) == (PF_R | PF_X)) {
			// this is the non-writable segment
			if(ro_segment_handled) {
				// we've already created this segment
				continue;
			}
			ro_segment_handled = true;
			image_region = 0;
//			lock = LOCK_RO|LOCK_KERNEL;
			lock = LOCK_RW|LOCK_KERNEL;
			sprintf(region_name, "%s_ro", path);
		} else {
			dprintf("weird program header flags 0x%x\n", pheaders[i].p_flags);
			continue;
		}
		image->regions[image_region].size = ROUNDUP(pheaders[i].p_memsz + (pheaders[i].p_vaddr % PAGE_SIZE), PAGE_SIZE);
		image->regions[image_region].id = vm_create_anonymous_region(vm_get_kernel_aspace_id(), region_name,
			(void **)&image->regions[image_region].start, REGION_ADDR_ANY_ADDRESS,
			image->regions[image_region].size, REGION_WIRING_WIRED, lock);
		if(image->regions[image_region].id < 0) {
			dprintf("error allocating region!\n");
			err = ERR_INVALID_BINARY;
			goto error3;
		}
		image->regions[image_region].delta = image->regions[image_region].start - ROUNDOWN(pheaders[i].p_vaddr, PAGE_SIZE);

//		dprintf("elf_load_kspace: created a region at 0x%x\n", image->regions[image_region].start);

		len = sys_read(fd, (void *)(image->regions[image_region].start + (pheaders[i].p_vaddr % PAGE_SIZE)),
			pheaders[i].p_offset, pheaders[i].p_filesz);
		if(len < 0) {
			err = len;
			dprintf("error reading in seg %d\n", i);
			goto error4;
		}
	}

	if(image->regions[1].start != 0) {
		if(image->regions[0].delta != image->regions[1].delta) {
			dprintf("could not load binary, fix the region problem!\n");
			dump_image_info(image);
			err = ERR_NO_MEMORY;
			goto error4;
		}
	}

	// modify the dynamic ptr by the delta of the regions
	image->dynamic_ptr += image->regions[0].delta;

	err = elf_parse_dynamic_section(image);
	if(err < 0)
		goto error4;

	err = elf_relocate(image, sym_prepend);
	if(err < 0)
		goto error4;

	err = 0;

	kfree(pheaders);
	sys_close(fd);

	insert_image_in_list(image);

done:
	mutex_unlock(&image_load_lock);
	
	dprintf("elf_load_kspace: done!\n");

	return image->id;

error4:
	if(image->regions[1].id >= 0)
		vm_delete_region(vm_get_kernel_aspace_id(), image->regions[1].id);
	if(image->regions[0].id >= 0)
		vm_delete_region(vm_get_kernel_aspace_id(), image->regions[0].id);
error3:
	kfree(image);
error2:
	kfree(pheaders);
error1:
	kfree(eheader);
error:
	mutex_unlock(&image_load_lock);
error0:
	if(vnode)
		vfs_put_vnode_ptr(vnode);
	sys_close(fd);

	return err;
}

static int elf_unload_image( struct elf_image_info *image );

static int elf_unlink_relocs( struct elf_image_info *image )
{
	elf_linked_image *link, *next_link;
	
	for( link = image->linked_images; link; link = next_link ) {
		next_link = link->next;
		elf_unload_image( link->image );
		kfree( link );
	}
	
	return NO_ERROR;
}

static void elf_unload_image_final( struct elf_image_info *image )
{
	int i;
	
	for( i = 0; i < 2; ++i ) {
		vm_delete_region( vm_get_kernel_aspace_id(), image->regions[i].id );
	}

	if( image->vnode )
		vfs_put_vnode_ptr( image->vnode );

	remove_image_from_list(image);
	kfree( image->eheader );
	kfree( image );
}

static int elf_unload_image( struct elf_image_info *image )
{
	if( atomic_add( &image->ref_count, -1 ) > 0 )
		return NO_ERROR;

	elf_unlink_relocs( image );
	elf_unload_image_final( image );

	return NO_ERROR;
}

int elf_unload_kspace( const char *path )
{
	int fd;
	int err;
	void *vnode;
	struct elf_image_info *image;
	
	fd = sys_open(path, STREAM_TYPE_FILE, 0);
	if(fd < 0)
		return fd;

	err = vfs_get_vnode_from_fd(fd, true, &vnode);
	if(err < 0)
		goto error0;

	mutex_lock(&image_load_lock);

	image = find_image_by_vnode(vnode);
	if( !image ) {
		dprintf( "Tried to unload image that wasn't loaded (%s)\n", path );
		err = ERR_NOT_FOUND;
		goto error;
	}
	
	err = elf_unload_image( image );

error:
	mutex_unlock(&image_load_lock);
error0:
	if(vnode)
		vfs_put_vnode_ptr(vnode);
	sys_close(fd);

	return err;
}

int elf_init(kernel_args *ka)
{
	vm_region_info rinfo;

	// build a image structure for the kernel, which has already been loaded
	kernel_image = create_image_struct();

	// text segment
	kernel_image->regions[0].id = vm_find_region_by_name(vm_get_kernel_aspace_id(), "kernel_ro");
	if(kernel_image->regions[0].id < 0)
		panic("elf_init: could not look up kernel text segment region\n");
	vm_get_region_info(kernel_image->regions[0].id, &rinfo);
	kernel_image->regions[0].start = rinfo.base;
	kernel_image->regions[0].size = rinfo.size;

	// data segment
	kernel_image->regions[1].id = vm_find_region_by_name(vm_get_kernel_aspace_id(), "kernel_rw");
	if(kernel_image->regions[1].id < 0)
		panic("elf_init: could not look up kernel data segment region\n");
	vm_get_region_info(kernel_image->regions[1].id, &rinfo);
	kernel_image->regions[1].start = rinfo.base;
	kernel_image->regions[1].size = rinfo.size;

	// we know where the dynamic section is
	kernel_image->dynamic_ptr = (addr)ka->kernel_dynamic_section_addr.start;

	// parse the dynamic section
	if(elf_parse_dynamic_section(kernel_image) < 0)
		dprintf("elf_init: WARNING elf_parse_dynamic_section couldn't find dynamic section.\n");

	// insert it first in the list of kernel images loaded
	kernel_images = NULL;
	insert_image_in_list(kernel_image);

	mutex_init(&image_lock, "kimages_lock");
	mutex_init(&image_load_lock, "kimages_load_lock");

	return 0;
}

