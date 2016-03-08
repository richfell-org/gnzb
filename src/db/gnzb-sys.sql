BEGIN TRANSACTION;
drop table if exists NntpServers;
create table NntpServers (
	id integer primary key autoincrement,
	name varchar(64) not null,
	url varchar(64) not null,
	port varchar(6) not null default '119',
	user varchar(32) default null,
	password varchar(32) default null,
	connCount integer not null default 1,
	useSSL integer not null default 0,
	enabled integer not null default 1
);

drop table if exists SystemOptions;
create table SystemOptions (
	id integer primary key autoincrement,
	downloadDir varchar(256) not null,
	doAlwaysAskForDir integer not null default 0,
	doMoveToDir integer not null default 0,
	moveToDir varchar(256) default null,
	showNotifications integer not null default 0,
	soundOnNzbFinish integer not null default 0,
	soundOnQueueFinish integer not null default 0,
	nzbFinishSound varchar(256) default null,
	queueFinishSound varchar(256) default null,
	enableScripting integer not null default 0,
	runScriptOnNzbAdded integer not null default 0,
	scriptOnNzbAdded varchar(256) default null,
	runScriptOnNzbFinished integer not null default 0,
	scriptOnNzbFinished varchar(256) default null,
	runScriptOnNzbCancelled integer not null default 0,
	scriptOnNzbCancelled varchar(256) default null
);

INSERT INTO "SystemOptions" VALUES(1,'',0,0,NULL,0,0,0,NULL,NULL,0,0,NULL,0,NULL,0,NULL);

drop table if exists SystemAttributes;
create table SystemAttributes (
	id integer primary key,
	name varchar(128) default null,
	data blob not null
);

INSERT INTO "SystemAttributes" VALUES(1, 'Window State', x'000A000A00C800640064000000000000');
INSERT INTO "SystemAttributes" VALUES(2, 'Python Module', 'libpython2.7.so');

drop table if exists NzbGroups;
create table NzbGroups (
	id integer primary key autoincrement,
	metric integer not null,
	name varchar(32) not null,
	color varchar (32) not null default '#000000',
	doMove integer not null default 0,
	doMoveTv integer not null default 0,
	doCleanup integer not null default 0,
	doAutoAssign integer not null default 0,
	moveTo varchar(256) default null,
	autoAssignNameIncludes varchar(64) default null,
	autoAssignNameExcludes varchar(64) default null,
	autoAssignMinSize integer not null default 0,
	autoassignMaxSize integer not null default 0
);


INSERT INTO "NzbGroups" VALUES(0,0,'No group','rgba(243,243,243,255)',0,0,0,0,NULL,NULL,NULL,0,0);
INSERT INTO "NzbGroups" VALUES(1,1,'Movies HD','rgb(65,97,245)',0,0,1,1,'','720p, 1080p, x264','/.*\.[sS][[:digit:]]+[eE][[:digit:]]+\..*/',0,0);
INSERT INTO "NzbGroups" VALUES(2,2,'TV Shows','rgb(237,70,47)',0,0,1,1,'','/.*\.[sS][[:digit:]]+[eE][[:digit:]]+\..*/, hdtv','',0,0);
COMMIT;
