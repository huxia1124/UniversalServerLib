require 'test_add'

	err, ret = xpcall(function()
	
	    if math.random(40000) > 39996 then
		    --print("Received!! : TID = "..utils.GetThreadId())  
			print("-----------------------------------")
			print("Receive: "..math.floor((server.ReceiveBytesPerSecond/1024)).." KB/s,\tPackages: "..server.ReceiveCountPerSecond.." pkg/sec,\t Current clients: "..server:GetTcpClientCount())
			print("         "..math.floor((server.TotalReceivedBytes/1024/1024)).." MB,\tPackages: "..server.TotalReceivedCount.." pkg")
			print("Send:    "..math.floor((server.SentBytesPerSecond/1024)).." KB/s,\tPackages: "..server.SentCountPerSecond.." pkg/sec")
			print("         "..math.floor((server.TotalSentBytes/1024/1024)).." MB,\tPackages: "..server.TotalSentCount.." pkg")
		end
		
		--if math.random(30000) > 10000 then
		    local uid = utils.GetClientUid()
			--print(uid)
			server:SendStringToClient(uid, "AAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAASSSSSSSSSSSSSSSSSSSDDDDDDDDDDDDDDDDDDDD")
			
		--end
		
	end, debug.traceback)
	
	if (ret) then
		--print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



