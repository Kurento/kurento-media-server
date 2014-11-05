#! /bin/sh

### BEGIN INIT INFO
# Provides:          kurento-media-serverd
# Required-Start:    $remote_fs $network
# Required-Stop:     $remote_fs
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Kurento Media Server daemon
### END INIT INFO

if [ -r "/lib/lsb/init-functions" ]; then
  . /lib/lsb/init-functions
else
  echo "E: /lib/lsb/init-functions not found, package lsb-base needed"
  exit 1
fi

# Variables
PID_FILE=/var/run/kurento-media-server.pid
DAEMON_CMD=/usr/bin/kurento-media-server
START_DAEMON=false
DAEMON_LOG=/var/log/kurento-media-server
LOG_SIZE=10M

# Include stun defaults if available
if [ -f /etc/default/kurento-media-server ] ; then
	. /etc/default/kurento-media-server
fi

if [ "$START_DAEMON" != "true" ]; then
    log_failure_msg "Review activate settings within file /etc/default/kurento-media-server"
    exit 1
fi

if [ ! -x $DAEMON_CMD ]; then
    log_failure_msg "$DAEMON_CMD does not have execution permission"
    exit 1
fi

#Helper function to verify user
verify_user () {
# Only root can start Kurento
    if [ `id -u` -ne 0 ]; then
        log_failure_msg "Only root can start Kurento Media Server"
        exit 1
    fi
}

# Helper function to check status
check_status() {
    pidofproc -p  $PID_FILE $DAEMON_CMD >/dev/null 2>&1
}


start_kurento () {

    verify_user

    # Verify pid file directory exists
    if [ ! -e /var/run ]; then
        install -d -m755 /var/run || { log_failure_msg "Unable to access /var/run directory"; exit 1; }
    fi
    # Verify log directory exists
    if [ ! -e $DAEMON_LOG ]; then
        install -d -o$DAEMON_USER -m755 $DAEMON_LOG || { log_failure_msg "Unable to access $DAEMON_LOG"; exit 1; }
    fi
    # Make sure the directory belongs to $DAEMON_USER
    sudo -u $DAEMON_USER -H [ -O $DAEMON_LOG ]
    if [ $? != 0 ]; then
        chown $DAEMON_USER $DAEMON_LOG || { log_failure_msg "Unable to access $DAEMON_LOG"; exit 1; }
    fi
    /sbin/start-stop-daemon --start --exec $DAEMON_CMD --pidfile "$PID_FILE" \
        --chuid $DAEMON_USER --background --no-close --make-pidfile 1>>"$DAEMON_LOG/media-server.log" 2>&1
    if [ $? != 0 ]; then
        log_failure_msg "Kurento Media Server already started"
	return
    fi

    # Add file rotation
    [ -e /etc/logrotate.d ] || install -d -m755 /etc/logrotate.d
    cat > /etc/logrotate.d/kurento-media-server  <<-EOFile
    "/var/log/kurento-media-server/*.log" {
        missingok
        copytruncate
        rotate 20
        size $LOG_SIZE
        compress
        notifempty
        sharedscripts
    }
	EOFile

    # Add logrotate cron to root user
    echo "`crontab -u root -l`"|grep -iv "kurento-media-server"|crontab -u root -
    echo "*/5 * * * * /usr/sbin/logrotate /etc/logrotate.d/kurento-media-server" | crontab -u root -

}

stop_kurento () {
    verify_user

    /sbin/start-stop-daemon --stop --exec $DAEMON_CMD --pidfile "$PID_FILE"
    [ $? != 0 ] && log_failure_msg "Kurento Media Server not running"
    [ -f $PID_FILE ] && rm -f $PID_FILE

    # Remove logrotate cron to root user
    echo "`crontab -u root -l`"|grep -iv "kurento-media-server"|crontab -u root -
}

status () {
    log_action_begin_msg "checking Kurento Media Server"

    check_status
    status=$?
    if [ $status -eq 0 ]; then
        read pid < $PID_FILE
        log_action_cont_msg "$DAEMON_CMD is running with pid $pid"
        log_action_end_msg 0
    elif [ $status -eq 1 ]; then
        log_action_cont_msg "$DAEMON_CMD is not running but the pid file exists"
        log_action_end_msg 1
    elif [ $status -eq 3 ]; then
        log_action_cont_msg "$DAEMON_CMD is not running"
        log_action_end_msg 3
    else
        log_action_cont_msg "Unable to determine $DAEMON_CMD status"
        log_action_end_msg 4
    fi
}

case "$1" in
  start)
    log_daemon_msg "Starting Kurento Media Server"
    start_kurento
    log_end_msg $?;
    ;;

  stop)
    log_daemon_msg "Stop Kurento Media Server daemon";
    stop_kurento
    log_end_msg $?;
    ;;

  restart)
    $0 stop
    $0 start
    ;;

  force-reload)
    $0 stop
    $0 start
    ;;

  status)
    status
    ;;

  *)
    echo "Usage: $0 {start|stop|restart|status}" >&2
    ;;
esac

exit 0
