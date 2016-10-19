
	err, ret = xpcall(function()

		    local uid = utils.GetClientUid()
			server:SendStringToClient(uid, "Welcome! your UID is "..uid)

			local sharedData = SharedData()
			sharedData:RegisterStringVariable("Server/TCP/"..utils.GetServerPort().."/Clients/"..uid)

		
	end, debug.traceback)
	
	if (ret) then
		--print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



