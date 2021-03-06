/*
 * Copyright 2013-2015 gtalent2@gmail.com
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "_misc.hpp"
#include "_tls.hpp"
#include "_threads.hpp"
#include "core.hpp"
#include "corecapabilities.hpp"
#include "task.hpp"

namespace wombat {
namespace core {

SubscriptionManager _submgr;

int subscribe(Event::Type et) {
	int retval = 0;
	auto tp = activeTaskProcessor();
	if (tp) {
		tp->addSubscription(et);
	} else {
		retval = 1;
	}
	return retval;
}

// TaskState

TaskState::TaskState(uint64_t sleep) {
	state = Running;
	wakeupTime = _schedTime() + sleep;
}

TaskState::TaskState(TaskState::State state) {
	this->state = state;
	wakeupTime = 0;
}

// Task

Task::~Task() {
}

void Task::setAutoDelete(bool autoDelete) {
	m_autoDelete = true;
}

void Task::post(Event event) {
	if (m_taskProcessor != nullptr) {
		event.setTask(this);
		event.setTaskPost();
		m_taskProcessor->post(event);
	}
}

void Task::init() {
}

bool Task::autoDelete() {
	return m_autoDelete;
}

void Task::_setTaskProcessor(TaskProcessor *tp) {
	this->m_taskProcessor = tp;
}

// FunctionTask

FunctionTask::FunctionTask(std::function<TaskState(Event)> func) {
	m_task = func;
	setAutoDelete(true);
}

FunctionTask::~FunctionTask() {
}

TaskState FunctionTask::run(Event e) {
	return m_task(e);
}

// TaskProcessor

TaskProcessor::TaskProcessor(BaseEventQueue *sem) {
	m_currentTask = nullptr;
	if (sem != nullptr) {
		m_events = sem;
		m_semInternal = false;
	} else {
		m_events = new EventQueue();
		m_semInternal = true;
	}
}

TaskProcessor::~TaskProcessor() {
	if (m_semInternal) {
		delete m_events;
	}
}

TaskState TaskProcessor::run(Event event) {
	// preserve previous TaskProcessor and restore, this allows for
	//  TaskProcessor nesting
	const auto prevTp = activeTaskProcessor();

	setActiveTaskProcessor(this);

	if (event.type() < Event::OptionalEventTypeRange) { // is optional
		auto &subs = m_submgr.subs(event.type());
		for (auto t : subs) {
			runTask(t, event);
		}
	} else if (!event.getTaskPost()) {
		switch (event.type()) {
			case Event::Timeout: // Timeout means something wants to run
				// put a limit on the number of Tasks processed in a single iteration
				{
					auto nt = popActiveTask();
					if (nt) {
						runTask(nt, event);
					} else {
						break;
					}
				}
				break;
			case Event::ChannelMessage:
				runTask(event.task(), event);
				break;
			case Event::InitTask:
				if (event.task()) {
					m_currentTask = event.task();
					event.task()->init();
					processTaskState(event.task(), TaskState::Running);
					m_currentTask = nullptr;
				}
				break;
			case Event::GenericPost:
				// GenericPost is already designated for use only as a
				//  sleep refresh in this switch or exit the thread loop
				break;
			default:
				break;
		}
	} else if (event.task()) {
		runTask(event.task(), event);
	}

	setActiveTaskProcessor(prevTp);

	TaskProcessor::ScheduleItem nt;
	if (nextTask(&nt) == 0) {
		auto time = _schedTime();
		if (time < nt.wakeupTime) {
			return nt.wakeupTime - time;
		} else {
			return 0;
		}
	} else {
		return TaskState::Waiting;
	}
}

void TaskProcessor::addTask(std::function<TaskState(Event)> task, TaskState state) {
	addTask(new FunctionTask(task), state);
}

void TaskProcessor::addTask(Task *task, TaskState state) {
	task->_setTaskProcessor(this);
	m_tasks.push_back(task);
	// post to the semaphore to refresh the sleep time
	m_events->post(Event(Event::InitTask, task));
}

void TaskProcessor::start() {
	m_mutex.lock();
	if (!m_running) {
		m_running = true;
		if (SupportsThreads) {

			startThread([this]() {
				TaskState taskState;
				Event post;

				// event loop
				while (m_running) {
					if (taskState.state == TaskState::Running) {
						const auto time = _schedTime();
						if (time < taskState.wakeupTime) {
							post = m_events->wait(taskState.wakeupTime - time);
						} else {
							post = m_events->wait(0);
						}
					} else {
						post = m_events->wait();
					}
					taskState = run(post);
				}

				// send FinishTask to all Tasks
				for (auto t : m_tasks) {
					runTask(t, Event::FinishTask);
				}

				m_done.write(true);
			});

		} else {
			core::addTask(this);
		}
	}
	m_mutex.unlock();
}

void TaskProcessor::stop() {
	m_running = false;
	m_events->post();
}

void TaskProcessor::done() {
	m_done.read();
}

void TaskProcessor::post(Event event) {
	m_events->post(event);
}

void TaskProcessor::addSubscription(Event::Type et) {
	if (m_submgr.subs(et).size() == 0) {
		_submgr.addSubscription(et, this);
	}
	if (m_currentTask) {
		m_submgr.addSubscription(et, m_currentTask);
	}
}

Task *TaskProcessor::popActiveTask() {
	TaskProcessor::ScheduleItem nt;
	m_mutex.lock();
	if (nextTask(&nt) == 0) {
		auto time = _schedTime();
		if (time >= nt.wakeupTime) {
			m_schedule.pop_back();
			m_mutex.unlock();
			return nt.task;
		}
	}
	return nullptr;
}

int TaskProcessor::nextTask(TaskProcessor::ScheduleItem *t) {
	int retval = 0;
	m_mutex.lock();
	if (m_schedule.empty()) {
		retval = 1;
	} else {
		*t = m_schedule.back();
	}
	m_mutex.unlock();
	return retval;
}

void TaskProcessor::processTaskState(Task *task, TaskState state) {
	m_mutex.lock();
	switch (state.state) {
		case TaskState::Running:
			{
				// remove old wake up time
				deschedule(task);

				const auto wakeup = state.wakeupTime;
				const auto val = TaskProcessor::ScheduleItem(task, wakeup);

				bool inserted = false;
				for (uint i = 0; i < m_schedule.size(); i++) {
					auto ptr = m_schedule.begin() + i;
					if (wakeup > ptr->wakeupTime) {
						m_schedule.insert(ptr, val);
						inserted = true;
						break;
					}
				}

				if (!inserted) {
					m_schedule.push_back(val);
				}
			}
			break;
		case TaskState::Waiting:
			// make sure the Task is not in the schedule
			deschedule(task);
			break;
		case TaskState::Done:
			deschedule(task);
			m_submgr.removeFromAllSubs(task);
			for (size_t i = 0; i < m_tasks.size(); i++) {
				auto t = m_tasks[i];
				if (t == task) {
					m_tasks.erase(m_tasks.begin() + i);
				}
			}
			if (task->autoDelete()) {
				delete task;
			}
			break;
		case TaskState::Continue:
			// Continue exists to do nothing
			break;
		default:
			// do nothing
			break;
	}
	m_mutex.unlock();
}

void TaskProcessor::runTask(Task *task, Event event) {
	auto prevTask = m_currentTask;
	m_currentTask = task;
	auto state = task->run(event);
	processTaskState(task, state);
	m_currentTask = prevTask;
}

void TaskProcessor::deschedule(Task *task) {
	// remove from schedule
	for (uint i = 0; i < m_schedule.size(); i++) {
		if (m_schedule[i].task == task) {
			m_schedule.erase(m_schedule.begin() + i);
		}
	}
}

Task *TaskProcessor::activeTask() {
	return m_currentTask;
}

}
}
