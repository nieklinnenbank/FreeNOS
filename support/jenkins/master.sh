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

#
# This script installs the jenkins master node on Ubuntu.
# Vagrant uses this script to automatically install and configure jenkins.
#
# See the README file for more details on the automated Jenkins setup.
#

JENKINS_VERSION="2.263.1"

# Include common functions
source common.sh

# Disable interactive dialogs for APT
export DEBIAN_FRONTEND=noninteractive

# Set hostname
echo master > /etc/hostname

# Auto select mirror
sed -i 's/us.archive.ubuntu.com/nl.archive.ubuntu.com/' /etc/apt/sources.list

# Remove hardcoded DNS servers
cat /etc/netplan/01-netcfg.yaml | grep -v 'nameservers:' | grep -v 'addresses: ' > /etc/netplan/01-netcfg.yaml.new
mv /etc/netplan/01-netcfg.yaml.new /etc/netplan/01-netcfg.yaml
netplan apply
sleep 5

# Ensure GRUB will not complain about out-of-sync installed bootloaders
firstdisk="`readlink -f /dev/disk/by-uuid/* | sort | head -n 1`"
echo "grub-pc grub-pc/install_devices multiselect $firstdisk" > /tmp/input.txt
debconf-set-selections /tmp/input.txt
rm -f /tmp/input.txt

# Update system to latest patches
run_command_retry "apt-get update"
run_command_retry "apt-get dist-upgrade -y"

# Install jenkins dependencies
run_command_retry "apt-get install -y git default-jre daemon"

# Add jenkins repository
run_command_retry "wget -q -O jenkins.io.key http://pkg.jenkins.io/debian-stable/jenkins.io.key"
apt-key add jenkins.io.key
echo deb http://pkg.jenkins.io/debian-stable binary/ > /etc/apt/sources.list.d/jenkins.list
run_command_retry "apt-get update"

# Prevent automatic start of jenkins
if [ ! -e /usr/bin/daemon.bak ] ; then
    mv /usr/bin/daemon /usr/bin/daemon.bak
fi
cp /bin/true /usr/bin/daemon

# Install jenkins
run_command_retry "apt-get install -y jenkins=$JENKINS_VERSION"
run_command_retry "apt-mark hold jenkins"

# Restore daemon program
mv /usr/bin/daemon.bak /usr/bin/daemon

# Disable the setup wizard and strict anti-CSRF session ID matching
if [ "`grep runSetupWizard /etc/default/jenkins|wc -l`" -eq "0" ] ; then
   echo 'JAVA_ARGS="$JAVA_ARGS -Djenkins.install.runSetupWizard=false"' >> /etc/default/jenkins
   echo 'JAVA_ARGS="$JAVA_ARGS -Dhudson.security.csrf.DefaultCrumbIssuer.EXCLUDE_SESSION_ID=true"' >> /etc/default/jenkins

    if [ ! -z "$TIMEZONE" ] ; then
       echo "JAVA_ARGS=\"\$JAVA_ARGS -Duser.timezone='$TIMEZONE'\"" >> /etc/default/jenkins
    fi
fi

source /etc/default/jenkins

# Add default admin user
mkdir -p $JENKINS_HOME/init.groovy.d/
cat > $JENKINS_HOME/init.groovy.d/01-basic-security.groovy << EOF
#!groovy

import jenkins.model.*
import hudson.util.*;
import jenkins.install.*;
import hudson.security.*
import jenkins.security.s2m.AdminWhitelistRule

def instance = Jenkins.getInstance()

def hudsonRealm = new HudsonPrivateSecurityRealm(false)
hudsonRealm.createAccount("admin", "admin")
instance.setSecurityRealm(hudsonRealm)

def strategy = new FullControlOnceLoggedInAuthorizationStrategy()
instance.setAuthorizationStrategy(strategy)
instance.save()

Jenkins.instance.getInjector().getInstance(AdminWhitelistRule.class).setMasterKillSwitch(false)
EOF

# Override number of executors, if cpu count is specified
if [ ! -z "$SLAVE_CPUS" ] ; then
    sed -i "s/<numExecutors>4<\/numExecutors>/<numExecutors>$SLAVE_CPUS<\/numExecutors>/" ~vagrant/*.node.xml
fi

# Add configuration files to jenkins
mkdir -p $JENKINS_HOME/jobs
mkdir -p $JENKINS_HOME/jobs/FreeNOS-ubuntu1804
mkdir -p $JENKINS_HOME/jobs/FreeNOS-ubuntu1804-loop
mkdir -p $JENKINS_HOME/jobs/FreeNOS-ubuntu1804-valgrind
mkdir -p $JENKINS_HOME/jobs/FreeNOS-freebsd12
mkdir -p $JENKINS_HOME/jobs/FreeNOS-freebsd12-loop
mkdir -p $JENKINS_HOME/nodes
mkdir -p $JENKINS_HOME/nodes/ubuntu1804
mkdir -p $JENKINS_HOME/nodes/freebsd12
mkdir -p $JENKINS_HOME/secrets
mkdir -p $JENKINS_HOME/plugins
mv ~vagrant/master.key $JENKINS_HOME/secrets/
mv ~vagrant/hudson.util.Secret $JENKINS_HOME/secrets/
mv ~vagrant/credentials.xml $JENKINS_HOME/
mv ~vagrant/ubuntu1804.job.xml $JENKINS_HOME/jobs/FreeNOS-ubuntu1804/config.xml
mv ~vagrant/ubuntu1804-loop.job.xml $JENKINS_HOME/jobs/FreeNOS-ubuntu1804-loop/config.xml
mv ~vagrant/ubuntu1804-valgrind.job.xml $JENKINS_HOME/jobs/FreeNOS-ubuntu1804-valgrind/config.xml
mv ~vagrant/ubuntu1804.node.xml $JENKINS_HOME/nodes/ubuntu1804/config.xml
mv ~vagrant/freebsd12.job.xml $JENKINS_HOME/jobs/FreeNOS-freebsd12/config.xml
mv ~vagrant/freebsd12-loop.job.xml $JENKINS_HOME/jobs/FreeNOS-freebsd12-loop/config.xml
mv ~vagrant/freebsd12.node.xml $JENKINS_HOME/nodes/freebsd12/config.xml

# Apply proper GIT branch to job files
if [ ! -z "$GIT_BRANCH" ] ; then
    sed -i "s,<name>\*/master</name>,<name>refs/heads/$GIT_BRANCH</name>,g" `find $JENKINS_HOME/jobs -maxdepth 2 -name 'config.xml'`
fi

# Jenkins plugin list with its dependencies
JENKINS_PLUGINS=(
    git/4.4.5 workflow-scm-step/2.11 workflow-step-api/2.23 credentials/2.3.13 git-client/3.5.1
              mailer/1.32.1 scm-api/2.6.4 script-security/1.75 ssh-credentials/1.18.1 structs/1.20
              apache-httpcomponents-client-4-api/4.5.10-2.0 jsch/0.1.55.2 display-url-api/2.3.3
    matrix-project/1.18 trilead-api/1.0.12
    matrix-combinations-parameter/1.3.1 bouncycastle-api/2.18 command-launcher/1.5 jdk-tool/1.4 jaxb/2.3.0.1
    nodelabelparameter/1.7.2 jquery/1.12.4-1 token-macro/2.12
    ws-cleanup/0.38 workflow-durable-task-step/2.36 workflow-api/2.40 workflow-support/3.6 durable-task/1.35 resource-disposer/0.14
    junit/1.43 valgrind/0.28 jquery-detached/1.2.1 workflow-cps/2.84 workflow-job/2.40 ace-editor/1.1
    bootstrap4-api/4.5.3-1 plugin-util-api/1.4.0 echarts-api/4.9.0-2 jackson2-api/2.11.3 checks-api/1.1.0
    jquery3-api/3.5.1-2 popper-api/1.16.0-7 font-awesome-api/5.15.1-1 snakeyaml-api/1.27.0
    ssh-slaves/1.31.2
    timestamper/1.11.8
)

# Install plugins
for p in "${JENKINS_PLUGINS[@]}"
do
    PLUGVER="`basename $p`"
    PLUGNAME="`dirname $p`"

    if [ ! -e $JENKINS_HOME/plugins/$PLUGNAME.hpi ] ; then
        run_command_retry "wget --quiet https://updates.jenkins.io/download/plugins/$PLUGNAME/$PLUGVER/$PLUGNAME.hpi -O $JENKINS_HOME/plugins/$PLUGNAME.hpi"
    fi
done

# Ensure permissions are set properly for jenkins
chown -R $JENKINS_USER:$JENKINS_GROUP $JENKINS_HOME

# Restart jenkins to load the new plugins and jobs
/etc/init.d/jenkins stop
/etc/init.d/jenkins start
