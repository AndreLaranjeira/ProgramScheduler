#!/bin/bash

ps -la
ps -la | grep $1 | cut -d " " -f 5 | while read pid; do
  echo $pid
  kill $pid
done
ps -la
