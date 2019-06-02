#!/bin/bash

# This script runs a test series, of the project
# See this script like a BDD test, but you still have to check,
# visually if the results are correct by seeing outputs givem by
# Scheduler at terminal
#
# OBS: It is highly recommended, set the debug
# print flag to 0 (Print no debug messages).
#
# OBS2: This script don't compile the programs
# make sure you have compiled every one, before continue
#
# OBS3: RUN THIS SCRIPT AT SAME PLACE OF THE EXECUTABLES


# Control variables
# TODO - se alguem conseguir ai, coloque aqui por favor controle de path
# (substituindo a necessidade de rodar isso junto aos executáveis)
# E colocar uma variavel para controle dos testes de argumento,
# e controle de quais topologias seram testadas via array, while sei la,
# executando os mesmo testes para todas as topologias demandadas

# Title (Context) - Magenta
function title(){
echo ""
echo -e "\e[35m -$1\e[0m"
}

# SubTitle (Sub context) - Blue
function subTitle(){
echo ""
echo -e "\e[34m --$1\e[0m"
}

# Test title - Cyan
function testTitle(){
echo -e "\e[36m ---$1\e[0m"
}

# Description  Dark gray
function description(){
echo -e "\e[90m -$1\e[0m"
}

# Run program function
function run(){
./$1
}

# Run program function without show prints
function hideRun(){
$(./$1)
}


#------------------------- Tests ---------------------------

if false; then
title "Test Verification of Arguments"
description "The following tests must fail, checking the arguments handler"

    subTitle "Test arguments of Execute"
        testTitle "with invalid executable"
            run "execute 10 \"fake_exec\""

        testTitle "with negative delay"
            run "execute -10 ./dummy"

        testTitle "with delay at end"
            run "execute ./dummy 10"

        testTitle "with just delay argument"
            run "execute 10"

    subTitle "Test arguments of Scheduler"
        testTitle "with no arguments"
            run "scheduler"

        testTitle "with invalid topology"
            run "scheduler \"fake_name"\"

    subTitle "Test arguments of Node"
        testTitle "without param id"
            hideRun "scheduler tree" &
            sleep 3
            run "node"
            sleep 3
            hideRun "shutdown"
        testTitle "with not number id"
            hideRun "scheduler tree" &
            sleep 3
            run "node id"
            sleep 3
            hideRun "shutdown"
fi

title "Test normal flow of program"
description "This tests represent a normal use of the project, using 'Tree' topology."
description "Including some fails case of executing, like execute non existent program or shutdown while something is executing"
description "For test purpose we will execute 'dummy' program that sleeps as demanded by param"
    subTitle "Start Scheduler"
        run "scheduler tree" &
        sleep 3

    subTitle "Run Execute and send a jobs to be executed to Scheduler"
        testTitle "Execute './dummy' with sleep: 1s delay:0s"
            run "execute 0 ./dummy"
        testTitle "Execute './dummy' with sleep: 5s and delay: 10s"
            run "execute 10 ./dummy 5"
        testTitle "Execute './dummy' with sleep: 15s and delay: 10s"
            run "execute 10 ./dummy 15"
        testTitle "Execute 'dummy' with delay of 0s"
        description "Must be send, but no executed, because left ./"
            run "execute 0 dummy"
        testTitle "Execute './dummy' with sleep: 1s and delay: 1s"
            run "execute 1 ./dummy"

        sleep 55    # wait for correct execution of job above

    subTitle "Run 3 simultaneous executes and send jobs that will not be execute"
    description "This jobs will not be executed because the shutdown will be called first"
        testTitle "Execute './dummy' with sleep: 1s and delay: 40s"
            run "execute 1 ./dummy 40" &
        testTitle "Execute './dummy' with sleep: 1s and delay: 50s"
            run "execute 1 ./dummy 50" &
        testTitle "Execute './dummy' with sleep: 1s and delay: 60s"
            run "execute 1 ./dummy 60"

        sleep 2     # wait, to not make mess with terminal colors, by background process

    subTitle "Stop Scheduler, by running Shutdown while a job is running"
    description "send one more job, but with great sleep, and shutdowns the scheduler"
        run "execute 0 ./dummy 15"
        run "shutdown"