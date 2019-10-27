#!/bin/bash
#
# Copyright (C) 2015 Niek Linnenbank
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

set -e
set -x

# Set hostname
echo master > /etc/hostname

# Auto select mirror
sed -i 's/us.archive.ubuntu.com/nl.archive.ubuntu.com/' /etc/apt/sources.list

# Remove hardcoded DNS servers
cat /etc/netplan/01-netcfg.yaml | grep -v 'nameservers:' | grep -v 'addresses: ' > /etc/netplan/01-netcfg.yaml.new
mv /etc/netplan/01-netcfg.yaml.new /etc/netplan/01-netcfg.yaml
netplan apply
sleep 5

# Update system to latest patches
apt-get update
apt-get dist-upgrade -y

# Install jenkins dependencies
apt-get install -y git default-jre daemon

# Add jenkins repository
wget -q -O - http://pkg.jenkins.io/debian/jenkins.io.key | apt-key add -
echo deb http://pkg.jenkins.io/debian-stable binary/ > /etc/apt/sources.list.d/jenkins.list
apt-get update

# Prevent automatic start of jenkins
mv /usr/bin/daemon /usr/bin/daemon.bak
cp /bin/true /usr/bin/daemon

# Install jenkins
if ! apt-get install -y jenkins; then
   # Ensure to restore the daemon program if installation failed
   mv /usr/bin/daemon.bak /usr/bin/daemon
   echo "Jenkins install failed: aborting"
   exit 1
fi

# Restore daemon program
mv /usr/bin/daemon.bak /usr/bin/daemon

# Disable the setup wizard
if [ "`grep runSetupWizard /etc/default/jenkins|wc -l`" -eq "0" ] ; then
   echo 'JAVA_ARGS="$JAVA_ARGS -Djenkins.install.runSetupWizard=false"' >> /etc/default/jenkins
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
mkdir -p $JENKINS_HOME/jobs/FreeNOS-freebsd12
mkdir -p $JENKINS_HOME/nodes
mkdir -p $JENKINS_HOME/nodes/ubuntu1804
mkdir -p $JENKINS_HOME/nodes/freebsd12
mkdir -p $JENKINS_HOME/secrets
mv ~vagrant/master.key $JENKINS_HOME/secrets/
mv ~vagrant/hudson.util.Secret $JENKINS_HOME/secrets/
mv ~vagrant/credentials.xml $JENKINS_HOME/
mv ~vagrant/ubuntu1804.job.xml $JENKINS_HOME/jobs/FreeNOS-ubuntu1804/config.xml
mv ~vagrant/ubuntu1804.node.xml $JENKINS_HOME/nodes/ubuntu1804/config.xml
mv ~vagrant/freebsd12.job.xml $JENKINS_HOME/jobs/FreeNOS-freebsd12/config.xml
mv ~vagrant/freebsd12.node.xml $JENKINS_HOME/nodes/freebsd12/config.xml

# Ensure permissions are set properly for jenkins
chown -R $JENKINS_USER:$JENKINS_GROUP $JENKINS_HOME

# Restart jenkins
/etc/init.d/jenkins stop
/etc/init.d/jenkins start
sleep 30

# Download jenkins client library
for i in {1..10}; do
    rm -f jenkins-cli.jar
    wget http://localhost:$HTTP_PORT/jnlpJars/jenkins-cli.jar -O jenkins-cli.jar || true
    if [ -s jenkins-cli.jar ] ; then
        break
    else
        sleep 5
    fi
done

# Install jenkins plugins
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin git
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin matrix-project
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin matrix-combinations-parameter
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin nodelabelparameter
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin ws-cleanup
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin junit
java -jar jenkins-cli.jar -s http://localhost:$HTTP_PORT/ -auth admin:admin install-plugin ssh-slaves

# Restart jenkins to load the new plugins
/etc/init.d/jenkins stop
/etc/init.d/jenkins start
