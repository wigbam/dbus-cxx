#!/bin/sh

terminate_jobs() {
  kill $! > /dev/null 2>&1
}

trap "terminate_jobs" INT TERM

$1 server "$2" &
SERVER_PID=$!
sleep .1
$1 client "$2"
# get the exit code
EXIT_CODE=$?

# wait for server to be done
terminate_jobs
wait $SERVER_PID
exit $EXIT_CODE
