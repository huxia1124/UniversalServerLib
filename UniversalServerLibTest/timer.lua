
	err, ret = xpcall(function()
	timerTick = timerTick or 0
	timerTick = timerTick + 1

		print("----------------".."(timer tick = "..timerTick..")-------------------")
		print("Receive: "..math.floor((server.ReceivedBytesPerSecond/1024)).." KB/s,\tPackages: "..server.ReceivedCountPerSecond.." pkg/sec,\t Current clients: "..server:GetTcpClientCount())
		print("         "..math.floor((server.TotalReceivedBytes/1024/1024)).." MB,\tPackages: "..server.TotalReceivedCount.." pkg")
		print("Send:    "..math.floor((server.SentBytesPerSecond/1024)).." KB/s,\tPackages: "..server.SentCountPerSecond.." pkg/sec")
		print("         "..math.floor((server.TotalSentBytes/1024/1024)).." MB,\tPackages: "..server.TotalSentCount.." pkg")

	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



