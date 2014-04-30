/*
 * Copyright 2013-2014 gtalent2@gmail.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "task.hpp"

namespace wombat {
namespace core {

// TaskState

TaskState::TaskState(uint64 sleep) {
	state = Running;
	sleepDuration = sleep;
}

TaskState::TaskState(TaskState::State state) {
	this->state = state;
	sleepDuration = 0;
}

// Task

Task::Task() {
	m_autoDelete = false;
}

Task::~Task() {
}

void Task::setAutoDelete(bool autoDelete) {
	m_autoDelete = true;
}

bool Task::autoDelete() {
	return m_autoDelete;
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

TaskProcessor::TaskProcessor(BaseSemaphore *sem) {
	m_running = false;
	if (sem) {
		m_sem = sem;
		m_semInternal = false;
	} else {
		m_sem = new Semaphore();
		m_semInternal = true;
	}
}

TaskProcessor::~TaskProcessor() {
	if (m_semInternal) {
		delete m_sem;
	}
}

TaskState TaskProcessor::run(Event post) {
	switch (post.type()) {
	case Timeout:
		// Timeout means something wants to run
		{
			while (1) {
				auto nt = popActiveTask();
				if (nt) {
					runTask(nt, Timeout);
				} else {
					break;
				}
			}
		}
		break;
	case ChannelMessage:
		runTask(post.task(), ChannelMessage);
		break;
	case SemaphorePost:
		// SemaphorePost is already designated for use only as a
		//  sleep refresh in this switch or exit the thread loop
		break;
	default:
		break;
	}

	std::pair<Task*, uint64> nt;
	if (nextTask(nt) == 0) {
		auto time = core::time();
		if (time < nt.second) {
			return nt.second - time;
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
	processTaskState(task, state);

	// post to the semaphore to refresh the sleep time
	m_sem->post();
}

void TaskProcessor::start() {
	if (!m_running) {
		m_running = true;
		startThread([this]() {
			TaskState taskState;
			while (m_running) {
				Event post;
				if (taskState.state == TaskState::Running) {
					post = m_sem->wait(taskState.sleepDuration);
				} else {
					post = m_sem->wait();
				}
				taskState = run(post);
			}
			m_done.write(true);
		});
	}
}

void TaskProcessor::stop() {
	m_sem->post();
	m_running = false;
}

void TaskProcessor::done() {
	m_done.read();
}

Task *TaskProcessor::popActiveTask() {
	std::pair<Task*, uint64> nt;
	m_mutex.lock();
	if (nextTask(nt) == 0) {
		auto time = core::time();
		if (time >= nt.second) {
			m_schedule.pop_back();
			m_mutex.unlock();
			return nt.first;
		}
	}
	return 0;
}

int TaskProcessor::nextTask(std::pair<Task*, uint64> &t) {
	int retval = 0;
	m_mutex.lock();
	if (m_schedule.empty()) {
		retval = 1;
	} else {
		t = m_schedule.back();
	}
	m_mutex.unlock();
	return retval;
}

void TaskProcessor::processTaskState(Task *task, TaskState state) {
	m_mutex.lock();
	switch (state.state) {
	case TaskState::Running:
		{
			const auto wakeup = time() + state.sleepDuration;
			const auto val = std::pair<Task*, uint64>(task, wakeup);

			bool inserted = false;
			for (auto i = m_schedule.begin(); i < m_schedule.end(); i++) {
				if (wakeup > i->second) {
					m_schedule.insert(i, val);
					inserted = true;
				}
			}

			if (!inserted) {
				m_schedule.push_back(val);
			}
		}
		break;
	case TaskState::Done:
		if (task->autoDelete()) {
			// remove from schedule
			for (auto i = 0; i < m_schedule.size(); i++) {
				if (m_schedule[i].first == task) {
					m_schedule.erase(m_schedule.begin() + i);
				}
			}

			// actually delete the Task
			delete task;
		}
		break;
	default:
		// do nothing
		break;
	}
	m_mutex.unlock();
}

void TaskProcessor::runTask(Task *task, Event event) {
	auto state = task->run(event);
	processTaskState(task, state);
}

}
}