#!/usr/sbin/dtrace -s

#pragma D option ustackframes=100

profile:::profile-99 /pid == $target && arg1/ 
{ 
	@[ustack()] = count(); 
} 

tick-60s { exit(0); }
