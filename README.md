csplugin-procwatch
==================

This plugin watches for process changes and can execute configurable actions.

ClearSync Plugin: Process Watch
-------------------------------

The process watch plugin uses the procps-ng API to watch processes for
configurable events such as start-up and terminate, etc.  When an event occurs
an associated "action-group" is executed.  Actions are delayed in a queue for
a configurable amount of time (in seconds) to prevent excessive executions from
a rapid flood of events.


CONFIGURATION
-------------

The process watch plugin has, at minimum, two configuration blocks.  The order
is arbitrary.  The first set is the <action-group> block.  This tag has one
required parameter, "name", which uniquely designates the action group.  The
second parameter is an optional integer value which sets the action group's
delay queue value in seconds.  If omitted, this defaults to 5 seconds.

Inside the <action-group> block are one or more <action> tags which simply
define a program/script to run.  There can be as many <action> tags as
required.  These actions are executed in the order which they are listed.

An example <action-group> may look like this:

  <action-group name="NetworkRestart" delay="5">
    <action>sudo service firewall restart</action>
    <action>sudo service ldap condrestart</action>
    <action>sudo service nmb condrestart</action>
  </action-group>

The second type of block in the process watch configuration file is event tags
which look something like <on-xxx>...</on-xxx>.  These tags define what
processes to watch for a given event type and what action group to execute
when triggered.  An example "process" watch type may look like:

  <on-startup type="text"
    action-group="NetworkRestart">suvad</on-startup>

This watch executes the "NetworkRestart" action group when the "suvad" process
starts.

There are currently two event types:

Tag               Description
-----------------------------------------------------------------------------
<on-startup>      Process has started.
<on-terminate>    Process has terminated.

vi: textwidth=79 syntax=txt
