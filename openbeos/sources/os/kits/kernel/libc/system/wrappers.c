#include <ktypes.h>
#include <sysctl.h>
#include <socket.h>
#include <OS.h>
#include <sem.h>
#include <syscalls.h>

int sysctl(int *name, uint namelen, void *oldp, size_t *oldlenp,
           void *newp, size_t newlen)
{
	/* we should handle CTL_USER here, but as we don't even define it yet :) */
	int err = sys_sysctl(name, namelen, oldp, oldlenp, newp, newlen);
	
	if (err < 0) {
		/* XXX - set errno */
	}
	
	return err;
}

/* This should maybe have it's own file but that seems a bit daft.
 * Hmm, maybe this file should just have a more generic name?
 */
int socket(int dom, int type, int prot)
{
	int err = sys_socket(dom, type, prot);
	
	if (err < 0) {
		/* XXX - set errno */
	}
	
	return err;
}

/* Thread calls */

thread_id spawn_thread(thread_func function, const char *name, 
                       int32 priority, void *data)
{
	thread_id new_thread = kern_spawn_thread(function, name, priority, data);
	
	if (new_thread < 0) {
		/* XXX - set errno */
	}
	
	return new_thread;
}

int kill_thread(thread_id thread)
{
	return kern_kill_thread(thread);
}

int resume_thread(thread_id thread)
{
	return kern_resume_thread(thread);
}

int suspend_thread(thread_id thread)
{
	return kern_suspend_thread(thread);
}

/* XXX - We should really find a better way of doing this... */
thread_id find_thread(const char *name)
{
	if (!name)
		return kern_get_current_thread_id();
	else
		printf("Not Implemented!!\n");
	return -1;
}

sem_id create_sem(int count, const char *name)
{
	return kern_create_sem(count, name);
}

int delete_sem(sem_id id)
{
	return kern_delete_sem(id);
}


int acquire_sem(sem_id id)
{
	return kern_acquire_sem(id);
}

int acquire_sem_etc(sem_id id, int count, int flags, bigtime_t timeout)
{
	return kern_acquire_sem_etc(id, count, flags, timeout);
}

int release_sem(sem_id id)
{
	return kern_release_sem(id);
}

int release_sem_etc(sem_id id, int count, int flags)
{
	return kern_release_sem_etc(id, count, flags);
}

/* XXX - add the others! */
