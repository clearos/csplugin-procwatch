// ClearSync: Process Watch plugin.
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

#include <clearsync/csplugin.h>

#include <proc/readproc.h>

#include "procwatch-conf.h"
#include "procwatch-action-group.h"
#include "csplugin-procwatch.h"

csPluginProcessWatch::csPluginProcessWatch(const string &name,
    csEventClient *parent, size_t stack_size)
    : csPlugin(name, parent, stack_size), conf(NULL)
{
    csLog::Log(csLog::Debug, "%s: Initialized.", name.c_str());
}

csPluginProcessWatch::~csPluginProcessWatch()
{
    Join();

    for (csProcessStateVector::iterator i = proc_state.begin();
        i != proc_state.end(); i++) {
        if ((*i)->pattern != NULL) delete (*i)->pattern;
    }
    for (csActionGroupMap::iterator i = action_group.begin();
        i != action_group.end(); i++) delete i->second;

    if (conf) delete conf;
}

void csPluginProcessWatch::SetConfigurationFile(const string &conf_filename)
{
    if (conf == NULL) {
        csPluginXmlParser *parser = new csPluginXmlParser();
        conf = new csPluginConf(this, conf_filename.c_str(), parser);
        parser->SetConf(dynamic_cast<csConf *>(conf));
        conf->Reload();
    }
}

void *csPluginProcessWatch::Entry(void)
{
    csLog::Log(csLog::Info, "%s: Started", name.c_str());

    csTimer *refresh_timer = new csTimer(_CSPLUGIN_PROCWATCH_REFRESH_TIMER,
        conf->GetRefreshRate(), conf->GetRefreshRate(), this);
    refresh_timer->Start();

    for (bool run = true; run; ) {
        csTimer *timer;
        csEvent *event = EventPopWait();

        if (event != NULL) {
            switch (event->GetId()) {
            case csEVENT_QUIT:
                csLog::Log(csLog::Debug, "%s: Terminated.", name.c_str());
                refresh_timer->Stop();
                run = false;
                break;

            case csEVENT_TIMER:
                timer = static_cast<csEventTimer *>(event)->GetTimer();

                if (timer->GetId() == _CSPLUGIN_PROCWATCH_REFRESH_TIMER) {
                    ProcessTableRefresh();
                    ProcessStateUpdate();
                    break;
                }
                for (csActionGroupMap::iterator i = action_group.begin();
                    i != action_group.end(); i++) {
                    if (*(i->second) != timer->GetId()) continue;
                    i->second->Execute(this, parent);
                    break;
                }
                break;
            }
        }

        EventDestroy(event);
    }

    delete refresh_timer;

    return NULL;
}

void csPluginProcessWatch::ProcessTableRefresh(void)
{
    PROCTAB *proc_tab = NULL;
    proc_t *proc_info = NULL;

    proc_map.clear();
    proc_tab = openproc(PROC_FILLSTAT);

    while (proc_tab != NULL && (proc_info = readproc(proc_tab, NULL)) != NULL) {
        if (proc_info->ppid == 1)
            proc_map[proc_info->cmd].push_back(proc_info->tid);
       
        freeproc(proc_info);
        proc_info = NULL;
    }

    if (proc_info != NULL) freeproc(proc_info);
    if (proc_tab != NULL) closeproc(proc_tab);
}

void csPluginProcessWatch::ProcessStateUpdate(void)
{
    for (csProcessStateVector::iterator i = proc_state.begin();
        i != proc_state.end(); i++) {
        csProcessTableMap::iterator match = proc_map.end();

        if ((*i)->type == csPMATCH_TEXT)
            match = proc_map.find((*i)->text);
        else if ((*i)->type == csPMATCH_PATTERN && (*i)->pattern != NULL) {
            for (csProcessTableMap::iterator j = proc_map.begin();
                j != proc_map.end(); j++) {
                if ((*i)->pattern->Execute(j->first.c_str()) == 0) {
                    match = j;
                    break;
                }
            }
        }

        csProcessStates state = csPSTATE_NOT_RUNNING;
        if (match != proc_map.end()) state = csPSTATE_RUNNING;

        if ((*i)->state == csPSTATE_INIT)
            (*i)->state = state;
        else if ((*i)->state != state) {

            csLog::Log(csLog::Debug, "%s: Process state changed: %d -> %d",
                name.c_str(), (*i)->state, state);

            (*i)->state = state;

            if ((*i)->event == csPEVENT_ON_START &&
                (*i)->state == csPSTATE_RUNNING)
                ProcessStateChange((*i));
            else if ((*i)->event == csPEVENT_ON_TERMINATE &&
                (*i)->state == csPSTATE_NOT_RUNNING &&
                (*i)->one_shot == true)
                ProcessStateChange((*i));
        }

        if ((*i)->event == csPEVENT_ON_TERMINATE &&
            (*i)->state == csPSTATE_NOT_RUNNING &&
            (*i)->one_shot == false)
            ProcessStateChange((*i));
    }
}

void csPluginProcessWatch::ProcessStateChange(csProcessState *state)
{
    if (time(NULL) > state->last_action) {
        csActionGroupMap::iterator i = action_group.find(state->action_group);
        if (i == action_group.end()) {
            csLog::Log(csLog::Error, "%s: Can't find action group: %s", name.c_str(),
                state->action_group.c_str());
            return;
        }
        if (state->event == csPEVENT_ON_START) {
            csLog::Log(csLog::Debug, "%s: Running on-start action-group: %s",
                name.c_str(), i->first.c_str());
        }
        else if (state->event == csPEVENT_ON_TERMINATE) {
            csLog::Log(csLog::Debug, "%s: Running on-terminate action-group: %s",
                name.c_str(), i->first.c_str());
        }
        i->second->ResetDelayTimer(this);
        state->last_action = time(NULL) + state->retry_delay;
    }
}

csPluginInit(csPluginProcessWatch);

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
