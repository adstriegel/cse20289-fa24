#!/bin/sh

ps aux | grep -v grep | grep -Eo 'bash' | wc -l

ps aux | grep -v grep | grep -Eo '(bash|csh|tcsh|zsh|sh)' | grep -v ssh | sort | uniq

for user in `ps aux | cut -d ' ' -f 1 | sort | uniq`; do echo $(ps aux | grep -v grep | grep $user | wc -l) $user; done | sort -rn
