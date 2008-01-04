/* Definition of the queue support module.
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */

#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#include <pthread.h>

/* queue types */
typedef enum {
	QUEUETYPE_FIXED_ARRAY = 0,/* a simple queue made out of a fixed (initially malloced) array fast but memoryhog */
	QUEUETYPE_LINKEDLIST = 1, /* linked list used as buffer, lower fixed memory overhead but slower */
	QUEUETYPE_DISK = 2 	  /* disk files used as buffer */
} queueType_t;

/* list member definition for linked list types of queues: */
typedef struct qLinkedList_S {
	struct qLinkedList_S *pNext;
	void *pUsr;
} qLinkedList_t;

/* the queue object */
typedef struct queue_s {
	queueType_t	qType;
	int	iQueueSize;	/* Current number of elements in the queue */
	int	iMaxQueueSize;	/* how large can the queue grow? */
	pthread_t thrdWorker;	/* ID of the worker thread associated with this queue */
	int	bDoRun;		/* 1 - run queue, 0 - shutdown of queue requested */
	rsRetVal (*pConsumer)(void *); /* user-supplied consumer function for dequeued messages */
	/* type-specific handlers (set during construction) */
	rsRetVal (*qConstruct)(struct queue_s *pThis);
	rsRetVal (*qDestruct)(struct queue_s *pThis);
	rsRetVal (*qAdd)(struct queue_s *pThis, void *pUsr);
	rsRetVal (*qDel)(struct queue_s *pThis, void **ppUsr);
	/* the following two are currently only required for disk queuing, but
	 * we keep them global because we otherwise needed to change the interface
	 * too much.
	 */
	rsRetVal (*serializer)(uchar **ppOutBuf, size_t *lenBuf, void *pUsr);
	rsRetVal (*deSerializer)(void *ppUsr, uchar *ppBuf, size_t lenBuf);
	/* end type-specific handler */
	/* synchronization variables */
	pthread_mutex_t *mut;
	pthread_cond_t *notFull, *notEmpty;
	/* end sync variables */
	union {			/* different data elements based on queue type (qType) */
		struct {
			long head, tail;
			void** pBuf;		/* the queued user data structure */
		} farray;
		struct {
			qLinkedList_t *pRoot;
			qLinkedList_t *pLast;
		} linklist;
		struct {
			uchar *pszSpoolDir;
			size_t lenSpoolDir;
			uchar *pszFilePrefix;
			size_t lenFilePrefix;
			int iCurrFileNum;	/* number of file currently processed */
			int fd;		/* current file descriptor */
			long iWritePos; /* next write position offset */
			long iReadPos;	/* next read position offset */
		} disk;
	} tVars;
} queue_t;


/* prototypes */
rsRetVal queueDestruct(queue_t *pThis);
rsRetVal queueEnqObj(queue_t *pThis, void *pUsr);
rsRetVal queueConstruct(queue_t **ppThis, queueType_t qType, int iMaxQueueSize, rsRetVal (*pConsumer)(void*),
	rsRetVal (*serializer)(uchar **ppOutBuf, size_t *lenBuf, void *pUsr),
	rsRetVal (*deSerializer)(void *ppUsr, uchar *ppBuf, size_t lenBuf)
	);

#endif /* #ifndef QUEUE_H_INCLUDED */
