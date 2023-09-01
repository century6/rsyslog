#!/bin/bash
# added 2023-10-05 by alorbach
# This file is part of the rsyslog project, released under ASL 2.0
. ${srcdir:=.}/diag.sh init
export NUMMESSAGES=2000
export SENDESSAGES=500
generate_conf
export PORT_RCVR="$(get_free_port)"
export TIMEOUT="5"

add_conf '
global(	defaultNetstreamDriverCAFile="'$srcdir/tls-certs/ca.pem'"
	defaultNetstreamDriverCertFile="'$srcdir/tls-certs/cert.pem'"
	defaultNetstreamDriverKeyFile="'$srcdir/tls-certs/key.pem'"
#	debug.whitelist="on"
#	debug.files=["nsd_ossl.c", "tcpsrv.c", "nsdsel_ossl.c", "nsdpoll_ptcp.c", "dnscache.c"]
)

module(	load="../plugins/imdtls/.libs/imdtls" )
# tls.authmode="anon" )
input(	type="imdtls"
	port="'$PORT_RCVR'"
	timeout="'$TIMEOUT'"
	)

template(name="outfmt" type="string" string="%msg:F,58:2%\n")
:msg, contains, "msgnum:" action(	type="omfile" 
					template="outfmt"
					file=`echo $RSYSLOG_OUT_LOG`)
'
# Begin actual testcase

startup
#	valgrind --tool=helgrind $RS_TEST_VALGRIND_EXTRA_OPTS $RS_TESTBENCH_VALGRIND_EXTRA_OPTS --log-fd=1 --error-exitcode=10 
./tcpflood -b1 -W1000 -p$PORT_RCVR -m$SENDESSAGES -Tdtls -x$srcdir/tls-certs/ca.pem -Z$srcdir/tls-certs/cert.pem -z$srcdir/tls-certs/key.pem -L0
# ./msleep 500
./tcpflood -b1 -W1000 -i500 -p$PORT_RCVR -m$SENDESSAGES -Tdtls -x$srcdir/tls-certs/ca.pem -Z$srcdir/tls-certs/cert.pem -z$srcdir/tls-certs/key.pem -L0
./tcpflood -b1 -W1000 -i1000 -p$PORT_RCVR -m$SENDESSAGES -Tdtls -x$srcdir/tls-certs/ca.pem -Z$srcdir/tls-certs/cert.pem -z$srcdir/tls-certs/key.pem -L0
./tcpflood -b1 -W1000 -i1500 -p$PORT_RCVR -m$SENDESSAGES -Tdtls -x$srcdir/tls-certs/ca.pem -Z$srcdir/tls-certs/cert.pem -z$srcdir/tls-certs/key.pem -L0

wait_file_lines
shutdown_when_empty
wait_shutdown
seq_check
exit_test
