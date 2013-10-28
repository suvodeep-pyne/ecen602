# creating new simulator
set ns [new Simulator]

# dumping data into these files
set nf [open out.nam w]
set tr1 [open out1.txt w]
$ns namtrace-all $nf
$ns trace-all $nf

proc finish {} {
	global ns nf tr1
	$ns flush-trace
	close $nf
	exec nam out.nam &
	close $tr1
	exit 0
}

proc record {} {
	global sink1 sink2 tr1 sum1 sum2 ratio
	set ns [Simulator instance]
	set time 10.0
	set bw1 [$sink1 set bytes_]
	set bw2 [$sink2 set bytes_]
	set now [$ns now]
	puts $tr1 "Src1: $now [expr $bw1/$time*8/1000000], Src2: $now [expr $bw2/$time*8/1000000]"
	if {$now > 100 && $now <= 400 }	{
		set sum1 [expr $sum1+$bw1/$time*8/1000000]
		set sum2 [expr $sum2+$bw2/$time*8/1000000]	
	}    
	if {$now == 400} {
		puts $tr1 "Average Throughput for Src1-Rcv1 link is [expr $sum1/30] Mbps"	
		puts $tr1 "Average Throughput in Src2-Rcv2 link is [expr $sum2/30] Mbps"
		set ratio [expr $sum1/$sum2]
		puts $tr1 "Ratio of Throughputs: [expr $ratio]"	
	}
	$sink1 set bytes_ 0
	$sink2 set bytes_ 0
	$ns at [expr $now+$time] "record"
}

# Create Nodes
set R1   [$ns node]
set R2   [$ns node]
set src1 [$ns node]
set src2 [$ns node]
set rcv1 [$ns node]
set rcv2 [$ns node]

$R1 label "R1"

# Initialize variables
set sum1 0
set sum2 0
set ratio 0

# Connect the nodes
$ns duplex-link $R1   $R2  1Mb    5ms DropTail
$ns duplex-link $src1 $R1 10Mb    0ms DropTail
$ns duplex-link $src2 $R1 10Mb 3.75ms DropTail
$ns duplex-link $rcv1 $R2 10Mb    0ms DropTail
$ns duplex-link $rcv2 $R2 10Mb 3.75ms DropTail

set tcp1 [new Agent/TCP/Vegas]
$ns attach-agent $src1 $tcp1
set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1

set tcp2 [new Agent/TCP/Vegas]
$ns attach-agent $src2 $tcp2
set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2

set sink1 [new Agent/TCPSink]
$ns attach-agent $rcv1 $sink1

set sink2 [new Agent/TCPSink]
$ns attach-agent $rcv2 $sink2

$ns connect $tcp1 $sink1
$ns connect $tcp2 $sink2

$tcp1 set class_ 1
$tcp2 set class_ 2
$ns color 1 Blue
$ns color 2 Red
$ns duplex-link-op $R1 $R2 queuePos 0.5

$ns at   0.0 "record"
$ns at   1.0 "$ftp1 start" 
$ns at   1.0 "$ftp2 start"
$ns at 401.0 "$ftp2 stop"
$ns at 401.0 "$ftp1 stop"
$ns at 402.0 "finish"

$ns run

