/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspthreadman.h - Library imports for the kernel threading library.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 * Copyright (c) 2005 Florin Sasu
 *
 * $Id$
 */
#ifndef __THREADMAN_H__
#define __THREADMAN_H__

#include <psptypes.h>
#include <pspkerneltypes.h>
/* Include for profile register definitions */
#include <pspdebug.h>

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

/** @defgroup ThreadMan Thread Manager Library
  * Library imports for the kernel threading library.
  */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ThreadMan */

/*@{*/

/** 64-bit system clock type. */
typedef struct SceKernelSysClock {
	SceUInt32   low;
	SceUInt32   hi;
} SceKernelSysClock;

/** Attribute for threads. */
enum PspThreadAttributes
{
	/** Enable VFPU access for the thread. */
	PSP_THREAD_ATTR_VFPU = 0x00004000,
	/** Start the thread in user mode (done automatically 
	  if the thread creating it is in user mode). */
	PSP_THREAD_ATTR_USER = 0x80000000,
	/** Thread is part of the USB/WLAN API. */
	PSP_THREAD_ATTR_USBWLAN = 0xa0000000,
	/** Thread is part of the VSH API. */
	PSP_THREAD_ATTR_VSH = 0xc0000000,
	/** Allow using scratchpad memory for a thread, NOT USABLE ON V1.0 */
	PSP_THREAD_ATTR_SCRATCH_SRAM = 0x00008000,
	/** Disables filling the stack with 0xFF on creation */
	PSP_THREAD_ATTR_NO_FILLSTACK = 0x00100000,
	/** Clear the stack when the thread is deleted */
	PSP_THREAD_ATTR_CLEAR_STACK = 0x00200000,
};

/* Maintained for compatibility with older versions of PSPSDK. */
#define THREAD_ATTR_VFPU PSP_THREAD_ATTR_VFPU
#define THREAD_ATTR_USER PSP_THREAD_ATTR_USER


/* Threads. */

typedef int (*SceKernelThreadEntry)(SceSize args, void *argp);

/** Additional options used when creating threads. */
typedef struct SceKernelThreadOptParam {
	/** Size of the ::SceKernelThreadOptParam structure. */
	SceSize 	size;
	/** UID of the memory block (?) allocated for the thread's stack. */
	SceUID 		stackMpid;
} SceKernelThreadOptParam;

/** Structure to hold the status information for a thread
  * @see sceKernelReferThreadStatus
  */
typedef struct SceKernelThreadInfo {
	/** Size of the structure */
	SceSize     size;
	/** Nul terminated name of the thread */
	char    	name[32];
	/** Thread attributes */
	SceUInt     attr;
	/** Thread status */
	int     	status;
	/** Thread entry point */
	SceKernelThreadEntry    entry;
	/** Thread stack pointer */
	void *  	stack;
	/** Thread stack size */
	int     	stackSize;
	/** Pointer to the gp */
	void *  	gpReg;
	/** Initial priority */
	int     	initPriority;
	/** Current priority */
	int     	currentPriority;
	/** Wait type */
	int     	waitType;
	/** Wait id */
	SceUID  	waitId;
	/** Wakeup count */
	int     	wakeupCount;
	/** Exit status of the thread */
	int     	exitStatus;
	/** Number of clock cycles run */
	SceKernelSysClock   runClocks;
	/** Interrupt preemption count */
	SceUInt     intrPreemptCount;
	/** Thread preemption count */
	SceUInt     threadPreemptCount;
	/** Release count */
	SceUInt     releaseCount;
} SceKernelThreadInfo;

/** Statistics about a running thread.
 * @see sceKernelReferThreadRunStatus.
 */
typedef struct SceKernelThreadRunStatus {
	SceSize 	size;
	int 		status;
	int 		currentPriority;
	int 		waitType;
	int 		waitId;
	int 		wakeupCount;
	SceKernelSysClock runClocks;
	SceUInt 	intrPreemptCount;
	SceUInt 	threadPreemptCount;
	SceUInt 	releaseCount;
} SceKernelThreadRunStatus;

/* Sure there must be more than this, but haven't seen them */
enum PspThreadStatus
{
	PSP_THREAD_RUNNING = 1,
	PSP_THREAD_READY   = 2,
	PSP_THREAD_WAITING = 4,
	PSP_THREAD_SUSPEND = 8,
	PSP_THREAD_STOPPED = 16,
	PSP_THREAD_KILLED  = 32, /* Thread manager has killed the thread (stack overflow) */
};

/**
 * Create a thread
 *
 * @par Example:
 * @code
 * SceUID thid;
 * thid = sceKernelCreateThread("my_thread", threadFunc, 0x18, 0x10000, 0, NULL);
 * @endcode
 *
 * @param name - An arbitrary thread name.
 * @param entry - The thread function to run when started.
 * @param initPriority - The initial priority of the thread. Less if higher priority.
 * @param stackSize - The size of the initial stack.
 * @param attr - The thread attributes, zero or more of ::PspThreadAttributes.
 * @param option - Additional options specified by ::SceKernelThreadOptParam.

 * @return UID of the created thread, or an error code.
 */
SceUID sceKernelCreateThread(const char *name, SceKernelThreadEntry entry, int initPriority,
                             int stackSize, SceUInt attr, SceKernelThreadOptParam *option);

/**
 * Delate a thread
 *
 * @param thid - UID of the thread to be deleted.
 *
 * @return < 0 on error.
 */
int sceKernelDeleteThread(SceUID thid);

/**
 * Start a created thread
 *
 * @param thid - Thread id from sceKernelCreateThread
 * @param arglen - Length of the data pointed to by argp, in bytes
 * @param argp - Pointer to the arguments.
 */
int sceKernelStartThread(SceUID thid, SceSize arglen, void *argp);

/**
 * Exit a thread
 *
 * @param status - Exit status.
 */
int sceKernelExitThread(int status);

/** 
  * Exit a thread and delete itself.
  *
  * @param status - Exit status
  */
int sceKernelExitDeleteThread(int status);

/**
 * Terminate a thread.
 *
 * @param thid - UID of the thread to terminate.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelTerminateThread(SceUID thid);

/**
 * Terminate and delete a thread.
 *
 * @param thid - UID of the thread to terminate and delete.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelTerminateDeleteThread(SceUID thid);

/**
 * Suspend the dispatch thread
 *
 * @return The current state of the dispatch thread, < 0 on error
 */
int sceKernelSuspendDispatchThread(void);

/**
 * Resume the dispatch thread
 *
 * @param state - The state of the dispatch thread 
 * (from ::sceKernelSuspendDispatchThread)
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelResumeDispatchThread(int state);

/**
 * Sleep thread
 *
 * @return < 0 on error.
 */
int sceKernelSleepThread(void);

/**
 * Sleep thread but service any callbacks as necessary
 *
 * @par Example:
 * @code
 * // Once all callbacks have been setup call this function
 * sceKernelSleepThreadCB();
 * @endcode
 */
void sceKernelSleepThreadCB(void);

/**
 * Wake a thread previously put into the sleep state.
 *
 * @param thid - UID of the thread to wake.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelWakeupThread(SceUID thid);

/**
 * Cancel a thread that was to be woken with ::sceKernelWakeupThread.
 *
 * @param thid - UID of the thread to cancel.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelCancelWakeupThread(SceUID thid);

/**
 * Suspend a thread.
 *
 * @param thid - UID of the thread to suspend.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelSuspendThread(SceUID thid);

/**
 * Resume a thread previously put into a suspended state with ::sceKernelSuspendThread.
 *
 * @param thid - UID of the thread to resume.
 *
 * @returns Success if >= 0, an error if < 0.
 */
int sceKernelResumeThread(SceUID thid);

/** 
  * Wait until a thread has ended.
  *
  * @param thid - Id of the thread to wait for.
  * @param timeout - Timeout in microseconds (assumed).
  *
  * @return < 0 on error.
  */
int sceKernelWaitThreadEnd(SceUID thid, SceUInt *timeout);

/** 
  * Wait until a thread has ended and handle callbacks if necessary.
  *
  * @param thid - Id of the thread to wait for.
  * @param timeout - Timeout in microseconds (assumed).
  *
  * @return < 0 on error.
  */
int sceKernelWaitThreadEndCB(SceUID thid, SceUInt *timeout);

/**
  * Delay the current thread by a specified number of microseconds
  *
  * @param delay - Delay in microseconds.
  *
  * @par Example:
  * @code
  * sceKernelDelayThread(1000000); // Delay for a second
  * @endcode
  */
void sceKernelDelayThread(SceUInt delay);

/**
  * Delay the current thread by a specified number of microseconds and handle any callbacks.
  *
  * @param delay - Delay in microseconds.
  *
  * @par Example:
  * @code
  * sceKernelDelayThread(1000000); // Delay for a second
  * @endcode
  */
void sceKernelDelayThreadCB(SceUInt delay);

/**
 * Modify the attributes of the current thread.
 *
 * @param unknown - Set to 0.
 * @param attr - The thread attributes to modify.  One of ::PspThreadAttributes.
 *
 * @returns < 0 on error.
 */
int sceKernelChangeCurrentThreadAttr(int unknown, SceUInt attr);

/**
  * Change the threads current priority.
  * 
  * @param thid - The ID of the thread (from sceKernelCreateThread or sceKernelGetThreadId)
  * @param priority - The new priority (the lower the number the higher the priority)
  *
  * @par Example:
  * @code
  * int thid = sceKernelGetThreadId();
  * // Change priority of current thread to 16
  * sceKernelChangeThreadPriority(thid, 16);
  * @endcode
  *
  * @return 0 if successful, otherwise the error code.
  */
int sceKernelChangeThreadPriority(SceUID thid, int priority);

/**
 * Rotate thread ready queue at a set priority
 *
 * @param priority - The priority of the queue
 * 
 * @return 0 on success, < 0 on error.
 */
int sceKernelRotateThreadReadyQueue(int priority);

/**
 * Release a thread in the wait state.
 *
 * @param thid - The UID of the thread.
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReleaseWaitThread(SceUID thid);

/** 
  * Get the current thread Id
  *
  * @return The thread id of the calling thread.
  */
int sceKernelGetThreadId(void);

/**
 * Get the current priority of the thread you are in.
 *
 * @return The current thread priority
 */
int sceKernelGetThreadCurrentPriority(void);

/**
 * Get the exit status of a thread.
 *
 * @param thid - The UID of the thread to check.
 *
 * @return The exit status
 */
int sceKernelGetThreadExitStatus(SceUID thid);

/**
 * Check the thread stack?
 *
 * @return Unknown.
 */
int sceKernelCheckThreadStack(void);

/**
 * Get the free stack size for a thread.
 *
 * @param thid - The thread ID. Seem to take current thread
 * if set to 0.
 *
 * @return The free size.
 */
int sceKernelGetThreadStackFreeSize(SceUID thid);

/** 
  * Get the status information for the specified thread.
  * 
  * @param thid - Id of the thread to get status
  * @param info - Pointer to the info structure to receive the data.
  * Note: The structures size field should be set to
  * sizeof(SceKernelThreadInfo) before calling this function.
  *
  * @par Example:
  * @code
  * SceKernelThreadInfo status;
  * status.size = sizeof(SceKernelThreadInfo);
  * if(sceKernelReferThreadStatus(thid, &status) == 0)
  * { Do something... }
  * @endcode 
  * @return 0 if successful, otherwise the error code.
  */
int sceKernelReferThreadStatus(SceUID thid, SceKernelThreadInfo *info);

/**
 * Retrive the runtime status of a thread.
 *
 * @param thid - UID of the thread to retrive status.
 * @param status - Pointer to a ::SceKernelThreadRunStatus struct to receive the runtime status.
 *
 * @returns 0 if successful, otherwise the error code.
 */
int sceKernelReferThreadRunStatus(SceUID thid, SceKernelThreadRunStatus *status);


/* Semaphores. */

/** Additional options used when creating semaphores. */
typedef struct SceKernelSemaOptParam {
	/** Size of the ::SceKernelSemaOptParam structure. */
	SceSize 	size;
} SceKernelSemaOptParam;

/** Current state of a semaphore.
 * @see sceKernelReferSemaStatus.
 */
typedef struct SceKernelSemaInfo {
	/** Size of the ::SceKernelSemaInfo structure. */
	SceSize 	size;
	/** NUL-terminated name of the semaphore. */
	char 		name[32];
	/** Attributes. */
	SceUInt 	attr;
	/** The initial count the semaphore was created with. */
	int 		initCount;
	/** The current count. */
	int 		currentCount;
	/** The maximum count. */
	int 		maxCount;
	/** The number of threads waiting on the semaphore. */
	int 		numWaitThreads;
} SceKernelSemaInfo;

/**
 * Creates a new semaphore
 *
 * @par Example:
 * @code
 * int semaid;
 * semaid = sceKernelCreateSema("MyMutex", 0, 1, 1, 0);
 * @endcode
 *
 * @param name - Specifies the name of the sema
 * @param attr - Sema attribute flags (normally set to 0)
 * @param initVal - Sema initial value 
 * @param maxVal - Sema maximum value
 * @param option - Sema options (normally set to 0)
 * @return A semaphore id
 */
SceUID sceKernelCreateSema(const char *name, SceUInt attr, int initVal, int maxVal, SceKernelSemaOptParam *option);

/**
 * Destroy a semaphore
 *
 * @param semaid - The semaid returned from a previous create call.
 * @return Returns the value 0 if its succesful otherwise -1
 */
int sceKernelDeleteSema(SceUID semaid);

/**
 * Send a signal to a semaphore
 *
 * @par Example:
 * @code
 * // Signal the sema
 * sceKernelSignalSema(semaid, 1);
 * @endcode
 *
 * @param semaid - The sema id returned from sceKernelCreateSema
 * @param signal - The amount to signal the sema (i.e. if 2 then increment the sema by 2)
 *
 * @return < 0 On error.
 */
int sceKernelSignalSema(SceUID semaid, int signal);

/**
 * Lock a semaphore
 *
 * @par Example:
 * @code
 * sceKernelWaitSema(semaid, 1, 0);
 * @endcode
 *
 * @param semaid - The sema id returned from sceKernelCreateSema
 * @param signal - The value to wait for (i.e. if 1 then wait till reaches a signal state of 1)
 * @param timeout - Timeout in microseconds (assumed).
 *
 * @return < 0 on error.
 */
int sceKernelWaitSema(SceUID semaid, int signal, SceUInt *timeout);

/**
 * Lock a semaphore a handle callbacks if necessary.
 *
 * @par Example:
 * @code
 * sceKernelWaitSemaCB(semaid, 1, 0);
 * @endcode
 *
 * @param semaid - The sema id returned from sceKernelCreateSema
 * @param signal - The value to wait for (i.e. if 1 then wait till reaches a signal state of 1)
 * @param timeout - Timeout in microseconds (assumed).
 *
 * @return < 0 on error.
 */
int sceKernelWaitSemaCB(SceUID semaid, int signal, SceUInt *timeout);

/**
 * Poll a sempahore.
 *
 * @param semaid - UID of the semaphore to poll.
 * @param signal - The value to test for.
 *
 * @return < 0 on error.
 */
int sceKernelPollSema(SceUID semaid, int signal);

/**
 * Retrieve information about a semaphore.
 *
 * @param semaid - UID of the semaphore to retrieve info for.
 * @param info - Pointer to a ::SceKernelSemaInfo struct to receive the info.
 *
 * @returns < 0 on error.
 */
int sceKernelReferSemaStatus(SceUID semaid, SceKernelSemaInfo *info);


/* Event flags. */

/** Structure to hold the event flag information */
typedef struct SceKernelEventFlagInfo {
	SceSize 	size;
	char 		name[32];
	SceUInt 	attr;
	SceUInt 	initPattern;
	SceUInt 	currentPattern;
	int 		numWaitThreads;
} SceKernelEventFlagInfo;

struct SceKernelEventFlagOptParam {
	SceSize 	size;
};

typedef struct SceKernelEventFlagOptParam SceKernelEventFlagOptParam;

/** 
  * Create an event flag.
  *
  * @param name - The name of the event flag.
  * @param attr - Set to 0
  * @param bits - Initial bit pattern.
  * @param opt  - Options, set to NULL
  * @return < 0 on error. >= 0 event flag id.
  *
  * @par Example:
  * @code
  * int evid;
  * evid = sceKernelCreateEventFlag("wait_event", 0, 0, 0);
  * @endcode
  */
int sceKernelCreateEventFlag(const char *name, int attr, int bits, SceKernelEventFlagOptParam *opt);

/** 
  * Set an event flag bit pattern.
  *
  * @param evid - The event id returned by sceKernelCreateEventFlag.
  * @param bits - The bit pattern to set.
  *
  * @return < 0 On error
  */
int sceKernelSetEventFlag(int evid, u32 bits);

/** 
  * Poll an event flag for a given bit pattern.
  *
  * @param evid - The event id returned by sceKernelCreateEventFlag.
  * @param bits - The bit pattern to poll for.
  * @param wait - 0 to return immediately, 1 to wait until the pattern is matched
  * @param outBits - The bit pattern that was matched.
  * @return < 0 On error
  */
int sceKernelPollEventFlag(int evid, u32 bits, u32 wait, u32 *outBits);

/** 
  * Wait for an event flag for a given bit pattern.
  *
  * @param evid - The event id returned by sceKernelCreateEventFlag.
  * @param bits - The bit pattern to poll for.
  * @param wait - 0 to return immediately, 1 to wait until the pattern is matched
  * @param outBits - The bit pattern that was matched.
  * @param arg     - Unknown (possibly timeout)
  * @return < 0 On error
  */
int sceKernelWaitEventFlag(int evid, u32 bits, u32 wait, u32 *outBits, void *arg);

/** 
  * Delete an event flag
  *
  * @param evid - The event id returned by sceKernelCreateEventFlag.
  *
  * @return < 0 On error
  */
int sceKernelDeleteEventFlag(int evid);

/** 
  * Get the status of an event flag.
  * 
  * @param event - The UID of the event.
  * @param status - A pointer to a ::SceKernelEventFlagInfo structure.
  *
  * @return < 0 on error.
  */
int sceKernelReferEventFlagStatus(SceUID event, SceKernelEventFlagInfo *status);


/* Message boxes. */

/** Additional options used when creating messageboxes. */
typedef struct SceKernelMbxOptParam {
	/** Size of the ::SceKernelMbxOptParam structure. */
	SceSize 	size;
} SceKernelMbxOptParam;

/** Current state of a messagebox.
 * @see sceKernelReferMbxStatus.
 */
typedef struct SceKernelMbxInfo {
	/** Size of the ::SceKernelMbxInfo structure. */
	SceSize 	size;
	/** NUL-terminated name of the messagebox. */
	char 		name[32];
	/** Attributes. */
	SceUInt 	attr;
	/** The number of threads waiting on the messagebox. */
	int 		numWaitThreads;
	/** Number of messages currently in the messagebox. */
	int 		numMessages;
	/** The message currently at the head of the queue. */
	void		*firstMessage;
} SceKernelMbxInfo;

/**
 * Creates a new messagebox
 *
 * @par Example:
 * @code
 * int mbxid;
 * mbxid = sceKernelCreateMbx("MyMessagebox", 0, NULL);
 * @endcode
 *
 * @param name - Specifies the name of the mbx
 * @param attr - Mbx attribute flags (normally set to 0)
 * @param option - Mbx options (normally set to NULL)
 * @return A messagebox id
 */
SceUID sceKernelCreateMbx(const char *name, SceUInt attr, SceKernelMbxOptParam *option);

/**
 * Destroy a messagebox
 *
 * @param mbxid - The mbxid returned from a previous create call.
 * @return Returns the value 0 if its succesful otherwise an error code
 */
int sceKernelDeleteMbx(SceUID mbxid);

/**
 * Send a message to a messagebox
 *
 * @par Example:
 * @code
 * // Send the message
 * sceKernelSendMbx(mbxid, "Hello");
 * @endcode
 *
 * @param mbxid - The mbx id returned from sceKernelCreateMbx
 * @param message - A message to be forwarded to the receiver.
 *                  The first word in the message will be used
 *                  by the kernel to link messages together,
 *                  but the rest is used defined.
 *
 * @return < 0 On error.
 */
int sceKernelSendMbx(SceUID mbxid, void *message);

/**
 * Wait for a message to arrive in a messagebox
 *
 * @par Example:
 * @code
 * void *msg;
 * sceKernelReceiveMbx(mbxid, &msg, NULL);
 * @endcode
 *
 * @param mbxid - The mbx id returned from sceKernelCreateMbx
 * @param pmessage - A pointer to where a pointer to the
 *                   received message should be stored
 * @param timeout - Timeout in microseconds
 *
 * @return < 0 on error.
 */
int sceKernelReceiveMbx(SceUID mbxid, void **pmessage, SceUInt *timeout);

/**
 * Wait for a message to arrive in a messagebox and handle callbacks if necessary.
 *
 * @par Example:
 * @code
 * void *msg;
 * sceKernelReceiveMbxCB(mbxid, &msg, NULL);
 * @endcode
 *
 * @param mbxid - The mbx id returned from sceKernelCreateMbx
 * @param pmessage - A pointer to where a pointer to the
 *                   received message should be stored
 * @param timeout - Timeout in microseconds
 *
 * @return < 0 on error.
 */
int sceKernelReceiveMbxCB(SceUID mbxid, void **pmessage, SceUInt *timeout);

/**
 * Check if a message has arrived in a messagebox
 *
 * @par Example:
 * @code
 * void *msg;
 * sceKernelPollMbx(mbxid, &msg);
 * @endcode
 *
 * @param mbxid - The mbx id returned from sceKernelCreateMbx
 * @param pmessage - A pointer to where a pointer to the
 *                   received message should be stored
 *
 * @return < 0 on error (SCE_KERNEL_ERROR_MBOX_NOMSG if the mbx is empty).
 */
int sceKernelPollMbx(SceUID mbxid, void **pmessage);

/**
 * Abort all wait operations on a messagebox
 *
 * @par Example:
 * @code
 * sceKernelCancelReceiveMbx(mbxid, NULL);
 * @endcode
 *
 * @param mbxid - The mbx id returned from sceKernelCreateMbx
 * @param pnum  - A pointer to where the number of threads which
 *                were waiting on the mbx should be stored (NULL
 *                if you don't care)
 *
 * @return < 0 on error
 */
int sceKernelCancelReceiveMbx(SceUID mbxid, int *pnum);

/**
 * Retrieve information about a messagebox.
 *
 * @param mbxid - UID of the messagebox to retrieve info for.
 * @param info - Pointer to a ::SceKernelMbxInfo struct to receive the info.
 *
 * @returns < 0 on error.
 */
int sceKernelReferMbxStatus(SceUID mbxid, SceKernelMbxInfo *info);


/* Alarms. */

/** Prototype for alarm handlers. */
typedef SceUInt (*SceKernelAlarmHandler)(void *common);

/** Struct containing alarm info */
typedef struct SceKernelAlarmInfo {
	/** Size of the structure (should be set before calling
	 * :: sceKernelReferAlarmStatus */
	SceSize		size;
	/* The current schedule */
	SceKernelSysClock schedule;
	/** Pointer to the alarm handler */
	SceKernelAlarmHandler handler;
	/** Common pointer argument */
	void *		common;
} SceKernelAlarmInfo;

/** 
 * Set an alarm.
 * @param clock - The number of micro seconds till the alarm occurrs.
 * @param handler - Pointer to a ::SceKernelAlarmHandler
 * @param common - Common pointer for the alarm handler
 *
 * @return A UID representing the created alarm, < 0 on error.
 */
SceUID sceKernelSetAlarm(SceUInt clock, SceKernelAlarmHandler handler, void *common);

/**
 * Set an alarm using a ::SceKernelSysClock structure for the time
 * 
 * @param clock - Pointer to a ::SceKernelSysClock structure
 * @param handler - Pointer to a ::SceKernelAlarmHandler
 * @param common - Common pointer for the alarm handler.
 * 
 * @return A UID representing the created alarm, < 0 on error.
 */
SceUID sceKernelSetSysClockAlarm(SceKernelSysClock *clock, SceKernelAlarmHandler handler, void *common);

/**
 * Cancel a pending alarm.
 *
 * @param alarmid - UID of the alarm to cancel.
 *
 * @return 0 on success, < 0 on error.
 */
int sceKernelCancelAlarm(SceUID alarmid);

/**
 * Refer the status of a created alarm.
 *
 * @param alarmid - UID of the alarm to get the info of
 * @param info - Pointer to a ::SceKernelAlarmInfo structure
 *
 * @return 0 on success, < 0 on error.
 */
int sceKernelReferAlarmStatus(SceUID alarmid, SceKernelAlarmInfo *info);

/* Callbacks. */

/** Callback function prototype */
typedef int (*SceKernelCallbackFunction)(int arg1, int arg2, void *arg);

/** Structure to hold the status information for a callback */
typedef struct SceKernelCallbackInfo {
	/** Size of the structure (i.e. sizeof(SceKernelCallbackInfo)) */
	SceSize 	size;
	/** The name given to the callback */
	char 	name[32];
	/** The thread id associated with the callback */
	SceUID 	threadId;
	/** Pointer to the callback function */
	SceKernelCallbackFunction 	callback;
	/** User supplied argument for the callback */
	void * 	common;
	/** Unknown */
	int 	notifyCount;
	/** Unknown */
	int 	notifyArg;
} SceKernelCallbackInfo;

/**
 * Create callback
 *
 * @par Example:
 * @code
 * int cbid;
 * cbid = sceKernelCreateCallback("Exit Callback", exit_cb, NULL);
 * @endcode
 *
 * @param name - A textual name for the callback
 * @param func - A pointer to a function that will be called as the callback
 * @param arg  - Argument for the callback ?
 *
 * @return >= 0 A callback id which can be used in subsequent functions, < 0 an error.
 */
int sceKernelCreateCallback(const char *name, SceKernelCallbackFunction func, void *arg);

/**
  * Gets the status of a specified callback.
  *
  * @param cb - The UID of the callback to refer.
  * @param status - Pointer to a status structure. The size parameter should be
  * initialised before calling.
  *
  * @return < 0 on error.
  */
int sceKernelReferCallbackStatus(SceUID cb, SceKernelCallbackInfo *status);


/* Misc. */

/** Threadman types for ::sceKernelGetThreadmanIdList */
enum SceKernelIdListType
{
	SCE_KERNEL_TMID_Thread = 1,
	SCE_KERNEL_TMID_Semaphore = 2,
	SCE_KERNEL_TMID_EventFlag = 3,
	SCE_KERNEL_TMID_Mbox = 4,
	SCE_KERNEL_TMID_Vpl = 5,
	SCE_KERNEL_TMID_Fpl = 6,
	SCE_KERNEL_TMID_Mpipe = 7,
	SCE_KERNEL_TMID_Callback = 8,
	SCE_KERNEL_TMID_ThreadEventHandler = 9,
	SCE_KERNEL_TMID_Alarm = 10,
	SCE_KERNEL_TMID_VTimer = 11,
	SCE_KERNEL_TMID_SleepThread = 64,
	SCE_KERNEL_TMID_DelayThread = 65,
	SCE_KERNEL_TMID_SuspendThread = 66,
	SCE_KERNEL_TMID_DormantThread = 67,
};

/**
  * Get a list of UIDs from threadman. Allows you to enumerate 
  * resources such as threads or semaphores.
  *
  * @param type - The type of resource to list, one of ::SceKernelIdListType.
  * @param readbuf - A pointer to a buffer to store the list.
  * @param readbufsize - The size of the buffer in SceUID units.
  * @param idcount - Pointer to an integer in which to return the number of ids in the list.
  *
  * @return < 0 on error. Either 0 or the same as idcount on success.
  */
int sceKernelGetThreadmanIdList(enum SceKernelIdListType type, SceUID *readbuf, int readbufsize, int *idcount);

/** Structure to contain the system status returned by ::sceKernelReferSystemStatus */
typedef struct SceKernelSystemStatus {
	/** Size of the structure (should be set prior to the call) */
	SceSize 	size;
	/** The status ? */
	SceUInt 	status;
	/** The number of cpu clocks in the idle thread */
	SceKernelSysClock 	idleClocks;
	/** Number of times we resumed from idle */
	SceUInt 	comesOutOfIdleCount;
	/** Number of thread context switches */
	SceUInt 	threadSwitchCount;
	/** Number of vfpu switches ? */
	SceUInt 	vfpuSwitchCount;
} SceKernelSystemStatus;

/**
  * Get the current system status.
  *
  * @param status - Pointer to a ::SceKernelSystemStatus structure.
  *
  * @return < 0 on error.
  */
int sceKernelReferSystemStatus(SceKernelSystemStatus *status);

//sceKernelCreateMsgPipe
//sceKernelDeleteMsgPipe
//sceKernelSendMsgPipe
//sceKernelSendMsgPipeCB
//sceKernelTrySendMsgPipe
//sceKernelReceiveMsgPipe
//sceKernelReceiveMsgPipeCB
//sceKernelTryReceiveMsgPipe
//sceKernelCancelMsgPipe

typedef struct SceKernelMppInfo {
	SceSize 	size;
	char 	name[32];
	SceUInt 	attr;
	int 	bufSize;
	int 	freeSize;
	int 	numSendWaitThreads;
	int 	numReceiveWaitThreads;
} SceKernelMppInfo;
 
/**
 * Get the status of a Message Pipe
 *
 * @param uid - The uid of the Message Pipe
 * @param info - Pointer to a ::SceKernelMppInfo structure
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReferMsgPipeStatus(SceUID uid, SceKernelMppInfo *info);

/* VPL Functions */
//sceKernelCreateVpl
//sceKernelDeleteVpl
//sceKernelAllocateVpl
//sceKernelAllocateVplCB
//sceKernelTryAllocateVpl
//sceKernelFreeVpl
//sceKernelCancelVpl

typedef struct SceKernelVplInfo {
	SceSize 	size;
	char 	name[32];
	SceUInt 	attr;
	int 	poolSize;
	int 	freeSize;
	int 	numWaitThreads;
} SceKernelVplInfo;

/**
 * Get the status of an VPL
 *
 * @param uid - The uid of the VPL
 * @param info - Pointer to a ::SceKernelVplInfo structure
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReferVplStatus(SceUID uid, SceKernelVplInfo *info);

/* FPL Functions */
//sceKernelCreateFpl
//sceKernelDeleteFpl
//sceKernelAllocateFpl
//sceKernelAllocateFplCB
//sceKernelTryAllocateFpl
//sceKernelFreeFpl
//sceKernelCancelFpl

typedef struct SceKernelFplInfo {
	SceSize 	size;
	char 	name[32];
	SceUInt 	attr;
	int 	blockSize;
	int 	numBlocks;
	int 	freeBlocks;
	int 	numWaitThreads;
} SceKernelFplInfo;

/**
 * Get the status of an FPL
 *
 * @param uid - The uid of the FPL
 * @param info - Pointer to a ::SceKernelFplInfo structure
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReferFplStatus(SceUID uid, SceKernelFplInfo *info);

/**
 * Return from a timer handler (doesn't seem to do alot)
 */
void _sceKernelReturnFromTimerHandler(void);

/**
 * Return from a callback (used as a syscall for the return 
 * of the callback function)
 */
void _sceKernelReturnFromCallback(void);

//sceKernelUSec2SysClock
//sceKernelUSec2SysClockWide
//sceKernelSysClock2USec
//sceKernelSysClock2USecWide
//sceKernelGetSystemTime
//sceKernelGetSystemTimeWide
//sceKernelGetSystemTimeLow

struct SceKernelVTimerOptParam {
	SceSize 	size;
};

/**
 * Create a virtual timer
 *
 * @param name - Name for the timer.
 * @param opt  - Pointer to an ::SceKernelVTimerOptParam (pass NULL)
 *
 * @return The VTimer's UID or < 0 on error.
 */
SceUID sceKernelCreateVTimer(const char *name, struct SceKernelVTimerOptParam *opt);

/**
 * Delete a virtual timer
 *
 * @param uid - The UID of the timer
 *
 * @return < 0 on error.
 */
int sceKernelDeleteVTimer(SceUID uid);

//sceKernelGetVTimerBase
//sceKernelGetVTimerBaseWide
//sceKernelGetVTimerTime
//sceKernelGetVTimerTimeWide
//sceKernelSetVTimerTime
//sceKernelSetVTimerTimeWide

/**
 * Start a virtual timer
 *
 * @param uid - The UID of the timer
 *
 * @return < 0 on error
 */
int sceKernelStartVTimer(SceUID uid);

/**
 * Stop a virtual timer
 *
 * @param uid - The UID of the timer
 *
 * @return < 0 on error
 */
int sceKernelStopVTimer(SceUID uid);

//sceKernelSetVTimerHandler
//sceKernelSetVTimerHandlerWide
//sceKernelCancelVTimerHandler

typedef SceUInt (*SceKernelVTimerHandler)(SceUID uid, SceKernelSysClock *, SceKernelSysClock *, void *);

typedef struct SceKernelVTimerInfo {
	SceSize 	size;
	char 	name[32];
	int 	active;
	SceKernelSysClock 	base;
	SceKernelSysClock 	current;
	SceKernelSysClock 	schedule;
	SceKernelVTimerHandler 	handler;
	void * 	common;
} SceKernelVTimerInfo;

/**
 * Get the status of a VTimer
 *
 * @param uid - The uid of the VTimer
 * @param info - Pointer to a ::SceKernelVTimerInfo structure
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReferVTimerStatus(SceUID uid, SceKernelVTimerInfo *info);

/**
 * Exit the thread (probably used as the syscall when the main thread
 * returns
 */
void _sceKernelExitThread(void);

//sceKernelGetThreadmanIdType

typedef int (*SceKernelThreadEventHandler)(int mask, SceUID thid, void *common);

/** Struct for event handler info */
struct SceKernelThreadEventHandlerInfo {
	SceSize 	size;
	char 	name[32];
	SceUID 	threadId;
	int 	mask;
	SceKernelThreadEventHandler 	handler;
	void * 	common;
};

/**
 * Register a thread event handler
 *
 * @param name - Name for the thread event handler
 * @param handler - Pointer to a ::SceKernelThreadEventHandler function
 * @param mask - Bit mask for what events to handle (only lowest 4 bits valid)
 * @param common - Common pointer
 *
 * @return The UID of the create event handler, < 0 on error
 */
SceUID sceKernelRegisterThreadEventHandler(const char *name, SceKernelThreadEventHandler handler, 
		int mask, void *common);

/**
 * Release a thread event handler.
 *
 * @param uid - The UID of the event handler
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReleaseThreadEventHandler(SceUID uid);

/**
 * Refer the status of an thread event handler
 *
 * @param uid - The UID of the event handler
 * @param info - Pointer to a ::SceKernelThreadEventHandlerInfo structure
 *
 * @return 0 on success, < 0 on error
 */
int sceKernelReferThreadEventHandlerStatus(SceUID uid, struct SceKernelThreadEventHandlerInfo *info);

/**
 * Get the thread profiler registers.
 * @return Pointer to the registers, NULL on error
 */
PspDebugProfilerRegs *sceKernelReferThreadProfiler(void);

/**
 * Get the globile profiler registers.
 * @return Pointer to the registers, NULL on error
 */
PspDebugProfilerRegs *sceKernelReferGlobalProfiler(void);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
