
	err, ret = xpcall(function()
	
			local serverPort = utils.GetServerPort()
		    local clientIp = utils.GetClientIp()
			print(clientIp)
			server:SendStringToUdpClientEx(serverPort, clientIp, "Test UDP Message")
			
		
	end, debug.traceback)
	
	if (ret) then
		--print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



