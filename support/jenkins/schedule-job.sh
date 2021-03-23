#!/bin/bash
#
# Copyright (C) 2019 Niek Linnenbank
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

JENKINS_URL="http://192.168.50.10:8080"
JENKINS_USER="admin"
JENKINS_PASS="admin"
WAITTIME=15
RETRIES=20

# Number of jobs to submit
if [ -z "$JOB_COUNT" ] ; then
    JOB_COUNT=1
fi

# Trace execution and stop on errors
set -e
set -x

for i in $(seq 1 $JOB_COUNT) ; do
    for j in $(seq 0 $RETRIES) ; do

        # Fetch the crumb: a token to prevent cross site request forgery
        CRUMB="`curl --silent -X GET $JENKINS_URL/crumbIssuer/api/json --user $JENKINS_USER:$JENKINS_PASS | cut -d , -f 2 | cut -d : -f 2`"
        CRUMB="`echo $CRUMB | sed s/\\"//g`"
        echo "Using Jenkins crumb for Job request: $CRUMB"

        # Just fetch the jenkins URL with the given build token (configured per job)
        if curl -XPOST --silent --show-error $JENKINS_URL/job/$1/buildWithParameters?token=$1-JobToken --user $JENKINS_USER:$JENKINS_PASS -H "Jenkins-Crumb: $CRUMB"; then
            echo "Job $1 started via master at $JENKINS_URL"
            sleep $WAITTIME
            break
        fi

        # Wait before retrying
        if [ $j -lt $RETRIES ] ; then
            sleep $WAITTIME
        else
            echo "Failed to trigger build job $1 (tried $RETRIES times)"
            exit 1
        fi
    done
done
