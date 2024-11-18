#!/bin/sh 

REPO_PATH="/home/maxleb/OS/HelloWorld"

while true
do 
    git --git-dir=$REPO_PATH/.git fetch origin main
    STATUS=$(git --git-dir=$REPO_PATH/.git rev-list --count HEAD...origin/main)

    if [ "$STATUS" != "0" ]; then
        git --git-dir=$REPO_PATH/.git --work-tree=$REPO_PATH pull origin main
    else
        :
    fi

    sleep 10
done
