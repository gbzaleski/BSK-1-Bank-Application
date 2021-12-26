#!/bin/bash

echo 'AllowGroups officers' >> etc/ssh/sshd_config
echo 'if [ -n "$SSH_CONNECTION" ] ; then
    /bank_officer_app
fi
' >> /etc/profile