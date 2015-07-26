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

#include "procwatch-conf.h"
#include "procwatch-action-group.h"
#include "csplugin-procwatch.h"

csPluginConf::csPluginConf(csPluginProcessWatch *parent,
    const char *filename, csPluginXmlParser *parser)
    : csConf(filename, parser), parent(parent),
    refresh_rate(_PROCWATCH_REFRESH_RATE)
{ }

void csPluginConf::Reload(void)
{
    csConf::Reload();
    parser->Parse();
}

void csPluginXmlParser::ParseElementOpen(csXmlTag *tag)
{
    if ((*tag) == "action-group") {
        if (!stack.size() || (*stack.back()) != "plugin")
            ParseError("unexpected tag: " + tag->GetName());
        if (!tag->ParamExists("name"))
            ParseError("name parameter missing");

        time_t delay = _PROCWATCH_DEFAULT_DELAY;
        if (tag->ParamExists("delay"))
            delay = (time_t)atoi(tag->GetParamValue("delay").c_str());

        csActionGroup *action_group = new csActionGroup(
            tag->GetParamValue("name"), delay);
        tag->SetData((void *)action_group);
    }
    else if ((*tag) == "action") {
        if (!stack.size() || (*stack.back()) != "action-group")
            ParseError("unexpected tag: " + tag->GetName());
    }
    else if ((*tag) == "on-start" || (*tag) == "on-terminate") {
        if (!stack.size() || (*stack.back()) != "plugin")
            ParseError("unexpected tag: " + tag->GetName());
        ParseProcWatchOpen(tag);
    }
}

void csPluginXmlParser::ParseElementClose(csXmlTag *tag)
{
    csPluginConf *_conf = static_cast<csPluginConf *>(conf);

    if ((*tag) == "refresh-interval") {
        if (!stack.size() || (*stack.back()) != "plugin")
            ParseError("unexpected tag: " + tag->GetName());
        _conf->refresh_rate = (unsigned int)atoi(tag->GetText().c_str());
        if (_conf->refresh_rate == 0)
            _conf->refresh_rate = _PROCWATCH_REFRESH_RATE;
    }
    else if ((*tag) == "action-group") {
        if (!stack.size() || (*stack.back()) != "plugin")
            ParseError("unexpected tag: " + tag->GetName());
        csActionGroup *action_group;
        action_group = reinterpret_cast<csActionGroup *>(tag->GetData());
        csPluginProcessWatch::csActionGroupMap::iterator agi;
        agi = _conf->parent->action_group.find(action_group->GetName());
        if (agi != _conf->parent->action_group.end()) {
            delete action_group;
            ParseError("duplicate action group: " + agi->second->GetName());
        }
        _conf->parent->action_group[action_group->GetName()] = action_group;
    }
    else if ((*tag) == "action") {
        if (!stack.size() || (*stack.back()) != "action-group")
            ParseError("unexpected tag: " + tag->GetName());
        csXmlTag *tag_parent = stack.back();
        csActionGroup *action_group;
        action_group = reinterpret_cast<csActionGroup *>(tag_parent->GetData());
        action_group->AppendAction(tag->GetText());
    }
    else if ((*tag) == "on-start" || (*tag) == "on-terminate") {
        if (!stack.size() || (*stack.back()) != "plugin")
            ParseError("unexpected tag: " + tag->GetName());
        ParseProcWatchClose(tag);
    }
}

void csPluginXmlParser::ParseProcWatchOpen(csXmlTag *tag)
{
    csPluginProcessWatch::csProcessState *watch_conf = NULL;

    try {
        watch_conf = new csPluginProcessWatch::csProcessState;
        watch_conf->state = csPluginProcessWatch::csPSTATE_INIT;
        watch_conf->last_action = time(NULL);
        watch_conf->pattern = NULL;

        if ((*tag) == "on-start")
            watch_conf->event = csPluginProcessWatch::csPEVENT_ON_START;
        else if ((*tag) == "on-terminate")
            watch_conf->event = csPluginProcessWatch::csPEVENT_ON_TERMINATE;

        if (!tag->ParamExists("type"))
            ParseError("type parameter missing");
        else if (tag->GetParamValue("type") == "text")
            watch_conf->type = csPluginProcessWatch::csPMATCH_TEXT;
        else if (tag->GetParamValue("type") == "pattern")
            watch_conf->type = csPluginProcessWatch::csPMATCH_PATTERN;
        else
            ParseError("invalid watch type: " + tag->GetParamValue("type"));
        
        if (!tag->ParamExists("action-group"))
            ParseError("action-group parameter missing");
        csPluginProcessWatch::csActionGroupMap::iterator i;
        csPluginConf *_conf = static_cast<csPluginConf *>(conf);
        i = _conf->parent->action_group.find(tag->GetParamValue("action-group"));
        if (i == _conf->parent->action_group.end()) {
            ParseError("action-group not found: " +
                tag->GetParamValue("action-group")
            );
        }
        watch_conf->action_group = tag->GetParamValue("action-group");
        watch_conf->retry_delay = _PROCWATCH_DEFAULT_RETRY;
        if (tag->ParamExists("retry-delay")) {
            watch_conf->retry_delay = (time_t)atoi(
                tag->GetParamValue("retry-delay").c_str()
            );
        }
        watch_conf->one_shot = false;
        if ((*tag) == "on-terminate" && tag->ParamExists("one-shot")) {
            watch_conf->one_shot =
                (tag->GetParamValue("one-shot") == "true") ? true : false;
        }

        tag->SetData((void *)watch_conf);
    }
    catch (exception &e) {
        if (watch_conf != NULL) delete watch_conf;
        throw;
    }
}

void csPluginXmlParser::ParseProcWatchClose(csXmlTag *tag)
{
    csPluginProcessWatch::csProcessState *watch_conf = NULL;

    try {
        watch_conf = reinterpret_cast<csPluginProcessWatch::csProcessState *>(tag->GetData());
        if (watch_conf->type == csPluginProcessWatch::csPMATCH_TEXT)
            watch_conf->text = tag->GetText();
        else if (watch_conf->type == csPluginProcessWatch::csPMATCH_PATTERN)
            watch_conf->pattern = new csRegEx(tag->GetText().c_str());
    }
    catch (exception &e) {
        if (watch_conf != NULL) delete watch_conf;
        throw;
    }

    csPluginConf *_conf = static_cast<csPluginConf *>(conf);
    _conf->parent->proc_state.push_back(watch_conf);
}

// vi: expandtab shiftwidth=4 softtabstop=4 tabstop=4
