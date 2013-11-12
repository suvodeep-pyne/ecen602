set ns [new Simulator]
set file1 [open Scen2Src1Red.txt w]
set file2 [open Scen2Src2Red.txt w]
set file3 [open Scen2Src3Red.txt w]

set H1 [$ns node]
set H2 [$ns node]
set H3 [$ns node]
set R1 [$ns node]
set R2 [$ns node]
set H4 [$ns node]
set H5 [$ns node]
set H6 [$ns node]

Queue/RED set thresh_ 10
Queue/RED set maxthresh_ 15
Queue/RED set linterm_ 50

$ns duplex-link $H1 $R1 10Mb 1ms RED
$ns duplex-link $H2 $R1 10Mb 1ms RED
$ns duplex-link $H3 $R1 10Mb 1ms RED
$ns duplex-link $R1 $R2 1Mb 10ms RED
$ns queue-limit $R1 $R2 20;
$ns duplex-link $R2 $H4 10Mb 1ms RED
$ns duplex-link $R2 $H5 10Mb 1ms RED
$ns duplex-link $R2 $H6 10Mb 1ms RED

set tcp1 [new Agent/TCP/Reno]
set tcp2 [new Agent/TCP/Reno]
set tcpsink1 [new Agent/TCPSink]
set tcpsink2 [new Agent/TCPSink]

$ns attach-agent $H1 $tcp1
$ns attach-agent $H2 $tcp2
$ns attach-agent $H4 $tcpsink1
$ns attach-agent $H5 $tcpsink2

$ns connect $tcp1 $tcpsink1
$ns connect $tcp2 $tcpsink2

set ftp1 [new Application/FTP]
$ftp1 attach-agent $tcp1

set ftp2 [new Application/FTP]
$ftp2 attach-agent $tcp2

#Create a UDP agent and attach it to node n0
set udp [new Agent/UDP]
$ns attach-agent $H3 $udp

# Create a CBR traffic source and attach it to udp0
set cbr [new Application/Traffic/CBR]
$cbr set type_ CBR
$cbr set packet_size_ 100
$cbr set rate_ 1Mb
$cbr set random_ false
$cbr attach-agent $udp

set udpsink [new Agent/LossMonitor] 
$ns attach-agent $H6 $udpsink

$ns connect $udp $udpsink

set sum1 0
set sum2 0
set sum3 0

proc finish {} {
	global file1 file2 file3
#Close the output files
		close $file1
		close $file2
		close $file3
		exit 0
}

proc record {} {
	global tcpsink1 tcpsink2 udpsink file1 file2 file3 sum1 sum2 sum3
#Get an instance of the simulator
		set ns [Simulator instance]
#Set the time after which the procedure should be called again
		set time 1.0
#How many bytes have been received by the traffic sinks?
		set bw0 [$tcpsink1 set bytes_]
		set bw1 [$tcpsink2 set bytes_]
		set bw2 [$udpsink set bytes_]
#Get the current time
		set now [$ns now]
#Calculate the throughput (in MBit/s) and write it to the files
		puts $file1 "$now      [expr $bw0/$time*8/1000]"
		puts $file2 "$now      [expr $bw1/$time*8/1000]"
		puts $file3 "$now      [expr $bw2/$time*8/1000]"


		if {$now >= 30 && $now <= 180 }	{

			set sum1 [expr $sum1+$bw0/$time*8/1000]
				set sum2 [expr $sum2+$bw1/$time*8/1000]	
				set sum3 [expr $sum3+$bw2/$time*8/1000]

		}    

	if {$now == 180} {

		puts $file1 "Average Throughput in Kbits/sec  is [expr $sum1/151]"	
			puts $file2 "Average Throughput in Kbits/sec in [expr $sum2/151]"
			puts $file3 "Average Throughput in Kbits/sec in [expr $sum3/151]"

	}

#Reset the bytes_ values on the traffic sinks
	$tcpsink1 set bytes_ 0
		$tcpsink2 set bytes_ 0
		$udpsink  set bytes_ 0
#Re-schedule the procedure
		$ns at [expr $now+$time] "record"
}

puts $file1 "Time   Packets"
puts $file2 "Time   Packets"
puts $file3 "Time   Packets"

$ns at 0.0 "record"
$ns at 1.0 "$ftp1 start"
$ns at 1.0 "$ftp2 start"
$ns at 1.0 "$cbr start"
$ns at 181.0 "$ftp1 stop"
$ns at 181.0 "$ftp2 stop"
$ns at 181.0 "$cbr stop"
$ns at 182.0 "finish"
