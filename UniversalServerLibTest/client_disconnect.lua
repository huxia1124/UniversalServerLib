
	err, ret = xpcall(function()
		local uid = utils.GetClientUid()
			local sharedData = SharedData()
			sharedData:UnregisterVariable("Server/TCP/"..utils.GetServerPort().."/Clients/"..uid)
		
	end, debug.traceback)
	
	if (ret) then
		--print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



