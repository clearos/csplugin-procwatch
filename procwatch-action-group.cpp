// ClearSync: file watch plugin.
// Copyright (C) 2011 ClearFoundation <http://www.clearfoundation.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

#include <clearsync/csplugin.h>

#include "procwatch-conf.h"
#include "procwatch-action-group.h"
#include "csplugin-procwatch.h"

cstimer_id_t csActionGroup::timer_index = _PROCWATCH_DELAY_TIMER_ID;

csActionGroup::csActionGroup(const string &name, time_t delay)
    : name(name), delay(delay), timer(NULL) { }

csActionGroup::~csActionGroup()
{
    if (timer != NULL) delete timer;
}

void csActionGroup::ResetDelayTimer(csPluginProcessWatch *plugin)
{
    if (timer != NULL)
        timer->SetValue(delay);
    else {
        cstimer_id_t id = 0;

        ::csCriticalSection::Lock();
        id = timer_index++;
        if (id == 0) id = _PROCWATCH_DELAY_TIMER_ID;
        ::csCriticalSection::Unlock();

        timer = new csTimer(id, delay, 0, plugin);
        timer->Start();
    }
}

void csActionGroup::Execute(csEventClient *src, csEventClient *dst)
{
    int rc;
    sigset_t signal_set;
    vector<string>::iterator i;

    delete timer; timer = NULL;

    for (i = action.begin(); i != action.end(); i++) {
        sigemptyset(&signal_set);
        sigaddset(&signal_set, SIGCHLD);
        sigaddset(&signal_set, SIGALRM);
        if ((rc = pthread_sigmask(SIG_UNBLOCK, &signal_set, NULL)) != 0) {
            csLog::Log(csLog::Error, "%s: pthread_sigmask: %s",
                name.c_str(), strerror(rc));
        }

        int rc = ::csExecute((*i));
        csLog::Log(csLog::Debug,
            "%s: %s: %d", name.c_str(), (*i).c_str(), rc);
        if (rc != 0) {
            csLog::Log(csLog::Warning,
                "%s: %s: %d", name.c_str(), (*i).c_str(), rc);
        }

        sigemptyset(&signal_set);
        sigaddset(&signal_set, SIGCHLD);
        sigaddset(&signal_set, SIGALRM);
        if ((rc = pthread_sigmask(SIG_BLOCK, &signal_set, NULL)) != 0) {
            csLog::Log(csLog::Error, "%s: pthread_sigmask: %s",
                name.c_str(), strerror(rc));
        }
    }
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
