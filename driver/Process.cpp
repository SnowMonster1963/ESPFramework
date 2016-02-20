/*
 * Process.cpp
 *
 *  Created on: Jan 2, 2016
 *      Author: tsnow
 */

#include "driver/Process.h"

ProcessManager Priority0Manager;
ProcessManager Priority1Manager;
ProcessManager Priority2Manager;


///////////// Task Implementation /////////////////////
ICACHE_FLASH_ATTR ProcessTask::ProcessTask(ProcessManager &mgr) : taskmgr(mgr)
{
	mgr.Register(this);
}


ICACHE_FLASH_ATTR ProcessTask::~ProcessTask()
{
	taskmgr.Deregister(this);
}


ICACHE_FLASH_ATTR void ProcessTask::Post(const void *p)
{
	taskmgr.Post(this,p);
}


////////////////// Task Manager Implementation ///////////////////
uint16_t ProcessManager::taskIDs = 0;

ICACHE_FLASH_ATTR ProcessManager::ProcessManager(uint8_t queuelen,uint8_t priority)
{
	tasks = NULL;
	taskcnt = 0;
	queue = new os_event_t[queuelen];
	memset(queue,0,sizeof(os_event_t) * queuelen);
	this->priority = priority;
	system_os_task(Handler, priority, queue, queuelen);
}

ICACHE_FLASH_ATTR ProcessManager::~ProcessManager()
{
	delete [] queue;
	if(tasks != NULL)
		delete [] tasks;
}

ICACHE_FLASH_ATTR void ProcessManager::Register(ProcessTask *task)
{
	taskIDs++;
	task->setID(taskIDs);
	if(tasks == NULL)
	{
		tasks = new ProcessTask *[1];
	}
	else
	{
		ProcessTask **p = new ProcessTask *[taskcnt+1];
		for(int i=0;i<taskcnt;i++)
			p[i] = tasks[i];
		delete [] tasks;
		tasks = p;
	}

	tasks[taskcnt++] = task;
}

ICACHE_FLASH_ATTR void ProcessManager::Deregister(ProcessTask *task)
{
	if(tasks != NULL)
	{
		for(int i=0;i<taskcnt;i++)
		{
			ProcessTask *p = tasks[i];
			if(task->getID() == p->getID())
			{
				tasks[i] = NULL;
				for(;i<taskcnt-1;i++)
				{
					tasks[i] = tasks[i+1];
				}
				taskcnt--;
				return;
			}
		}
	}
}

ICACHE_FLASH_ATTR void ProcessManager::Post(ProcessTask *task,const void *p)
{
	TaskMessage *tm = new TaskMessage(this,p);
	system_os_post(priority,(os_signal_t)task->getID(),(os_param_t)tm);
}

ICACHE_FLASH_ATTR void ProcessManager::Handle(os_signal_t id,const void *param)
{
	for(int i=0;i<taskcnt;i++)
	{
		if(tasks[i]->getID() == id)
		{
			tasks[i]->runTask(param);
		}
	}
}

ICACHE_FLASH_ATTR void ProcessManager::Handler(os_event_t *event)
{
	TaskMessage *tm = (TaskMessage *)event->par;
	ProcessManager *pm = tm->mgr;
	pm->Handle(event->sig,tm->pMessage);
	delete tm;
}

