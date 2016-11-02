
	err, ret = xpcall(function()
	
		 print("Data received : TID = "..utils.GetThreadId())
		 local msg = utils.GetMessage()
		 local firstString = msg:GetNextString()
		 print(firstString)
		 server:Log(firstString)
		 local uid = utils.GetClientUid()
		 server:SendPackageToClient(uid, msg)
	end, debug.traceback)
	
	if (ret) then
		print("complicated(true) --> ", err, ret)
		--server:Log(err)   
		end



