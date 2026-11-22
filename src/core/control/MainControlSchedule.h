#pragma once

#include <utility>  // for forward

#include "control/jobs/Scheduler.h"         // for JOB_...
#include "control/jobs/XournalScheduler.h"  // for Xour...

#include "Control.h"  // TODO remove
#include "MainControl.h"

template <typename JobType, auto priority, typename... Args>
void MainControl::schedule(Args&&... args) {
    auto* job = new JobType(dynamic_cast<Control*>(this), std::forward<Args>(args)...);
    this->scheduler->addJob(job, priority);
    job->unref();
}
