/* Module manager. Uses hash.c */

/*
** Copyright 2001, Thomas Kurschel. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

// TODO:
// - "offsetof" macro is missing
//   -> move it to a public place

#include <module.h>
#include <lock.h>
#include <errors.h>
#include <arch/cpu.h>
#include <debug.h>
#include <khash.h>
#include <memheap.h>
#include <elf.h>
/* TODO: add the header once we have min() */
#include <stdio.h>
#include <string.h>

#if 1
#include <isa.h>
#endif


#ifndef offsetof
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
#endif

bool modules_disable_user_addons = false;

#define debug_level_flow  0
#define debug_level_error 1
#define debug_level_info  1

#define WAIT 
#define WAIT_ERROR
#define MSG_PREFIX "MODULE -- "

#define FUNC_NAME MSG_PREFIX __FUNCTION__ ": "

#define SHOW_FLOW(seriousness, format, param...) \
	do { if (debug_level_flow > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME, param ); WAIT \
	}} while( 0 )

#define SHOW_FLOW0(seriousness, format) \
	do { if (debug_level_flow > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME); WAIT \
	}} while( 0 )

#define SHOW_ERROR(seriousness, format, param...) \
	do { if (debug_level_error > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME, param ); WAIT_ERROR \
	}} while( 0 )

#define SHOW_ERROR0(seriousness, format) \
	do { if (debug_level_error > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME); WAIT_ERROR \
	}} while( 0 )

#define SHOW_INFO(seriousness, format, param...) \
	do { if (debug_level_info > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME, param ); WAIT \
	}} while( 0 )

#define SHOW_INFO0(seriousness, format) \
	do { if (debug_level_info > seriousness ) { \
		dprintf( "%s"##format, FUNC_NAME); WAIT \
	}} while( 0 )


typedef enum {
	MOD_QUERIED = 0,
	MOD_LOADED,
	MOD_INIT,
	MOD_RDY,
	MOD_UNINIT,
	MOD_ERROR
} module_state;

/* This represents the actual loaded module. The module is loaded and
 * may have more than one exported images, 
 * i.e. the module foo may actually have module_info structures for foo and bar
 * To allow for this each module_info structure within the module loaded is represented
 * by a loaded_module_info structure.
 */
typedef struct loaded_module {
	struct loaded_module      *next;
	struct loaded_module      *prev;
	module_info              **info;          /* the module_info we use */
	char                      *path;          /* the full path for the module */
	int                        ref_cnt;       /* how many ref's to this file */
} loaded_module;
struct loaded_module loaded_modules;

/* This is used to keep a list of module and the file it's found
 * in. It's used when we do searches to record that a module_info for 
 * a particular module is found in a particular file which covers us for
 * the case where we have a single file exporting a number of modules.
 */
typedef struct module {
	struct module         *next;
	struct module         *prev;
	struct loaded_module  *module;
	char                  *name;
	char                  *file;
	int                    ref_cnt;
	module_info           *ptr;    /* will only be valid if loaded == 1 */
	int                    offset; /* this is the offset in the headers */
	int                    state;  /* state of module */
	bool                   keep_loaded;
} module;

/* This is used to provide a list of modules we know about */
static struct module known_modules; 

#define INC_MOD_REF_COUNT(x) \
	x->ref_cnt++; \
	x->module->ref_cnt++;

#define DEC_MOD_REF_COUNT(x) \
	x->ref_cnt--; \
	x->module->ref_cnt--;


typedef struct module_iterator {
	char                        *prefix;
	int                          base_path_id;
	struct module_dir_iterator  *base_dir;
	struct module_dir_iterator  *cur_dir;
	int                          err;
	int                          module_pos;           /* This is used to keep track of which module_info
	                                                    * within a module we're addressing. */
	module_info                **cur_header;
	char                        *cur_path;
} module_iterator;

typedef struct module_dir_iterator {
	struct module_dir_iterator  *parent_dir;
	struct module_dir_iterator  *sub_dir;
	char                        *name;
	int                          file;
	int                          hdr_prefix;
} module_dir_iterator;


/* XXX - These should really be in a header so they are system wide... */
/* These are GCC only, so we'll need PPC version eventually... */
struct quehead {
	struct quehead *qh_link;
	struct quehead *qh_rlink;
};

static __inline void
insque(void *a, void *b)
{
	struct quehead *element = (struct quehead *)a,
		 *head = (struct quehead *)b;

	element->qh_link = head->qh_link;
	element->qh_rlink = head;
	head->qh_link = element;
	element->qh_link->qh_rlink = element;
}

static __inline void
remque(void *a)
{
	struct quehead *element = (struct quehead *)a;

	element->qh_link->qh_rlink = element->qh_rlink;
	element->qh_rlink->qh_link = element->qh_link;
	element->qh_rlink = 0;
}
  
/* XXX locking scheme: there is a global lock only; having several locks
 * makes trouble if dependent modules get loaded concurrently ->
 * they have to wait for each other, i.e. we need one lock per module;
 * also we must detect circular references during init and not dead-lock
 */
static recursive_lock modules_lock;		

/* These are the standard paths that we look on for mdoules to load.
 * By default we only look on these plus the prefix, though we do search
 * below the prefix.
 * i.e. using media as the prefix will match
 *      /boot/user-addons/media
 *      /boot/addons/media
 *      /boot/addons/media/encoders
 * but will NOT match
 *      /boot/addons/kernel/media
 */
const char *const module_paths[] = {
	"/boot/user-addons", 
	"/boot/addons"
};

#define num_module_paths (sizeof( module_paths ) / sizeof( module_paths[0] ))

#define MODULES_HASH_SIZE 16

/* the hash tables we use */
void *module_files;
void *modules_list;

static int module_compare(void *a, const void *key)
{
	module *mod = a;
	
	return strcmp(mod->name, (const char*)key);
}

static unsigned int module_hash(void *a, const void *key, unsigned int range)
{
	module *module = a;

	if (module != NULL )
		return hash_hash_str(module->name) % range;
	else
		return hash_hash_str(key) % range;
}

static int mod_files_compare(void *a, const void *key )
{
	loaded_module *module = a;
	const char *name = key;
	
	return strcmp(module->path, name);
}

static unsigned int mod_files_hash(void *a, const void *key, unsigned int range)
{
	loaded_module *module = a;

	if (module != NULL )
		return hash_hash_str(module->path) % range;
	else
		return hash_hash_str(key) % range;
}

static module_info **load_module_file(char *path) 
{
	image_id file_image = elf_load_kspace( path, "" );
	loaded_module *lm;
		
	if (file_image < 0 ) {
		SHOW_FLOW( 3, "couldn't load image %s (%s)\n", path, strerror( file_image ));
		return NULL;
	}

	lm = (loaded_module*)kmalloc(sizeof(loaded_module));
	if (!lm)
		return NULL;
	
	lm->info = (module_info**) elf_lookup_symbol(file_image, "modules");
	if (!lm->info) {
		dprintf("Failed to load %s due to lack of 'modules' symbol\n", path);
		kfree(lm);
		return NULL;
	}
		
	lm->path = (char*)kstrdup(path);
	lm->ref_cnt = 0;

	hash_insert(module_files, lm);
	insque(lm, &loaded_modules);

	return lm->info;
}

static inline void unload_module_file(char *path)
{
	loaded_module *themod; 
	themod = (loaded_module*)hash_lookup(module_files, path);
	if (!themod) {
		dprintf("Failed to find module %s\n", path);
		return;
	}

	if (themod->ref_cnt != 0) {
		dprintf("Can't unload %s due to ref_cnt = %d\n", themod->path, themod->ref_cnt);
		return;
	}

	recursive_lock_lock(&modules_lock);
	remque(themod);
	hash_remove(module_files, themod);

	recursive_lock_unlock(&modules_lock);

	elf_unload_kspace(themod->path);
	kfree(themod);
}

static int simple_module_info(module_info *mod, char *file, int offset)
{
	module *m;
	
	m = (module*)hash_lookup(modules_list, mod->name);
	if (m) {
		dprintf("Duplicate module name detected...ignoring\n");
		return EALREADY;
	}
		
	if ((m = (module*)kmalloc(sizeof(module))) == NULL) {
		return ENOMEM;
	}
	
	SHOW_FLOW(3, "simple_module_info(%s, %s)\n", mod->name, file);
	
	m->module = NULL; /* back pointer */
	m->name = (char*)kstrdup(mod->name);	
	m->state = MOD_QUERIED;
	/* Record where the module_info can be found */
	m->offset = offset;
	m->file = (char*)kstrdup(file);
	/* set the keep_loaded flag */
	if (mod->flags & B_KEEP_LOADED) {
		dprintf("module %s wants to be kept loaded\n", m->name);
		m->keep_loaded = true;
	}

	/* Record our existance and details... */
	recursive_lock_lock(&modules_lock);
	/* Insert into linked list */
	insque(m, &known_modules);
	hash_insert(modules_list, m);
	recursive_lock_unlock(&modules_lock);

	return 0;
}

static int recurse_directory(const char *path, const char *match)
{
	struct file_stat stat;
	int res = 0, file;
	char *name;
	
	if ((file = sys_open(path, STREAM_TYPE_DIR, 0)) < 0) {
		return -1;
	}
	
	name = (char*)kmalloc(SYS_MAX_NAME_LEN);
	while (res <= 0) {
		char *newpath;
		SHOW_FLOW(3, "scanning %s\n", path);

		if ((res = sys_read(file, name, 0, SYS_MAX_NAME_LEN)) <= 0) {
			sys_close(file);
			kfree(name);
			return -1;
		}
		newpath = (char*)kmalloc(strlen(path) + strlen(name) + 2);
		/* XXX - we may want to remove sprintf as it's a libc thing */
		sprintf(newpath, "%s/%s", path, name);
		if ((res = sys_rstat(newpath, &stat)) != NO_ERROR) {
			kfree(newpath);
			kfree(name);
			sys_close(file);
			return -1;
		}
		if (stat.type == STREAM_TYPE_FILE) {
			module_info **hdrs = load_module_file(newpath);
			int i = 0;
			if (hdrs) {
				module_info **chk;
				for (chk = hdrs; *chk; chk++) {
					simple_module_info(*chk, newpath, i++);
					if (strcmp((*chk)->name, match) == 0) {
						res = 1;
						break;
					}
				}
			}
			if (res != 1)
				unload_module_file(newpath);
		} else if (stat.type == STREAM_TYPE_DIR) {
			res = recurse_directory(newpath, match);
		}
		kfree(newpath);
	}
	kfree(name);
	sys_close(file);
	return res;
}

/* This is only called if we fail to find a module already in our cache...saves us
 * some extra checking here :)
 */
static module *search_module(const char *name)
{
	int i;
	SHOW_FLOW(3, "search_module(%s)\n", name);
	
	for (i = 0; i < (int)num_module_paths; ++i) {
		if (recurse_directory(module_paths[i], name) == 1)
			break;
	}
	return (module*)hash_lookup(modules_list, name);
}


static inline int init_module(module *module)
{
	int res = 0;
		
	switch(module->state) {
		case MOD_QUERIED:
		case MOD_LOADED:
			module->state = MOD_INIT;	
			SHOW_FLOW( 3, "initing module %s... \n", module->name );
			res = module->ptr->std_ops(B_MODULE_INIT);
			SHOW_FLOW(3, "...done (%s)\n", strerror(res));

			if (!res ) 
				module->state = MOD_RDY;
			else
				module->state = MOD_LOADED;
			break;

		case MOD_RDY:	
			res = NO_ERROR;
			break;
		
		case MOD_INIT:
			SHOW_ERROR( 0, "circular reference to %s\n", module->name );
			res = ERR_GENERAL;
			break;
		
		case MOD_UNINIT:
			SHOW_ERROR( 0, "tried to load module %s which is currently unloading\n", module->name );
			res = ERR_GENERAL;
			break;

		case MOD_ERROR:
			SHOW_INFO( 0, "cannot load module %s because its earlier unloading failed\n", module->name );
			res = ERR_GENERAL;
			break;
		
		default:
			res = ERR_GENERAL;
	}
	
	return res;
}

static inline int uninit_module(module *module)
{
	switch( module->state ) {
		case MOD_QUERIED:
		case MOD_LOADED:
			return NO_ERROR;

		case MOD_INIT:
			panic( "Trying to unload module %s which is initializing\n", 
				module->name );
			return ERR_GENERAL;

		case MOD_UNINIT:
			panic( "Trying to unload module %s which is un-initializing\n", module->name );
			return ERR_GENERAL;
		
		case MOD_RDY:
			{
				int res;
			
				module->state = MOD_UNINIT;

				SHOW_FLOW( 2, "uniniting module %s...\n", module->name );
				res = module->ptr->std_ops(B_MODULE_UNINIT);
				SHOW_FLOW( 2, "...done (%s)\n", strerror( res ));

				if (res == NO_ERROR ) {
					module->state = MOD_LOADED;
					return 0;
				}
			
				SHOW_ERROR( 0, "Error unloading module %s (%i)\n", module->name, res );
			}
		
			module->state = MOD_ERROR;
			module->keep_loaded = true;
			
		// fall through
		default:	
			return ERR_GENERAL;		
	}
}

static int process_module_info(module_iterator *iter, char *buf, size_t *bufsize)
{
	module *m;
	module_info **mod;
	int res = NO_ERROR;
		
	mod = iter->cur_header;
	if (!mod || !(*mod)) {
		res = EINVAL;
	} else {
		res = simple_module_info(*mod, iter->cur_path, iter->module_pos++);
		
		m = (module*)hash_lookup(modules_list, (*mod)->name);
		if (m) {
			strlcpy(buf, m->name, *bufsize);
			*bufsize = strlen(m->name);
		}
	}
	
	/* Deal with the header pointer!
	 * Basically if we have a valid pointer (mod) and the next (++mod) is NOT null,
	 * then we advance the cur_header pointer, otherwise we specify it as
	 * NULL to make sure we don't have trouble :)
	 */
	if (mod && *(++mod) != NULL)
		iter->cur_header++;
	else
		iter->cur_header = NULL;

	return res;
}	

static inline int module_create_dir_iterator( module_iterator *iter, int file, const char *name )
{
	module_dir_iterator *dir;
	
	/* if we're creating a dir_iterator, there is no way that the
	 * cur_header value can be valid, so make sure and reset it
	 * here.
	 */
	iter->cur_header = NULL;
	
	dir = (struct module_dir_iterator *)kmalloc( sizeof( *dir ));
	if (dir == NULL )
		return ERR_NO_MEMORY;
		
	dir->name = (char *)kstrdup( name );
	if (dir->name == NULL ) {
		kfree( dir );
		return ERR_NO_MEMORY;
	}

	dir->file = file;
	dir->sub_dir = NULL;		
	dir->parent_dir = iter->cur_dir;
	
	if (iter->cur_dir )
		iter->cur_dir->sub_dir = dir;
	else
		iter->base_dir = dir;
		
	iter->cur_dir = dir;

	SHOW_FLOW( 3, "created dir iterator for %s\n", name );		
	return NO_ERROR;
}

static inline int module_enter_dir(module_iterator *iter, const char *path)
{
	int file;
	int res;
	
	file = sys_open( path, STREAM_TYPE_DIR, 0 );
	if (file < 0 ) {
		SHOW_FLOW( 3, "couldn't open directory %s (%s)\n", path, strerror( file ));
		
		// there are so many errors for "not found" that we don't bother
		// and always assume that the directory suddenly disappeared
		return NO_ERROR;
	}
						
	res = module_create_dir_iterator(iter, file, path);
	if (res != NO_ERROR) {
		sys_close(file);
		return ENOMEM;
	}
	
	SHOW_FLOW( 3, "entered directory %s\n", path );				
	return NO_ERROR;
}


static inline void destroy_dir_iterator( module_iterator *iter )
{
	module_dir_iterator *dir;
	
	dir = iter->cur_dir;
	
	SHOW_FLOW( 3, "destroying directory iterator for sub-dir %s\n", dir->name );
	
	if (dir->parent_dir )
		dir->parent_dir->sub_dir = NULL;
		
	iter->cur_dir = dir->parent_dir;

	kfree(dir->name);
	kfree(dir);
}


static inline void module_leave_dir( module_iterator *iter )
{
	module_dir_iterator *parent_dir;
	
	SHOW_FLOW( 3, "leaving directory %s\n", iter->cur_dir->name );
	
	parent_dir = iter->cur_dir->parent_dir;
	iter->cur_header = NULL;	
	sys_close( iter->cur_dir->file );
	destroy_dir_iterator( iter );
	
	iter->cur_dir = parent_dir;
}

static void compose_path( char *path, module_iterator *iter, const char *name, bool full_path )
{
	module_dir_iterator *dir;
	
	if (full_path ) {
		strlcpy( path, iter->base_dir->name, SYS_MAX_PATH_LEN );
		strlcat( path, "/", SYS_MAX_PATH_LEN );
	} else {
		strlcpy( path, iter->prefix, SYS_MAX_PATH_LEN );
		if (*iter->prefix )
			strlcat( path, "/", SYS_MAX_PATH_LEN );
	}
	
	for( dir = iter->base_dir->sub_dir; dir; dir = dir->sub_dir ) {
		strlcat( path, dir->name, SYS_MAX_PATH_LEN );
		strlcat( path, "/", SYS_MAX_PATH_LEN );
	}
		
	strlcat( path, name, SYS_MAX_PATH_LEN );
	
	SHOW_FLOW( 3, "name: %s, %s -> %s\n", name, 
		full_path ? "full path" : "relative path", 
		path );
}

/* module_traverse_directory
 * Logic as follows...
 * If we have a headers pointer,
 * - check if the next structure is NULL, if not process that module_info structure
 * - if it's null, close the file, NULL the headers pointer and fall through
 *
 * This function tries to find the next module filename and then set the headers
 * pointer in the cur_dir structure.
 */ 
static inline int module_traverse_dir(module_iterator *iter)
{
	int res;
	struct file_stat stat;
	char name[SYS_MAX_NAME_LEN];
	char path[SYS_MAX_PATH_LEN];
	
	/* If (*iter->cur_header) != NULL we have another module within
	 * the existing file to return, so just return.
	 * Otherwise, actually find the next file to read.
	 */ 
	if (iter->cur_header) {
		if (*iter->cur_header == NULL)
			unload_module_file(iter->cur_path);
		else
			return NO_ERROR;
	}
	
	SHOW_FLOW( 3, "scanning %s\n", iter->cur_dir->name );
	if ((res = sys_read(iter->cur_dir->file, name, 0, sizeof(name))) <= 0) {
		SHOW_FLOW(3, "got error: %s\n", strerror(res));
		module_leave_dir(iter);
		return NO_ERROR;
	}

	SHOW_FLOW( 3, "got %s\n", name );

	if (strcmp( name, "." ) == 0 ||
		strcmp( name, ".." ) == 0 )
		return NO_ERROR;
	
	/* currently, sys_read returns an error if buffer is too small
	 * I don't know the official specification, so it's always safe
	 * to add a trailing end-of-string
	 */
	name[sizeof(name) - 1] = 0;
	compose_path(path, iter, name, true);

	/* As we're doing a new file, reset the pointers that might get
	 * screwed up...
	 */
	iter->cur_header = NULL;
	iter->module_pos = 0;
		
	if ((res = sys_rstat(path, &stat)) != NO_ERROR )
		return res;
		
	switch(stat.type) {
		case STREAM_TYPE_FILE:
			iter->cur_header = load_module_file(path);
			iter->cur_path = (char*)kstrdup(path);			
			if (!iter->cur_header)
				return EINVAL;
			return NO_ERROR;

		case STREAM_TYPE_DIR:
			return module_enter_dir(iter, path);

		default:
			SHOW_FLOW( 3, "entry %s not a file nor a directory - ignored\n", name );
			return NO_ERROR;
	}
}

/* module_enter_base_path
 * Basically try each of the directories we have listed as module paths,
 * trying each with the prefix we've been allocated.
 */
static inline int module_enter_base_path(module_iterator *iter)
{
	char path[SYS_MAX_PATH_LEN];
	
	++iter->base_path_id;

	if (iter->base_path_id >= (int)num_module_paths ) {
		SHOW_FLOW0( 3, "no locations left\n" );
		return ERR_NOT_FOUND;
	}

	SHOW_FLOW(3, "trying base path (%s)\n", module_paths[iter->base_path_id]);
	
	if (iter->base_path_id == 0 && modules_disable_user_addons) {
		SHOW_FLOW0( 3, "ignoring user add-ons (they are disabled)\n" );
		return NO_ERROR;
	}
		
	strcpy( path, module_paths[iter->base_path_id] );
	if (*iter->prefix) {
		strcat(path, "/");
		strlcat(path, iter->prefix, sizeof(path));
	}

	return module_enter_dir(iter, path);
}

/* open_module_list
 * This returns a pointer to a structure that can be used to
 * iterate through a list of all modules available under
 * a given prefix.
 * All paths will be searched and the returned list will
 * contain all modules available under the prefix.
 * The structure is then used by the read_next_module_name function
 * and MUST be freed or memory will be leaked.
 */
void *open_module_list(const char *prefix)
{
	module_iterator *iter;
	
	SHOW_FLOW( 3, "prefix: %s\n", prefix );
	
	iter = (module_iterator *)kmalloc(sizeof( module_iterator));
	if (!iter)
		return NULL;

	iter->prefix = (char *)kstrdup( prefix );
	if(iter->prefix == NULL) {
		kfree(iter);
		return NULL;
	}
	
	iter->base_path_id = -1;
	iter->base_dir = iter->cur_dir = NULL;
	iter->err = NO_ERROR;
	iter->module_pos = 0;
		
	return (void *)iter;
}

/* read_next_module_name
 * Return the next module name from the available list, using
 * a structure previously created by a call to open_module_list.
 * Returns 0 if a module was available.
 */
int read_next_module_name(void *cookie, char *buf, size_t *bufsize )
{
	module_iterator *iter = (module_iterator *)cookie;
	int res;

	*buf = '\0';
		
	if(!iter)
		return EINVAL;

	res = iter->err;

	SHOW_FLOW0(3, "looking for next module\n");
	while (res == NO_ERROR) {	
		SHOW_FLOW0(3, "searching for module\n");
		if (iter->cur_dir == NULL) {
			res = module_enter_base_path(iter);
		} else {
			if ((res = module_traverse_dir(iter)) == NO_ERROR) {
				/* By this point we should have a valid pointer to a module_info structure
				 * in iter->cur_header
				 */
				if (process_module_info(iter, buf, bufsize) == NO_ERROR)
					break;			
			}
		}
	}

	/* did we get something?? */
	if (*buf == '\0')
		res = ENOENT;
		
	iter->err = res;
	
	SHOW_FLOW(3, "finished with status %s\n", strerror(iter->err));
	return iter->err;
}


int close_module_list(void *cookie)
{
	module_iterator *iter = (module_iterator *)cookie;
	
	SHOW_FLOW0( 3, "\n" );
	
	if (!iter )
		return EINVAL;
		
	while(iter->cur_dir)
		module_leave_dir(iter);

	kfree(iter->prefix);
	kfree(iter);

	return 0;
}

/* module_init
 * setup module structures and data for use
 */
int module_init( kernel_args *ka, module_info **sys_module_headers )
{
	int res;
		
	SHOW_FLOW0( 0, "\n" );
	recursive_lock_create( &modules_lock );
	
	modules_list = hash_init(MODULES_HASH_SIZE, offsetof(module, next),
                             module_compare, module_hash);
		
	module_files = hash_init(MODULES_HASH_SIZE, offsetof(loaded_module, next),
	                         mod_files_compare, mod_files_hash);
	                         
	if (modules_list == NULL || module_files == NULL)
		return ENOMEM;

	loaded_modules.next = loaded_modules.prev = &loaded_modules;
	known_modules.next = known_modules.prev = &known_modules;

/*
	if (sys_module_headers) { 
		if (register_module_image("", "(built-in)", 0, sys_module_headers) == NULL)
			return ENOMEM;
	}
*/	
	#if 1
	{
		isa_bus_manager *isa_interface;
		int i;
		char module_name[SYS_MAX_PATH_LEN];
		size_t name_len;
		const char prefix[] = "bus_managers";
		void *modules_cookie;
		
		if ((res = get_module(ISA_MODULE_NAME, (module_info **)&isa_interface)) != 0) 
			dprintf( "Cannot load isa module (%s)\n", strerror( res ));
		else {

			dprintf("ISA: Test : ");
			for (i = 'A'; i <= 'Z'; ++i)
				isa_interface->write_io_8(0xe9, i);
			dprintf("\n");
				
			put_module(ISA_MODULE_NAME);
		}
		
		modules_cookie = open_module_list( prefix );

		name_len = sizeof(module_name);
		while(read_next_module_name( modules_cookie, module_name, &name_len ) == 0)
		{
			dprintf( "Found module %s\n", module_name );
			name_len = sizeof(module_name);
//			get_module(module_name, (module_info**)&isa_interface);
		}

		close_module_list( modules_cookie );		
		dprintf( "done\n" );
	}
	#endif
	
	return NO_ERROR;
}


/* BeOS Compatibility... */
int	get_module(const char *path, module_info **vec)
{
	module *m = (module *)hash_lookup(modules_list, path);
	loaded_module *lm;
	int res = NO_ERROR;
	*vec = NULL;

dprintf("*** get_module: %s\n", path);
	
	if (!m) {
		m = search_module(path);
		if (!m) {
			dprintf("Search failed.\n");
			return ENOENT;
		}
	}

	recursive_lock_lock(&modules_lock);

	lm = (loaded_module*)hash_lookup(module_files, m->file);
	if (!lm) {
		(void)load_module_file(m->file);
		lm = (loaded_module*)hash_lookup(module_files, m->file);
		if (!lm)
			return ENOENT;
	}

	/* We have the module file required in memory! */
	m->ptr = lm->info[m->offset];
	m->module = lm;
	INC_MOD_REF_COUNT(m);
	*vec = m->ptr;
	/* The state will be adjusted by the call to init_module */

	recursive_lock_unlock(&modules_lock);
	
	if (res != NO_ERROR) {
		vec = NULL;
		return res;
	}
	return init_module(m);
}

int put_module(const char *path)
{
	module *m = (module *)hash_lookup(modules_list, path);
	
	if (!m) {
		dprintf("We don't seem to have a reference to module %s\n", path);
		return EINVAL;
	}
	DEC_MOD_REF_COUNT(m);

	if (m->ref_cnt == 0 && m->keep_loaded == false) {
		uninit_module(m);
		unload_module_file(m->file);
	}
}
