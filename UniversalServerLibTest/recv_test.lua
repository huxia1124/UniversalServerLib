require 'test_add'

	err, ret = xpcall(function()
	
	    if math.random(40000) > 39996 then
		    --print("Received!! : TID = "..utils.GetThreadId())  
			print("-----------------------------------")
			print("Receive: "..math.floor((server:GetReceiveBytesPerSecond()/1024)).." KB/s,\tPackages: "..server:GetReceiveCountPerSecond().." pkg/sec,\t Current clients: "..server:GetTcpClientCount())
			print("         "..math.floor((server:GetTotalReceivedBytes()/1024/1024)).." MB,\tPackages: "..server:GetTotalReceivedCount().." pkg")
			print("Send:    "..math.floor((server:GetSentBytesPerSecond()/1024)).." KB/s,\tPackages: "..server:GetSentCountPerSecond().." pkg/sec")
			print("         "..math.floor((server:GetTotalSentBytes()/1024/1024)).." MB,\tPackages: "..server:GetTotalSentCount().." pkg")
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



