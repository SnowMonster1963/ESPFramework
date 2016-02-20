/*
 * Process.h
 *
 *  Created on: Jan 2, 2016
 *      Author: tsnow
 */

#ifndef INCLUDE_DRIVER_PROCESS_H_
#define INCLUDE_DRIVER_PROCESS_H_

extern "C"
{
#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <user_interface.h>
}

class ProcessManager;

class ProcessTask
{
	friend class ProcessManager;

private:
	uint16_t taskID;
	ProcessManager &taskmgr;

protected:
	void setID(uint16_t id)
	{
		taskID = id;
	}
	;
	uint16_t getID(void)
	{
		return taskID;
	}
	;
	virtual void  runTask(const void *p) = 0;

public:
	ProcessTask(ProcessManager &mgr);
	virtual ~ProcessTask();

	void Post(const void *p);
};

template<class T,bool autodelete>
class UserTask: public ProcessTask
{
public:
	UserTask(ProcessManager &mgr) :
			ProcessTask(mgr)
	{

	};

	void PostMessage(const T *msg)
	{
		Post(msg);
	};


protected:
	virtual void OnMessage(const T *msg) = 0;

	void runTask(const void *p)
	{
		const T *msg = (const T *)p;
		OnMessage(msg);
		if(autodelete)
			delete msg;
	}
};

class ProcessManager
{
	friend class ProcessTask;

private:
	class TaskMessage
	{
	public:
		ProcessManager *mgr;
		const void *pMessage;

		TaskMessage(ProcessManager *m, const void *param)
		{
			mgr = m;
			pMessage = param;
		}
	};

private:
	static uint16_t taskIDs;
	ProcessTask **tasks;
	uint8_t taskcnt;
	uint8_t priority;os_event_t *queue;

	void Handle(os_signal_t id, const void *param);

protected:
	void Register(ProcessTask *task);
	void Deregister(ProcessTask *task);
	static void Handler(os_event_t *event);
	void Post(ProcessTask *task, const void *p);

public:
	ProcessManager(uint8_t queuelen = 16, uint8_t priority = 0);
	~ProcessManager();

};

extern ProcessManager Priority0Manager;
extern ProcessManager Priority1Manager;
extern ProcessManager Priority2Manager;

#endif /* INCLUDE_DRIVER_PROCESS_H_ */
